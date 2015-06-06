<?php

/**
 * @file
 *   Batch process to check the availability of remote or local po files.
 */

/**
 * Load the common translation API.
 */
// @todo Combine functions differently in files to avoid unnecessary includes.
// Follow-up issue http://drupal.org/node/1834298
require_once __DIR__ . '/l10n_update.translation.inc';

/**
 * Batch operation callback: Check status of a remote and local po file.
 *
 * Checks the presence and creation time po translation files in located at
 * remote server location and local file system.
 *
 * @param string $project
 *   Machine name of the project for which to check the translation status.
 * @param string $langcode
 *   Language code of the language for which to check the translation.
 * @param array $options
 *   Optional, an array with options that can have the following elements:
 *   - 'finish_feedback': Whether or not to give feedback to the user when the
 *     batch is finished. Optional, defaults to TRUE.
 *   - 'use_remote': Whether or not to check the remote translation file.
 *     Optional, defaults to TRUE.
 * @param array $context
 *   The batch context.
*/
function l10n_update_batch_status_check($project, $langcode, $options = array(), &$context) {
  $failure = $checked = FALSE;
  $options += array(
    'finish_feedback' => TRUE,
    'use_remote' => TRUE,
  );
  $source = l10n_update_get_status(array($project), array($langcode));
  $source = $source[$project][$langcode];

  // Check the status of local translation files.
  if (isset($source->files[L10N_UPDATE_LOCAL])) {
    if ($file = l10n_update_source_check_file($source)) {
      l10n_update_status_save($source->name, $source->langcode, L10N_UPDATE_LOCAL, $file);
    }
    $checked = TRUE;
  }

  // Check the status of remote translation files.
  if ($options['use_remote'] && isset($source->files[L10N_UPDATE_REMOTE])) {
    $remote_file = $source->files[L10N_UPDATE_REMOTE];
    module_load_include('http.inc', 'l10n_update');
    if ($result = l10n_update_http_check($remote_file->uri)) {
      // Update the file object with the result data. In case of a redirect we
      // store the resulting uri.
      if (!empty($result->updated)) {
        $remote_file->uri = isset($result->redirect_url) ? $result->redirect_url : $remote_file->uri;
        $remote_file->timestamp = $result->updated;
        l10n_update_status_save($source->name, $source->langcode, L10N_UPDATE_REMOTE, $remote_file);
      }
      // @todo What to do with when the file is not found (404)? To prevent
      //   re-checking within the TTL (1day, 1week) we can set a last_checked
      //   timestamp or cache the result.
      $checked = TRUE;
    }
    else {
      $failure = TRUE;
    }
  }

  // Provide user feedback and record success or failure for reporting at the
  // end of the batch.
  if ($options['finish_feedback'] && $checked) {
    $context['results']['files'][] = $source->name;
  }
  if ($failure && !$checked) {
    $context['results']['failed_files'][] = $source->name;
  }
  $context['message'] = t('Checked translation for %project.', array('%project' => $source->project));
}

/**
 * Batch finished callback: Set result message.
 *
 * @param boolean $success
 *   TRUE if batch successfully completed.
 * @param array $results
 *   Batch results.
 */
function l10n_update_batch_status_finished($success, $results) {
  if ($success) {
    if (isset($results['failed_files'])) {
        if (module_exists('dblog')) {
          $message = format_plural(count($results['failed_files']), 'One translation file could not be checked. <a href="@url">See the log</a> for details.', '@count translation files could not be checked. <a href="@url">See the log</a> for details.', array('@url' => url('admin/reports/dblog')));
        }
        else {
          $message = format_plural(count($results['failed_files']), 'One translation files could not be checked. See the log for details.', '@count translation files could not be checked. See the log for details.');
        }
        drupal_set_message($message, 'error');
    }
    if (isset($results['files'])) {
      drupal_set_message(format_plural(
        count($results['files']),
        'Checked available interface translation updates for one project.',
        'Checked available interface translation updates for @count projects.'
      ));
    }
    if (!isset($results['failed_files']) && !isset($results['files'])) {
      drupal_set_message(t('Nothing to check.'));
    }
    variable_set('l10n_update_last_check', REQUEST_TIME);
  }
  else {
    drupal_set_message(t('An error occurred trying to check available interface translation updates.'), 'error');
  }
}

