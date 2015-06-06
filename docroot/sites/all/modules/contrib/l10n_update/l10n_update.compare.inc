<?php

/**
 * @file
 * The API for comparing project translation status with available translation.
 */

/**
 * Load common APIs.
 */
// @todo Combine functions differently in files to avoid unnecessary includes.
// Follow-up issue http://drupal.org/node/1834298
require_once __DIR__ . '/l10n_update.translation.inc';

/**
 * Clear the project data table.
 */
function l10n_update_flush_projects() {
  db_truncate('l10n_update_project')->execute();
  drupal_static_reset('l10n_update_build_projects');
}

/**
 * Rebuild project list
 *
 * @param $refresh
 *   TRUE: Refresh project list.
 *
 * @return array
 *   Array of project objects to be considered for translation update.
 */
function l10n_update_build_projects($refresh = FALSE) {
  $projects = &drupal_static(__FUNCTION__, array(), $refresh);
  if (empty($projects)) {
    module_load_include('inc', 'l10n_update');

    // Get the project list based on .info files.
    $projects = l10n_update_project_list();

    // Mark all previous projects as disabled and store new project data.
    db_update('l10n_update_project')
      ->fields(array(
        'status' => 0,
      ))
      ->execute();

    $default_server = l10n_update_default_translation_server();

    if (module_exists('update')) {
      $projects_info = update_get_available(TRUE);
    }
    foreach ($projects as $name => $data) {

      // Force update fetch of project data in cases where Drupal's performance
      // optimized approach is missing out on some projects.
      // @see http://drupal.org/node/1671570#comment-6216090
      if (module_exists('update') && !isset($projects_info[$name])) {
        module_load_include('fetch.inc', 'update');
        _update_process_fetch_task($data);
        $available = _update_get_cached_available_releases();
        if (!empty($available[$name])) {
          $projects_info[$name] = $available[$name];
        }
      }

      if (isset($projects_info[$name]['releases']) && $projects_info[$name]['project_status'] != 'not-fetched') {
        // Find out if a dev version is installed.
        if (preg_match("/^[0-9]+\.x-([0-9]+)\..*-dev$/", $data['info']['version'], $matches)) {
          // Find a suitable release to use as alternative translation.
          foreach ($projects_info[$name]['releases'] as $project_release) {
            // The first release with the same major release number which is not
            // a dev release is the one. Releases are sorted the most recent first.
            if ($project_release['version_major'] == $matches[1] &&
              (!isset($project_release['version_extra']) || $project_release['version_extra'] != 'dev')) {
              $release = $project_release;
              break;
            }
          }
        }
        if (!empty($release['version'])) {
          $data['info']['version'] = $release['version'];
        }

        unset($release);
      }
      // Without Update module we do a best effort fallback. A development
      // release will fall back to the corresponding release version.
      elseif (!isset($projects_info) && isset($data['info']['version'])) {
        if (preg_match('/[^x](\+\d+)?-dev$/', $data['info']['version'])) {
          $data['info']['version'] = preg_replace('/(\+\d+)?-dev$/', '', $data['info']['version']);
        }
      }

      $data += array(
        'version' => isset($data['info']['version']) ? $data['info']['version'] : '',
        'core' => isset($data['info']['core']) ? $data['info']['core'] : DRUPAL_CORE_COMPATIBILITY,
        'l10n_path' => isset($data['info']['l10n path']) && $data['info']['l10n path'] ? $data['info']['l10n path'] : $default_server['pattern'],
        'status' => 1,
      );
      $project = (object) $data;
      $projects[$name] = $project;

      // Create or update the project record.
      db_merge('l10n_update_project')
        ->key(array('name' => $project->name))
        ->fields(array(
          'name' => $project->name,
          'project_type' => $project->project_type,
          'core' => $project->core,
          'version' => $project->version,
          'l10n_path' => $project->l10n_path,
          'status' => $project->status,
        ))
        ->execute();

      // Invalidate the cache of translatable projects.
      l10n_update_clear_cache_projects();
    }
  }
  return $projects;
}

/**
 * Get update module's project list
 *
 * @return array
 */
