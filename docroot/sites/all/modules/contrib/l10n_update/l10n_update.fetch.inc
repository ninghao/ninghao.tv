<?php

/**
 * @file
 * The API for download and import of translations from remote and local sources.
 */

/**
 * Load the common translation API.
 */
// @todo Combine functions differently in files to avoid unnecessary includes.
// Follow-up issue http://drupal.org/node/1834298
require_once __DIR__ . '/l10n_update.translation.inc';

/**
 * Builds a batch to check, download and import project translations.
 *
 * @param array $projects
 *   Array of project names for which to update the translations. Defaults to
 *   all translatable projects.
 * @param array $langcodes
 *   Array of language codes. Defaults to all translatable languages.
 * @param array $options
 *   Array of import options. See locale_translate_batch_import_files().
 *
 * @return array
 *   Batch definition array.
 */
function l10n_update_batch_update_build($projects = array(), $langcodes = array(), $options = array()) {
  module_load_include('compare.inc', 'l10n_update');
  $projects = $projects ? $projects : array_keys(l10n_update_get_projects());
  $langcodes = $langcodes ? $langcodes : array_keys(l10n_update_translatable_language_list());
  $status_options = $options;
  $status_options['finish_feedback'] = FALSE;

  // Check status of local and remote translation files.
  $operations = _l10n_update_batch_status_operations($projects, $langcodes, $status_options);
  // Download and import translations.
  $operations = array_merge($operations, _l10n_update_fetch_operations($projects, $langcodes, $options));

  $batch = array(
    'operations' => $operations,
    'title' => t('Updating translations'),
    'progress_message' => '',
    'error_message' => t('Error importing translation files'),
    'finished' => 'l10n_update_batch_fetch_finished',
    'file' => drupal_get_path('module', 'l10n_update') . '/l10n_update.batch.inc',
  );
  return $batch;
}

/**
 * Builds a batch to download and import project translations.
 *
 * @param array $projects
 *   Array of project names for which to check the state of translation files.
 *   Defaults to all translatable projects.
 * @param array $langcodes
 *   Array of language codes. Defaults to all translatable languages.
 * @param array $options
 *   Array of import options. See l10n_update_batch_import_files().
 *
 * @return array
 *   Batch definition array.
 */
function l10n_update_batch_fetch_build($projects = array(), $langcodes = array(), $options = array()) {
  $projects = $projects ? $projects : array_keys(l10n_update_get_projects());
  $langcodes = $langcodes ? $langcodes : array_keys(l10n_update_translatable_language_list());

  $batch = array(
    'operations' => _l10n_update_fetch_operations($projects, $langcodes, $options),
    'title' => t('Updating translations.'),
    'progress_message' => '',
    'error_message' => t('Error importing translation files'),
    'finished' => 'l10n_update_batch_fetch_finished',
    'file' => drupal_get_path('module', 'l10n_update') . '/l10n_update.batch.inc',
  );
  return $batch;
}

/**
 * Helper function to construct the batch operations to fetch translations.
 *
 * @param array $projects
 *   Array of project names for which to check the state of translation files.
 *   Defaults to all translatable projects.
 * @param array $langcodes
 *   Array of language codes. Defaults to all translatable languages.
 * @param array $options
 *   Array of import options.
 *
 * @return array
 *   Array of batch operations.
 */
function _l10n_update_fetch_operations($projects, $langcodes, $options) {
  $operations = array();

  foreach ($projects as $project) {
    foreach ($langcodes as $langcode) {
      if (l10n_update_use_remote_source()) {
        $operations[] = array('l10n_update_batch_fetch_download', array($project, $langcode));
      }
      $operations[] = array('l10n_update_batch_fetch_import', array($project, $langcode, $options));
    }
  }

  return $operations;
}
