<?php

/**
 * @file
 * Common API for interface translation.
 */

/**
 * Comparison result of source files timestamps.
 *
 * Timestamp of source 1 is less than the timestamp of source 2.
 * @see _l10n_update_source_compare()
 */
define('L10N_UPDATE_SOURCE_COMPARE_LT', -1);

/**
 * Comparison result of source files timestamps.
 *
 * Timestamp of source 1 is equal to the timestamp of source 2.
 * @see _l10n_update_source_compare()
 */
define('L10N_UPDATE_SOURCE_COMPARE_EQ', 0);

/**
 * Comparison result of source files timestamps.
 *
 * Timestamp of source 1 is greater than the timestamp of source 2.
 * @see _l10n_update_source_compare()
 */
define('L10N_UPDATE_SOURCE_COMPARE_GT', 1);

/**
 * Get array of projects which are available for interface translation.
 *
 * This project data contains all projects which will be checked for available
 * interface translations.
 *
 * For full functionality this function depends on Update module.
 * When Update module is enabled the project data will contain the most recent
 * module status; both in enabled status as in version. When Update module is
 * disabled this function will return the last known module state. The status
 * will only be updated once Update module is enabled.
 *
 *  @params array $project_names
 *    Array of names of the projects to get.
 *
 * @return array
 *   Array of project data for translation update.
 *
 * @see l10n_update_build_projects()
 */
function l10n_update_get_projects($project_names = array()) {
  $projects = &drupal_static(__FUNCTION__, array());

  if (empty($projects)) {
    // Get project data from the database.
    $result = db_query('SELECT name, project_type, core, version, l10n_path as server_pattern, status FROM {l10n_update_project}');
    // http://drupal.org/node/1777106 is a follow-up issue to make the check for
    // possible out-of-date project information more robust.
    if ($result->rowCount() == 0) {
      module_load_include('compare.inc', 'l10n_update');
      // At least the core project should be in the database, so we build the
      // data if none are found.
      l10n_update_build_projects();
      $result = db_query('SELECT name, project_type, core, version, l10n_path as server_pattern, status FROM {l10n_update_project}');
    }

    foreach ($result as $project) {
      $projects[$project->name] = $project;
    }
  }

  // Return the requested project names or all projects.
  if ($project_names) {
    return array_intersect_key($projects, drupal_map_assoc($project_names));
  }
  return $projects;
}

/**
 * Clears the projects cache.
 */
function l10n_update_clear_cache_projects() {
  drupal_static('l10n_update_get_projects', array());
}

/**
 * Loads cached translation sources containing current translation status.
 *
 * @param array $projects
 *   Array of project names. Defaults to all translatable projects.
 * @param array $langcodes
 *   Array of language codes. Defaults to all translatable languages.
 *
 * @return array
 *   Array of source objects. Keyed with <project name>:<language code>.
 *
 * @see l10n_update_source_build()
 */
function l10n_update_load_sources($projects = NULL, $langcodes = NULL) {
  $sources = array();
  $projects = $projects ? $projects : array_keys(l10n_update_get_projects());
  $langcodes = $langcodes ? $langcodes : array_keys(l10n_update_translatable_language_list());

  // Load source data from l10n_update_status cache.
  $status = l10n_update_get_status();

  // Use only the selected projects and languages for update.
  foreach($projects as $project) {
    foreach ($langcodes as $langcode) {
      $sources[$project][$langcode] = isset($status[$project][$langcode]) ? $status[$project][$langcode] : NULL;
    }
  }
  return $sources;
}

/**
 * Build translation sources.
 *
 * @param array $projects
 *   Array of project names. Defaults to all translatable projects.
 * @param array $langcodes
 *   Array of language codes. Defaults to all translatable languages.
 *
 * @return array
 *   Array of source objects. Keyed by project name and language code.
 *
 * @see l10n_update_source_build()
 */
