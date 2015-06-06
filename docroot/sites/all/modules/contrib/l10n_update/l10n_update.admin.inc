<?php

/**
 * @file
 *   Admin settings and update page.
 */

/**
 * Page callback: Checks for translation updates and displays the status.
 *
 * Manually checks the translation status without the use of cron.
 */
function l10n_update_manual_status() {
  module_load_include('compare.inc', 'l10n_update');

  // Check the translation status of all translatable projects in all languages.
  // First we clear the cached list of projects. Although not strictly
  // necessary, this is helpful in case the project list is out of sync.
  l10n_update_flush_projects();
  l10n_update_check_projects();

  // Execute a batch if required. A batch is only used when remote files
  // are checked.
  if (batch_get()) {
    batch_process('admin/config/regional/translate/update');
  }
  drupal_goto('admin/config/regional/translate/update');
}

/**
 * Page callback: Translation status page.
 */
function l10n_update_status_form() {
  module_load_include('compare.inc', 'l10n_update');
  $updates = $options = array();
  $languages_update = $languages_not_found = array();
  $projects_update = array();

  // @todo Calling l10n_update_build_projects() is an expensive way to
  //   get a module name. In follow-up issue http://drupal.org/node/1842362
  //   the project name will be stored to display use, like here.
  $project_data = l10n_update_build_projects();
  $languages = l10n_update_translatable_language_list();
  $status = l10n_update_get_status();

  // Prepare information about projects which have available translation
  // updates.
  if ($languages && $status) {
    foreach ($status as $project) {
      foreach ($project as $langcode => $project_info) {
        if (isset($project_data[$project_info->name])) {
          // No translation file found for this project-language combination.
          if (empty($project_info->type)) {
            $updates[$langcode]['not_found'][] = array(
              'name' => $project_info->name == 'drupal' ? t('Drupal core') : $project_data[$project_info->name]->info['name'],
              'version' => $project_info->version,
              'info' => _l10n_update_status_debug_info($project_info),
            );
            $languages_not_found[$langcode] = $langcode;
          }
          // Translation update found for this project-language combination.
          elseif ($project_info->type == L10N_UPDATE_LOCAL || $project_info->type == L10N_UPDATE_REMOTE ) {
            $local = isset($project_info->files[L10N_UPDATE_LOCAL]) ? $project_info->files[L10N_UPDATE_LOCAL] : NULL;
            $remote = isset($project_info->files[L10N_UPDATE_REMOTE]) ? $project_info->files[L10N_UPDATE_REMOTE] : NULL;
            $recent = _l10n_update_source_compare($local, $remote) == L10N_UPDATE_SOURCE_COMPARE_LT ? $remote : $local;
            $name = isset($project_data[$project_info->name]->info['name']) ? $project_data[$project_info->name]->info['name'] : '';
            $updates[$langcode]['updates'][] = array(
              'name' => $project_info->name == 'drupal' ? t('Drupal core') : $name,
              'version' => $project_info->version,
              'timestamp' => $recent->timestamp,
            );
            $languages_update[$langcode] = $langcode;
            $projects_update[$project_info->name] = $project_info->name;
          }
        }
      }
    }
    $languages_not_found = array_diff($languages_not_found, $languages_update);

    // Build data options for the select table.
    foreach($updates as $langcode => $update) {
      $title = check_plain($languages[$langcode]);
      $l10n_update_update_info = array('#theme' => 'l10n_update_update_info');
      foreach (array('updates', 'not_found') as $update_status) {
        if (isset($update[$update_status])) {
          $l10n_update_update_info['#' . $update_status] = $update[$update_status];
        }
      }
      $options[$langcode] = array(
        'title' => array(
          'class' => array('label'),
          'data' => array(
            '#title' => $title,
            '#markup' => $title
          ),
        ),
        'status' => array('class' => array('description', 'expand', 'priority-low'), 'data' => drupal_render($l10n_update_update_info)),
      );
    }
    // Sort the table data on language name.
    uasort($options, function ($a, $b) {
      return strcasecmp($a['title']['data']['#title'], $b['title']['data']['#title']);
    });
  }

  $last_checked = variable_get('l10n_update_last_check');
  $form['last_checked'] = array(
    '#theme' => 'l10n_update_last_check',
    '#last' => $last_checked,
  );

  $header = array(
    'title' => array(
      'data' => t('Language'),
      'class' => array('title'),
    ),
    'status' => array(
      'data' => t('Status'),
      'class' => array('status', 'priority-low'),
    ),
  );

  if (!$languages) {
    $empty = t('No translatable languages available. <a href="@add_language">Add a language</a> first.', array('@add_language' => url('admin/config/regional/language')));
  }
  elseif (empty($options)) {
    $empty = t('All translations up to date.');
  }
  else  {
    $empty = t('No translation status available. <a href="@check">Check manually</a>.', array('@check' => url('admin/config/regional/translate/check')));
  }

  // The projects which require an update. Used by the _submit callback.
  $form['projects_update'] = array(
    '#type' => 'value',
    '#value' => $projects_update,
  );
  $form['langcodes'] = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $options,
    '#default_value' => $languages_update,
    '#empty' => $empty,
    '#js_select' => TRUE,
    '#multiple' => TRUE,
    '#required' => TRUE,
    '#not_found' => $languages_not_found,
    '#after_build' => array('l10n_update_language_table'),
    '#attributes' => array(),
  );

  $form['#attached'] = array(
    'js' => array(
      drupal_get_path('module', 'l10n_update') . '/js/l10n_update.admin.js',
    ),
    'css' => array(
      drupal_get_path('module', 'l10n_update') . '/css/l10n_update.admin.css',
    ),
  );

  if ($languages_update) {
    $form['actions'] = array(
      '#type' => 'actions',
      'submit' => array(
        '#type' => 'submit',
        '#value' => t('Update translations'),
      ),
      '#attributes' => array(),
    );
  }

  return $form;
}

