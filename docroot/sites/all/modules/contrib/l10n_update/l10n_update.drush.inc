<?php

/**
 * @file
 * Drush interface to l10n-update functionalities.
 */

/**
 * Implements hook_drush_command().
 */
function l10n_update_drush_command() {
  $commands = array();
  $commands['l10n-update-refresh'] = array(
    'description' => 'Refresh available information.',
  );
  $commands['l10n-update-status'] = array(
    'description' => 'Show translation status of available projects.',
    'options' => array(
      'languages' => 'Comma separated list of languages. Defaults to all available languages. Example: --languages="nl, fr, de"',
    )
  );
  $commands['l10n-update'] = array(
    'description' => 'Update translations.',
    'options' => array(
      'languages' => 'Comma separated list of languages. Defaults to all available languages. Example: --languages="nl, fr, de"',
      'mode' => 'Determine if existing translations are overwitten during import. Use "overwrite" to overwrite any existing translation, "replace" to replace previously imported translations but not overwrite edited strings, "keep" to keep any existing translation and only add new translations. Default value: "keep"'
    ),
  );
  return $commands;
}

/**
 * Callback for command l10n-update-refresh.
 */
function drush_l10n_update_refresh() {
  module_load_include('compare.inc', 'l10n_update');

  // Check the translation status of all translatable projects in all languages.
  // First we clear the cached list of projects. Although not strictly
  // necessary, this is helpful in case the project list is out of sync.
  l10n_update_flush_projects();
  l10n_update_check_projects();

  // Execute a batch if required. A batch is only used when remote files
  // are checked.
  if (batch_get()) {
    drush_backend_batch_process();
  }
}

/**
 * Validate command l10n-update-status.
 */
function drush_l10n_update_status_validate() {
  return _drush_l10n_update_validate_languages();
}

/**
 * Callback for command l10n-update-status.
 */
function drush_l10n_update_status() {
  $status = l10n_update_get_status();

  if (!empty($status)) {
    $languages = drush_get_option('languages');

    // Build table header.
    $table = array();
    $header = array(dt('Project'));
    foreach ($languages as $langcode => $language) {
      $header[] = $language->name;
    }
    $table[] = $header;

    // Iterate projects to obtain per language status.
    foreach ($status as $name => $project) {
      $row = array();
      // First column: Project name & version number.
      $project_details = reset($project);
      $title = isset($project_details->title) ? $project_details->title : $project_details->name;
      $row[] = dt('@project (@version)', array('@project' => $title, '@version' => $project_details->version));

      // Other columns: Status per language.
      foreach ($languages as $langcode => $language) {
        $current = $project[$langcode]->type == L10N_UPDATE_CURRENT;
        $local_update = $project[$langcode]->type == L10N_UPDATE_LOCAL;
        $remote_update = $project[$langcode]->type == L10N_UPDATE_REMOTE;
        if ($local_update || $remote_update) {
          $row[] = $remote_update ? dt('Remote update available') : dt('Local update available');
        }
        elseif ($current) {
          $row[] = dt('Up to date');
        }
        else {
          $row[] = dt('No info');
        }
      }
      $table[] = $row;
    }
    drush_print_table($table, TRUE);
  }
  else {
    drush_log(dt('No languages to update.'), 'warning');
  }
}

/**
 * Validate command l10n-update.
 */
function drush_l10n_update_validate() {
  $lang_validation = _drush_l10n_update_validate_languages();
  if ($lang_validation == FALSE) {
    return FALSE;
  }
  // Check provided update mode is valid.
  $mode = drush_get_option('mode', 'keep');
  if (!in_array($mode, array('keep', 'replace', 'overwrite'))) {
    return drush_set_error('L10N_UPDATE_INVALID_MODE', dt('Invalid update mode. Valid options are keep, replace, overwrite.'));
  }
}

/**
 * Callback for command l10n-update.
 */