function l10n_update_build_sources($projects = array(), $langcodes = array()) {
  $sources = array();
  $projects = l10n_update_get_projects($projects);
  $langcodes = $langcodes ? $langcodes : array_keys(l10n_update_translatable_language_list());

  foreach ($projects as $project) {
    foreach ($langcodes as $langcode) {
      $source = l10n_update_source_build($project, $langcode);
      $sources[$source->name][$source->langcode] = $source;
    }
  }
  return $sources;
}

/**
 * Checks whether a po file exists in the local filesystem.
 *
 * It will search in the directory set in the translation source. Which defaults
 * to the "translations://" stream wrapper path. The directory may contain any
 * valid stream wrapper.
 *
 * The "local" files property of the source object contains the definition of a
 * po file we are looking for. The file name defaults to
 * %project-%release.%language.po. Per project this value can be overridden
 * using the server_pattern directive in the module's .info.yml file or by using
 * hook_l10n_update_projects_alter().
 *
 * @param object $source
 *   Translation source object.
 *
 * @return stdClass
 *   Source file object of the po file, updated with:
 *   - "uri": File name and path.
 *   - "timestamp": Last updated time of the po file.
 *   FALSE if the file is not found.
 *
 * @see l10n_update_source_build()
 */
function l10n_update_source_check_file($source) {
  if (isset($source->files[L10N_UPDATE_LOCAL])) {
    $source_file = $source->files[L10N_UPDATE_LOCAL];
    $directory = $source_file->directory;
    $filename = '/^' . preg_quote($source_file->filename) . '$/';

    if ($files = file_scan_directory($directory, $filename, array('key' => 'name', 'recurse' => FALSE))) {
      $file = current($files);
      $source_file->uri = $file->uri;
      $source_file->timestamp = filemtime($file->uri);
      return $source_file;
    }
  }
  return FALSE;
}

/**
 * Builds abstract translation source.
 *
 * @param object $project
 *   Project object.
 * @param string $langcode
 *   Language code.
 * @param string $filename
 *   File name of translation file. May contain placeholders.
 *
 * @return object
 *   Source object:
 *   - "project": Project name.
 *   - "name": Project name (inherited from project).
 *   - "language": Language code.
 *   - "core": Core version (inherited from project).
 *   - "version": Project version (inherited from project).
 *   - "project_type": Project type (inherited from project).
 *   - "files": Array of file objects containing properties of local and remote
 *     translation files.
 *   Other processes can add the following properties:
 *   - "type": Most recent translation source found. L10N_UPDATE_REMOTE and
 *      L10N_UPDATE_LOCAL indicate available new translations,
 *      L10N_UPDATE_CURRENT indicate that the current translation is them
 *      most recent. "type" sorresponds with a key of the "files" array.
 *   - "timestamp": The creation time of the "type" translation (file).
 *   - "last_checked": The time when the "type" translation was last checked.
 *   The "files" array can hold file objects of type:
 *   L10N_UPDATE_LOCAL, L10N_UPDATE_REMOTE and
 *   L10N_UPDATE_CURRENT. Each contains following properties:
 *   - "type": The object type (L10N_UPDATE_LOCAL,
 *     L10N_UPDATE_REMOTE, etc. see above).
 *   - "project": Project name.
 *   - "langcode": Language code.
 *   - "version": Project version.
 *   - "uri": Local or remote file path.
 *   - "directory": Directory of the local po file.
 *   - "filename": File name.
 *   - "timestamp": Timestamp of the file.
 *   - "keep": TRUE to keep the downloaded file.
 */