/**
 * Form validation handler for locale_translation_status_form().
 */
function l10n_update_status_form_validate($form, &$form_state) {
  // Check if a language has been selected. 'tableselect' doesn't.
  if (!array_filter($form_state['values']['langcodes'])) {
    form_set_error('', t('Select a language to update.'));
  }
}

/**
 * Form submission handler for locale_translation_status_form().
 */
function l10n_update_status_form_submit($form, $form_state) {
  module_load_include('fetch.inc', 'l10n_update');
  $langcodes = array_filter($form_state['values']['langcodes']);
  $projects = array_filter($form_state['values']['projects_update']);

  // Set the translation import options. This determines if existing
  // translations will be overwritten by imported strings.
  $options = _l10n_update_default_update_options();

  // If the status was updated recently we can immediately start fetching the
  // translation updates. If the status is expired we clear it an run a batch to
  // update the status and then fetch the translation updates.
  $last_checked = variable_get('l10n_update_last_check');
  if ($last_checked < REQUEST_TIME - L10N_UPDATE_STATUS_TTL) {
    l10n_update_clear_status();
    $batch = l10n_update_batch_update_build(array(), $langcodes, $options);
    batch_set($batch);
  }
  else {
    $batch = l10n_update_batch_fetch_build($projects, $langcodes, $options);
    batch_set($batch);
  }
}

/**
 * Page callback: Settings form.
 */
function l10n_update_admin_settings_form($form, &$form_state) {
  $form['l10n_update_check_frequency'] = array(
    '#type' => 'radios',
    '#title' => t('Check for updates'),
    '#default_value' => variable_get('l10n_update_check_frequency', '0'),
    '#options' => array(
      '0' => t('Never (manually)'),
      '7' => t('Weekly'),
      '30' => t('Monthly'),
    ),
    '#description' => t('Select how frequently you want to check for new interface translations for your currently installed modules and themes. <a href="@url">Check updates now</a>.', array('@url' => url('admin/config/regional/translate/check'))),
  );

  $form['l10n_update_check_disabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Check for updates of disabled modules and themes'),
    '#default_value' => variable_get('l10n_update_check_disabled', FALSE),
  );

  $form['l10n_update_check_mode'] = array(
    '#type' => 'radios',
    '#title' => t('Translation source'),
    '#default_value' => variable_get('l10n_update_check_mode', L10N_UPDATE_USE_SOURCE_REMOTE_AND_LOCAL),
    '#options' => array(
      L10N_UPDATE_USE_SOURCE_REMOTE_AND_LOCAL => t('Drupal translation server and local files'),
      L10N_UPDATE_USE_SOURCE_LOCAL => t('Local files only'),
    ),
    '#description' => t('The source of translation files for automatic interface translation.'),
  );

  $form['l10n_update_download_store'] = array(
    '#title' => t('Translations directory'),
    '#type' => 'textfield',
    '#default_value' => variable_get('l10n_update_download_store', L10N_UPDATE_DEFAULT_TRANSLATION_PATH),
    '#required' => TRUE,
    '#description' => t('A path relative to the Drupal installation directory where translation files will be stored, e.g. sites/all/translations. Saved translation files can be reused by other installations.'),
  );

  $form['l10n_update_import_mode'] = array(
    '#type' => 'radios',
    '#title' => t('Import behaviour'),
    '#default_value' => variable_get('l10n_update_import_mode', LOCALE_IMPORT_KEEP),
    '#options' => array(
      LOCALE_IMPORT_KEEP => t("Don't overwrite existing translations."),
      L10N_UPDATE_OVERWRITE_NON_CUSTOMIZED => t('Only overwrite imported translations, customized translations are kept.'),
      LOCALE_IMPORT_OVERWRITE => t('Overwrite existing translations.'),
    ),
    '#description' => t('How to treat existing translations when automatically updating the interface translations.'),
  );

  $form['#submit'][] = 'l10n_update_admin_settings_form_submit';
  return system_settings_form($form);
}