function drush_l10n_update() {
  module_load_include('fetch.inc', 'l10n_update');
  $updates = _drush_l10n_update_get_updates();

  if ($updates['projects']) {
    drush_log(dt('Found @count projects to update.', array('@count' => count($updates['projects']))), 'status');

    // Batch update all projects for selected languages.
    $mode = drush_get_option('mode', 'keep');
    $options = _l10n_update_default_update_options();

    switch ($mode) {
      case 'keep':
        $options['overwrite_options'] = array(
          'not_customized' => FALSE,
          'customized' => FALSE,
        );
        break;
      case 'replace':
        $options['overwrite_options'] = array(
          'not_customized' => TRUE,
          'customized' => FALSE,
        );
        break;
      case 'overwrite':
        $options['overwrite_options'] = array(
          'not_customized' => TRUE,
          'customized' => TRUE,
        );
        break;

      default:
        return drush_set_error('L10N_UPDATE_INVALID_MODE', dt('Invalid update mode. Valid options are keep, overwrite.'));
        break;
    }

    $languages = array_keys(drush_get_option('languages'));

    // Get translation status of the projects, download and update translations.
    $batch = l10n_update_batch_update_build(array(), $languages, $options);
    drush_log($batch['title'], 'status');
    drush_log($batch['init_message'], 'status');
    batch_set($batch);
    drush_backend_batch_process();
  }
  else {
    drush_log(dt('All project translations up to date'), 'status');
  }
}

/**
 * Helper function to validate languages.
 *
 * Used by _validate hooks.
 *  1. Check other languages than english are available.
 *  2. Check user provided languages are valid.
 */
function _drush_l10n_update_validate_languages() {
  // Check if there are installed languages other than english.
  $installed_languages = l10n_update_translatable_language_list();

  // Indicate that there's nothing to do, only show a warning.
  if (empty($installed_languages)) {
    drush_log(dt('No languages to update.'), 'warning');
    return FALSE;
  }

  // Check provided languages are valid.
  $languages = drush_get_option('languages', '');
  $languages = array_map('trim', _convert_csv_to_array($languages));
  if (count($languages)) {
    foreach ($languages as $key => $langcode) {
      if (!isset($installed_languages[$langcode])) {
        if (is_numeric($langcode)) {
          drush_set_error('L10N_UPDATE_INVALID_LANGUAGE', dt('Invalid language "@langcode". Use for example: --languages="nl, fr, de"', array('@langcode' => $langcode)));
        }
        else {
          drush_set_error('L10N_UPDATE_INVALID_LANGUAGE', dt('Language "@langcode" is not installed.', array('@langcode' => $langcode)));
        }
      }
      else {
        unset($languages[$key]);
        $languages[$langcode] = $installed_languages[$langcode];
      }
    }
    if (drush_get_error() != DRUSH_SUCCESS) {
      drush_print(dt('Available languages: @languages', array('@languages' => implode(', ', array_keys($installed_languages)))));
      return FALSE;
    }
  }
  else {
    $languages = $installed_languages;
  }
  drush_set_option('languages', $languages);
  return TRUE;
}

/**
 * Helper function to obtain $updates.
 *
 * @return $updates array or NULL.
 */
function _drush_l10n_update_get_updates() {
  $updates = array();
  $languages = l10n_update_translatable_language_list();
  $status = l10n_update_get_status();

    drush_log(dt('Fetching update information for all projects / all languages.'), 'status');

  // Prepare information about projects which have available translation
  // updates.
  if ($languages && $status) {
    foreach ($status as $project) {
      foreach ($project as $langcode => $project_info) {
        // Translation update found for this project-language combination.
        if ($project_info->type && ($project_info->type == L10N_UPDATE_LOCAL || $project_info->type == L10N_UPDATE_REMOTE)) {
          $updates['projects'][$project_info->name] = $project_info;
          $updates['languages'][$langcode] = $project_info;
        }
      }
    }
  }
  if ($updates) {
    return $updates;
  }
  else {
    drush_log(dt('No languages to update.'), 'warning');
  }
}