function l10n_update_source_build($project, $langcode, $filename = NULL) {
  // Create a source object with data of the project object.
  $source = clone $project;
  $source->project = $project->name;
  $source->langcode = $langcode;
  $source->type = '';
  $source->timestamp = 0;
  $source->last_checked = 0;

  $filename = $filename ? $filename : variable_get('l10n_update_default_filename', L10N_UPDATE_DEFAULT_FILE_NAME);

  // If the server_pattern contains a remote file path we will check for a
  // remote file. The local version of this file will only be checked if a
  // translations directory has been defined. If the server_pattern is a local
  // file path we will only check for a file in the local file system.
  $files = array();
  if (_l10n_update_file_is_remote($source->server_pattern)) {
    $files[L10N_UPDATE_REMOTE] = (object) array(
      'project' => $project->name,
      'langcode' => $langcode,
      'version' => $project->version,
      'type' => L10N_UPDATE_REMOTE,
      'filename' => l10n_update_build_server_pattern($source, basename($source->server_pattern)),
      'uri' => l10n_update_build_server_pattern($source, $source->server_pattern),
    );
    $files[L10N_UPDATE_LOCAL] = (object) array(
      'project' => $project->name,
      'langcode' => $langcode,
      'version' => $project->version,
      'type' => L10N_UPDATE_LOCAL,
      'filename' => l10n_update_build_server_pattern($source, $filename),
      'directory' => 'translations://',
    );
    $files[L10N_UPDATE_LOCAL]->uri = $files[L10N_UPDATE_LOCAL]->directory . $files[L10N_UPDATE_LOCAL]->filename;
  }
  else {
    $files[L10N_UPDATE_LOCAL] = (object) array(
      'project' => $project->name,
      'langcode' => $langcode,
      'version' => $project->version,
      'type' => L10N_UPDATE_LOCAL,
      'filename' => l10n_update_build_server_pattern($source, basename($source->server_pattern)),
      'directory' => l10n_update_build_server_pattern($source, drupal_dirname($source->server_pattern)),
    );
    $files[L10N_UPDATE_LOCAL]->uri = $files[L10N_UPDATE_LOCAL]->directory . '/' . $files[L10N_UPDATE_LOCAL]->filename;
  }
  $source->files = $files;

  // If this project+language is already translated, we add its status and
  // update the current translation timestamp and last_updated time. If the
  // project+language is not translated before, create a new record.
  $history = l10n_update_get_file_history();
  if (isset($history[$project->name][$langcode]) && $history[$project->name][$langcode]->timestamp) {
    $source->files[L10N_UPDATE_CURRENT] = $history[$project->name][$langcode];
    $source->type = L10N_UPDATE_CURRENT;
    $source->timestamp = $history[$project->name][$langcode]->timestamp;
    $source->last_checked = $history[$project->name][$langcode]->last_checked;
  }
  else {
    l10n_update_update_file_history($source);
  }

  return $source;
}

/**
 * Build path to translation source, out of a server path replacement pattern.
 *
 * @param object $project
 *   Project object containing data to be inserted in the template.
 * @param string $template
 *   String containing placeholders. Available placeholders:
 *   - "%project": Project name.
 *   - "%release": Project version.
 *   - "%core": Project core version.
 *   - "%language": Language code.
 *
 * @return string
 *   String with replaced placeholders.
 */
function l10n_update_build_server_pattern($project, $template) {
  $variables = array(
    '%project' => $project->name,
    '%release' => $project->version,
    '%core' => $project->core,
    '%language' => isset($project->langcode) ? $project->langcode : '%language',
  );
  return strtr($template, $variables);
}

/**
 * Populate a queue with project to check for translation updates.
 */