/**
 * Validation handler for translation update settings.
 */
function l10n_update_admin_settings_form_validate($form, &$form_state) {
  // Check for existing translations directory and create one if required.
  // When using local sources, only check if the directory exists.
  $directory = $form_state['values']['l10n_update_download_store'];
  $directory = rtrim($directory, '/\\');

  if ($form_state['values']['l10n_update_check_mode'] == L10N_UPDATE_USE_SOURCE_LOCAL
    && is_dir($directory)) {
    return;
  }
  if (!file_prepare_directory($directory, FILE_CREATE_DIRECTORY | FILE_MODIFY_PERMISSIONS)) {
    form_set_error('l10n_update_download_store', t('The directory %directory does not exist or is not writable.', array('%directory' => $directory)));
    watchdog('file system', 'The directory %directory does not exist or is not writable.', array('%directory' => $directory), WATCHDOG_ERROR);
  }
}

/**
 * Submit handler for translation update settings.
 */
function l10n_update_admin_settings_form_submit($form, $form_state) {
  // Invalidate the cached translation status when the configuration setting of
  // 'l10n_update_check_mode' or 'check_disabled' change.
  if ($form['l10n_update_check_mode']['#default_value'] != $form_state['values']['l10n_update_check_mode'] ||
    $form['l10n_update_check_disabled']['#default_value'] != $form_state['values']['l10n_update_check_disabled']) {
    l10n_update_clear_status();
  }
}

/**
 * Get array of import options.
 *
 * The import options of the Locale module are used but the UI text is altered
 * to suit the Localization update cases.
 *
 * @return
 *   Keyed array of import options.
 */
function _l10n_update_admin_import_options() {
  return array(
    LOCALE_IMPORT_OVERWRITE => t('Translation updates replace existing ones, new ones are added'),
    L10N_UPDATE_OVERWRITE_NON_CUSTOMIZED => t('Edited translations are kept, only previously imported ones are overwritten and new translations are added'),
    LOCALE_IMPORT_KEEP => t('All existing translations are kept, only new translations are added.'),
  );
}

/**
 * Provides debug info for projects in case translation files are not found.
 *
 * Translations files are being fetched either from Drupal translation server
 * and local files or only from the local filesystem depending on the
 * "Translation source" setting at admin/config/regional/language/update.
 * This method will produce debug information including the respective path(s)
 * based on this setting.
 *
 * Translations for development versions are never fetched, so the debug info
 * for that is a fixed message.
 *
 * @param array $source
 *   An array which is the project information of the source.
 *
 * @return string
 *   The string which contains debug information.
 */
function _l10n_update_status_debug_info($source) {
  $remote_path = isset($source->files['remote']->uri) ? $source->files['remote']->uri : '';
  $local_path = isset($source->files['local']->uri) ? $source->files['local']->uri : '';

  if (strpos($source->version, 'dev') !== FALSE) {
    return t('No translation files are provided for development releases.');
  }
  if (l10n_update_use_remote_source() && $remote_path && $local_path) {
    return t('File not found at %remote_path nor at %local_path', array(
      '%remote_path' => $remote_path,
      '%local_path' => $local_path,
    ));
  }
  elseif ($local_path) {
    return t('File not found at %local_path', array('%local_path' => $local_path));
  }
  return t('Translation file location could not be determined.');
}

/**
 * Form element callback: After build changes to the language update table.
 *
 * Adds labels to the languages and removes checkboxes from languages from which
 * translation files could not be found.
 */