function l10n_update_project_list() {
  $projects = array();
  $disabled = variable_get('l10n_update_check_disabled', 0);
  // Unlike update module, this one has no cache
  _l10n_update_project_info_list($projects, system_rebuild_module_data(), 'module', $disabled);
  _l10n_update_project_info_list($projects, system_rebuild_theme_data(), 'theme', $disabled);
  // Allow other modules to alter projects before fetching and comparing.
  drupal_alter('l10n_update_projects', $projects);
  return $projects;
}

/**
 * Populate an array of project data.
 *
 * Based on _update_process_info_list()
 *
 * @param $projects
 * @param $list
 * @param $project_type
 * @param $disabled
 *   TRUE to include disabled projects too
 */
function _l10n_update_project_info_list(&$projects, $list, $project_type, $disabled = FALSE) {
  foreach ($list as $file) {
    if (!$disabled && empty($file->status)) {
      // Skip disabled modules or themes.
      continue;
    }

    // Skip if the .info file is broken.
    if (empty($file->info)) {
      continue;
    }

    // If the .info doesn't define the 'project', try to figure it out.
    if (!isset($file->info['project'])) {
      $file->info['project'] = l10n_update_get_project_name($file);
    }

    // If the .info defines the 'interface translation project', this value will
    // override the 'project' value.
    if (isset($file->info['interface translation project'])) {
      $file->info['project'] = $file->info['interface translation project'];
    }

    // If we still don't know the 'project', give up.
    if (empty($file->info['project'])) {
      continue;
    }

    // If we don't already know it, grab the change time on the .info file
    // itself. Note: we need to use the ctime, not the mtime (modification
    // time) since many (all?) tar implementations will go out of their way to
    // set the mtime on the files it creates to the timestamps recorded in the
    // tarball. We want to see the last time the file was changed on disk,
    // which is left alone by tar and correctly set to the time the .info file
    // was unpacked.
    if (!isset($file->info['_info_file_ctime'])) {
      $info_filename = dirname($file->uri) . '/' . $file->name . '.info';
      $file->info['_info_file_ctime'] = filectime($info_filename);
    }

    $project_name = $file->info['project'];
    if (!isset($projects[$project_name])) {
      // Only process this if we haven't done this project, since a single
      // project can have multiple modules or themes.
      $projects[$project_name] = array(
        'name' => $project_name,
        'info' => $file->info,
        'datestamp' => isset($file->info['datestamp']) ? $file->info['datestamp'] : 0,
        'includes' => array($file->name => isset($file->info['name']) ? $file->info['name'] : $file->name),
        'project_type' => $project_name == 'drupal' ? 'core' : $project_type,
      );
    }
    else {
      $projects[$project_name]['includes'][$file->name] = $file->info['name'];
      $projects[$project_name]['info']['_info_file_ctime'] = max($projects[$project_name]['info']['_info_file_ctime'], $file->info['_info_file_ctime']);
    }
  }
}

/**
 * Given a $file object (as returned by system_rebuild_module_data()), figure
 * out what project it belongs to.
 *
 * Based on update_get_project_name().
 *
 * @param $file
 * @return string
 * @see system_get_files_database()
 */
function l10n_update_get_project_name($file) {
  $project_name = '';
  if (isset($file->info['project'])) {
    $project_name = $file->info['project'];
  }
  elseif (isset($file->info['package']) && (strpos($file->info['package'], 'Core') === 0)) {
    $project_name = 'drupal';
  }
  return $project_name;
}

/**
 * Retrieve data for default server.
 *
 * @return array
 *   Array of server parameters:
 *   - "server_pattern": URI containing po file pattern.
 */
function l10n_update_default_translation_server() {
  $pattern = variable_get('l10n_update_default_update_url', L10N_UPDATE_DEFAULT_SERVER_PATTERN);

  return array(
    'pattern' => $pattern,
  );
}

/**
 * Check for the latest release of project translations.
 *
 * @param array $projects
 *   Array of project names to check. Defaults to all translatable projects.
 * @param string $langcodes
 *   Array of language codes. Defaults to all translatable languages.
 *
 * @return array
 *   Available sources indexed by project and language.
 */
// @todo Return batch or NULL
function l10n_update_check_projects($projects = array(), $langcodes = array()) {
  if (l10n_update_use_remote_source()) {
    // Retrieve the status of both remote and local translation sources by
    // using a batch process.
    l10n_update_check_projects_batch($projects, $langcodes);
  }
  else {
    // Retrieve and save the status of local translations only.
    l10n_update_check_projects_local($projects, $langcodes);
    variable_set('l10n_update_last_check', REQUEST_TIME);
  }
}