function l10n_update_cron_fill_queue() {
  $updates = array();

  // Determine which project+language should be updated.
  $last = REQUEST_TIME - variable_get('l10n_update_check_frequency', '0') * 3600 * 24;
  $query = db_select('l10n_update_file', 'f');
  $query->join('l10n_update_project', 'p', 'p.name = f.project');
  $query->condition('f.last_checked', $last, '<');
  $query->fields('f', array('project', 'language'));
  // Only currently installed / enabled components should be checked for.
  $query->condition('p.status', 1);
  $files = $query->execute()->fetchAll();
  foreach ($files as $file) {
    $updates[$file->project][] = $file->language;

    // Update the last_checked timestamp of the project+language that will
    // be checked for updates.
    db_update('l10n_update_file')
      ->fields(array('last_checked' => REQUEST_TIME))
      ->condition('project', $file->project)
      ->condition('language', $file->language)
      ->execute();
  }

  // For each project+language combination a number of tasks are added to
  // the queue.
  if ($updates) {
    module_load_include('fetch.inc', 'l10n_update');
    $options = _l10n_update_default_update_options();
    $queue = DrupalQueue::get('l10n_update', TRUE);

    foreach ($updates as $project => $languages) {
      $batch = l10n_update_batch_update_build(array($project), $languages, $options);
      foreach ($batch['operations'] as $item) {
        $queue->createItem($item);
      }
    }
  }
}

/**
 * Determine if a file is a remote file.
 *
 * @param string $uri
 *   The URI or URI pattern of the file.
 *
 * @return boolean
 *   TRUE if the $uri is a remote file.
 */
function _l10n_update_file_is_remote($uri) {
  $scheme = file_uri_scheme($uri);
  if ($scheme) {
    return !drupal_realpath($scheme . '://');
  }
  return FALSE;
}

/**
 * Compare two update sources, looking for the newer one.
 *
 * The timestamp property of the source objects are used to determine which is
 * the newer one.
 *
 * @param object $source1
 *   Source object of the first translation source.
 * @param object $source2
 *   Source object of available update.
 *
 * @return integer
 *   - "L10N_UPDATE_SOURCE_COMPARE_LT": $source1 < $source2 OR $source1
 *     is missing.
 *   - "L10N_UPDATE_SOURCE_COMPARE_EQ":  $source1 == $source2 OR both
 *     $source1 and $source2 are missing.
 *   - "L10N_UPDATE_SOURCE_COMPARE_EQ":  $source1 > $source2 OR $source2
 *     is missing.
 */
function _l10n_update_source_compare($source1, $source2) {
  if (isset($source1->timestamp) && isset($source2->timestamp)) {
    if ($source1->timestamp == $source2->timestamp) {
      return L10N_UPDATE_SOURCE_COMPARE_EQ;
    }
    else {
      return $source1->timestamp > $source2->timestamp ? L10N_UPDATE_SOURCE_COMPARE_GT : L10N_UPDATE_SOURCE_COMPARE_LT;
    }
  }
  elseif (isset($source1->timestamp) && !isset($source2->timestamp)) {
    return L10N_UPDATE_SOURCE_COMPARE_GT;
  }
  elseif (!isset($source1->timestamp) && isset($source2->timestamp)) {
    return L10N_UPDATE_SOURCE_COMPARE_LT;
  }
  else {
    return L10N_UPDATE_SOURCE_COMPARE_EQ;
  }
}

/**
 * Returns default import options for translation update.
 *
 * @return array
 *   Array of translation import options.
 */
function _l10n_update_default_update_options() {
  $options = array(
    'customized' => L10N_UPDATE_NOT_CUSTOMIZED,
    'finish_feedback' => TRUE,
    'use_remote' => l10n_update_use_remote_source(),
  );

  switch (variable_get('l10n_update_import_mode', LOCALE_IMPORT_KEEP)) {
    case LOCALE_IMPORT_OVERWRITE:
      $options['overwrite_options'] = array(
        'customized' => TRUE,
        'not_customized' => TRUE,
      );
      break;
    case L10N_UPDATE_OVERWRITE_NON_CUSTOMIZED:
      $options['overwrite_options'] = array(
        'customized' => FALSE,
        'not_customized' => TRUE,
      );
      break;
    case LOCALE_IMPORT_KEEP:
      $options['overwrite_options'] = array(
        'customized' => FALSE,
        'not_customized' => FALSE,
      );
      break;
  }

  return $options;
}