function l10n_update_language_table($form_element) {
  // Remove checkboxes of languages without updates.
  if ($form_element['#not_found']) {
    foreach ($form_element['#not_found'] as $langcode) {
      $form_element[$langcode] = array();
    }
  }
  return $form_element;
}

/**
 * Returns HTML for translation edit form.
 *
 * @param array $variables
 *   An associative array containing:
 *   - form: The form that contains the language information.
 *
 * @see l10n_update_edit_form()
 * @ingroup themeable
 */
function theme_l10n_update_edit_form_strings($variables) {
  $output = '';
  $form = $variables['form'];
  $header = array(
    t('Source string'),
    t('Translation for @language', array('@language' => $form['#language'])),
  );
  $rows = array();
  foreach (element_children($form) as $lid) {
    $string = $form[$lid];
    if ($string['plural']['#value']) {
      $source = drupal_render($string['original_singular']) . '<br />' . drupal_render($string['original_plural']);
    }
    else {
      $source = drupal_render($string['original']);
    }
    $source .= empty($string['context']) ? '' : '<br /><small>' . t('In Context') . ':&nbsp;' . $string['context']['#value'] . '</small>';
    $rows[] = array(
      array('data' => $source),
      array('data' => $string['translations']),
    );
  }
  $table = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No strings available.'),
    '#attributes' => array('class' => array('locale-translate-edit-table')),
  );
  $output .= drupal_render($table);
  $pager = array('#theme' => 'pager');
  $output .= drupal_render($pager);
  return $output;
}

/**
 * Prepares variables for translation status information templates.
 *
 * Translation status information is displayed per language.
 *
 * Default template: l10n_update-translation-update-info.tpl.php.
 *
 * @param array $variables
 *   An associative array containing:
 *   - updates: The projects which have updates.
 *   - not_found: The projects which updates are not found.
 *
 * @see l10n_update_status_form()
 */
function template_preprocess_l10n_update_update_info(&$variables) {
  $details = array();
  $modules = array();

  // Default values
  $variables['modules'] = array();
  $variables['module_list'] = '';
  $details['available_updates_list'] = array();

  // Build output for available updates.
  if (isset($variables['updates'])) {
    $releases = array();
    if ($variables['updates']) {
      foreach ($variables['updates'] as $update) {
        $modules[] = $update['name'];
        $releases[] = t('@module (@date)', array('@module' => $update['name'], '@date' => format_date($update['timestamp'], 'html_date')));
      }
      $variables['modules'] = $modules;
      $variables['module_list'] = t('Updates for: @modules', array('@modules' => implode(', ', $modules)));
    }
    $details['available_updates_list'] = array(
      '#theme' => 'item_list',
      '#items' => $releases,
    );
  }

  // Build output for updates not found.
  if (isset($variables['not_found'])) {
    $releases = array();
    $variables['missing_updates_status'] = format_plural(count($variables['not_found']), 'Missing translations for one project', 'Missing translations for @count projects');
    if ($variables['not_found']) {
      foreach ($variables['not_found'] as $update) {
        $version = $update['version'] ? $update['version'] : t('no version');
        $releases[] = t('@module (@version).', array('@module' => $update['name'], '@version' => $version)) . ' ' . $update['info'];
      }
    }
    $details['missing_updates_list'] = array(
      '#theme' => 'item_list',
      '#items' => $releases,
    );
    // Prefix the missing updates list if there is an available updates lists
    // before it.
    if (!empty($details['missing_updates_list']['#items'])) {
      $details['missing_updates_list']['#prefix'] = t('Missing translations for:');
    }
  }
  $variables['details'] = $details;
}

/**
 * Prepares variables for most recent translation update templates.
 *
 * Displays the last time we checked for locale update data. In addition to
 * properly formatting the given timestamp, this function also provides a "Check
 * manually" link that refreshes the available update and redirects back to the
 * same page.
 *
 * Default template: l10n_update-translation-last-check.tpl.php.
 *
 * @param $variables
 *   An associative array containing:
 *   - last: The timestamp when the site last checked for available updates.
 *
 * @see l10n_update_status_form()
 */
function template_preprocess_l10n_update_last_check(&$variables) {
  $last = $variables['last'];
  $variables['last_checked'] = $last ? t('Last checked: !time ago', array('!time' => format_interval(REQUEST_TIME - $last))) : t('Last checked: never');
  $variables['link'] = l(t('Check manually'), 'admin/config/regional/translate/check');
}