/**
 * Gets and stores the status and timestamp of remote po files.
 *
 * A batch process is used to check for po files at remote locations and (when
 * configured) to check for po files in the local file system. The most recent
 * translation source states are stored in the state variable
 * 'l10n_update_translation_status'.
 *
 * @param array $projects
 *   Array of project names to check. Defaults to all translatable projects.
 * @param string $langcodes
 *   Array of language codes. Defaults to all translatable languages.
 */
function l10n_update_check_projects_batch($projects = array(), $langcodes = array()) {
  // Build and set the batch process.
  $batch = l10n_update_batch_status_build($projects, $langcodes);
  batch_set($batch);
}

/**
 * Builds a batch to get the status of remote and local translation files.
 *
 * The batch process fetches the state of both local and (if configured) remote
 * translation files. The data of the most recent translation is stored per
 * per project and per language. This data is stored in a state variable
 * 'l10n_update_translation_status'. The timestamp it was last updated is stored
 * in the state variable 'l10n_upate_last_checked'.
 *
 * @param array $projects
 *   Array of project names for which to check the state of translation files.
 *   Defaults to all translatable projects.
 * @param array $langcodes
 *   Array of language codes. Defaults to all translatable languages.
 *
 * @return array
 *   Batch definition array.
 */
function l10n_update_batch_status_build($projects = array(), $langcodes = array()) {
  $projects = $projects ? $projects : array_keys(l10n_update_get_projects());
  $langcodes = $langcodes ? $langcodes : array_keys(l10n_update_translatable_language_list());
  $options = _l10n_update_default_update_options();

  $operations = _l10n_update_batch_status_operations($projects, $langcodes, $options);

  $batch = array(
    'operations' => $operations,
    'title' => t('Checking translations'),
    'progress_message' => '',
    'finished' => 'l10n_update_batch_status_finished',
    'error_message' => t('Error checking translation updates.'),
    'file' => drupal_get_path('module', 'l10n_update') . '/l10n_update.batch.inc',
  );
  return $batch;
}

/**
 * Helper function to construct batch operations checking remote translation
 * status.
 *
 * @param array $projects
 *   Array of project names to be processed.
 * @param array $langcodes
 *   Array of language codes.
 * @param array $options
 *   Batch processing options.
 *
 * @return array
 *   Array of batch operations.
 */
function _l10n_update_batch_status_operations($projects, $langcodes, $options = array()) {
  $operations = array();

  foreach ($projects as $project) {
    foreach ($langcodes as $langcode) {
      // Check status of local and remote translation sources.
      $operations[] = array('l10n_update_batch_status_check', array($project, $langcode, $options));
    }
  }

  return $operations;
}

/**
 * Check and store the status and timestamp of local po files.
 *
 * Only po files in the local file system are checked. Any remote translation
 * files will be ignored.
 *
 * Projects may contain a server_pattern option containing a pattern of the
 * path to the po source files. If no server_pattern is defined the default
 * translation directory is checked for the po file. When a server_pattern is
 * defined the specified location is checked. The server_pattern can be set in
 * the module's .info.yml file or by using
 * hook_l10n_update_projects_alter().
 *
 * @param array $projects
 *   Array of project names for which to check the state of translation files.
 *   Defaults to all translatable projects.
 * @param array $langcodes
 *   Array of language codes. Defaults to all translatable languages.
 */
function l10n_update_check_projects_local($projects = array(), $langcodes = array()) {
  $projects = l10n_update_get_projects($projects);
  $langcodes = $langcodes ? $langcodes : array_keys(l10n_update_translatable_language_list());

  // For each project and each language we check if a local po file is
  // available. When found the source object is updated with the appropriate
  // type and timestamp of the po file.
  foreach ($projects as $name => $project) {
    foreach ($langcodes as $langcode) {
      $source = l10n_update_source_build($project, $langcode);
      if ($file = l10n_update_source_check_file($source)) {
        l10n_update_status_save($name, $langcode, L10N_UPDATE_LOCAL, $file);
      }
    }
  }
}