/**
 * Batch operation: Download a remote translation file.
 *
 * Downloads a remote gettext file into the translations directory. When
 * successfully the translation status is updated.
 *
 * @param string $project
 *   Name of the translatable project.
 * @param string $langcode
 *   Language code.
 * @param array $context
 *   The batch context.
 *
 * @see l10n_update_batch_fetch_import()
 */
function l10n_update_batch_fetch_download($project, $langcode, &$context) {
  $sources = l10n_update_get_status(array($project), array($langcode));
  if (isset($sources[$project][$langcode])) {
    $source = $sources[$project][$langcode];
    if (isset($source->type) && $source->type == L10N_UPDATE_REMOTE) {
      if ($file = l10n_update_download_source($source->files[L10N_UPDATE_REMOTE], 'translations://')) {
        $context['message'] = t('Downloaded translation for %project.', array('%project' => $source->project));
        l10n_update_status_save($source->name, $source->langcode, L10N_UPDATE_LOCAL, $file);
      }
      else {
        $context['results']['failed_files'][] = $source->files[L10N_UPDATE_REMOTE];
      }
    }
  }
}

/**
 * Batch process: Import translation file.
 *
 * Imports a gettext file from the translation directory. When successfully the
 * translation status is updated.
 *
 * @param string $project
 *   Name of the translatable project.
 * @param string $langcode
 *   Language code.
 * @param array $options
 *   Array of import options.
 * @param array $context
 *   The batch context.
 *
 * @see l10n_update_batch_import_files()
 * @see l10n_update_batch_fetch_download()
 */
function l10n_update_batch_fetch_import($project, $langcode, $options, &$context) {
  $sources = l10n_update_get_status(array($project), array($langcode));
  if (isset($sources[$project][$langcode])) {
    $source = $sources[$project][$langcode];
    if (isset($source->type)) {
      if ($source->type == L10N_UPDATE_REMOTE || $source->type == L10N_UPDATE_LOCAL) {
        $file = $source->files[L10N_UPDATE_LOCAL];
        module_load_include('bulk.inc', 'l10n_update');
        $options += array(
          'message' => t('Importing translation for %project.', array('%project' => $source->project)),
        );
        // Import the translation file. For large files the batch operations is
        // progressive and will be called repeatedly until finished.
        l10n_update_batch_import($file, $options, $context);

        // The import is finished.
        if (isset($context['finished']) && $context['finished'] == 1) {
          // The import is successful.
          if (isset($context['results']['files'][$file->uri])) {
            $context['message'] = t('Imported translation for %project.', array('%project' => $source->project));

            // Save the data of imported source into the {l10n_update_file} table and
            // update the current translation status.
            l10n_update_status_save($project, $langcode, L10N_UPDATE_CURRENT, $source->files[L10N_UPDATE_LOCAL]);
          }
        }
      }
    }
  }
}

/**
 * Batch finished callback: Set result message.
 *
 * @param boolean $success
 *   TRUE if batch successfully completed.
 * @param array
 *   Batch results.
 */
function l10n_update_batch_fetch_finished($success, $results) {
  module_load_include('bulk.inc', 'l10n_update');
  if ($success) {
    variable_set('l10n_update_last_check', REQUEST_TIME);
  }
  l10n_update_batch_finished($success, $results);
}



/**
 * Downloads a translation file from a remote server.
 *
 * @param object $source_file
 *   Source file object with at least:
 *   - "uri": uri to download the file from.
 *   - "project": Project name.
 *   - "langcode": Translation language.
 *   - "version": Project version.
 *   - "filename": File name.
 * @param string $directory
 *   Directory where the downloaded file will be saved. Defaults to the
 *   temporary file path.
 *
 * @return object
 *   File object if download was successful. FALSE on failure.
 */
function l10n_update_download_source($source_file, $directory = 'temporary://') {
  if ($uri = system_retrieve_file($source_file->uri, $directory)) {
    $file = clone($source_file);
    $file->type = L10N_UPDATE_LOCAL;
    $file->uri = $uri;
    $file->directory = $directory;
    $file->timestamp = filemtime($uri);
    return $file;
  }
  watchdog('l10n_update', 'Unable to download translation file @uri.', array('@uri' => $source_file->uri), WATCHDOG_ERROR);
  return FALSE;
}