/**
 * Import one string into the database.
 *
 * @param $report
 *   Report array summarizing the number of changes done in the form:
 *   array(inserts, updates, deletes).
 * @param $langcode
 *   Language code to import string into.
 * @param $context
 *   The context of this string.
 * @param $source
 *   Source string.
 * @param $translation
 *   Translation to language specified in $langcode.
 * @param $textgroup
 *   Name of textgroup to store translation in.
 * @param $location
 *   Location value to save with source string.
 * @param $mode
 *   Import mode to use, LOCALE_IMPORT_KEEP or LOCALE_IMPORT_OVERWRITE.
 * @param $status
 *   Status of translation if created: L10N_UPDATE_STRING_DEFAULT or L10N_UPDATE_STRING_CUSTOM
 * @param $plid
 *   Optional plural ID to use.
 * @param $plural
 *   Optional plural value to use.
 * @return
 *   The string ID of the existing string modified or the new string added.
 */
function _l10n_update_locale_import_one_string_db(&$report, $langcode, $context, $source, $translation, $textgroup, $location, $mode, $status = L10N_UPDATE_NOT_CUSTOMIZED, $plid = 0, $plural = 0) {
  $lid = db_query("SELECT lid FROM {locales_source} WHERE source = :source AND context = :context AND textgroup = :textgroup", array(':source' => $source, ':context' => $context, ':textgroup' => $textgroup))->fetchField();

  if (!empty($translation)) {
    // Skip this string unless it passes a check for dangerous code.
    // Text groups other than default still can contain HTML tags
    // (i.e. translatable blocks).
    if ($textgroup == "default" && !locale_string_is_safe($translation)) {
      $report['skips']++;
      $lid = 0;
      watchdog('locale', 'Disallowed HTML detected. String not imported: %string', array('%string' => $translation), WATCHDOG_WARNING);
    }
    elseif ($lid) {
      // We have this source string saved already.
      db_update('locales_source')
        ->fields(array(
          'location' => $location,
        ))
        ->condition('lid', $lid)
        ->execute();

      $exists = db_query("SELECT lid, l10n_status FROM {locales_target} WHERE lid = :lid AND language = :language", array(':lid' => $lid, ':language' => $langcode))->fetchObject();

      if (!$exists) {
        // No translation in this language.
        db_insert('locales_target')
          ->fields(array(
            'lid' => $lid,
            'language' => $langcode,
            'translation' => $translation,
            'plid' => $plid,
            'plural' => $plural,
          ))
          ->execute();

        $report['additions']++;
      }
      elseif (($exists->l10n_status == L10N_UPDATE_NOT_CUSTOMIZED && $mode == L10N_UPDATE_OVERWRITE_NON_CUSTOMIZED) || $mode == LOCALE_IMPORT_OVERWRITE) {
        // Translation exists, only overwrite if instructed.
        db_update('locales_target')
          ->fields(array(
            'translation' => $translation,
            'plid' => $plid,
            'plural' => $plural,
          ))
          ->condition('language', $langcode)
          ->condition('lid', $lid)
          ->execute();

        $report['updates']++;
      }
    }
    else {
      // No such source string in the database yet.
      $lid = db_insert('locales_source')
        ->fields(array(
          'location' => $location,
          'source' => $source,
          'context' => (string) $context,
          'textgroup' => $textgroup,
        ))
        ->execute();

      db_insert('locales_target')
        ->fields(array(
          'lid' => $lid,
          'language' => $langcode,
          'translation' => $translation,
          'plid' => $plid,
          'plural' => $plural,
          'l10n_status' => $status,
        ))
        ->execute();

      $report['additions']++;
    }
  }
  elseif ($mode == LOCALE_IMPORT_OVERWRITE) {
    // Empty translation, remove existing if instructed.
    db_delete('locales_target')
      ->condition('language', $langcode)
      ->condition('lid', $lid)
      ->condition('plid', $plid)
      ->condition('plural', $plural)
      ->execute();

    $report['deletes']++;
  }

  return $lid;
}
