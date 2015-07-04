<?php
/**
 * @file
 * Entity base controller for EntityBundlePlugin.
 */

class EntityBundlePluginEntityController extends EntityAPIController {

  /**
   * Overrides EntityAPIController::query().
   */
  public function query($ids, $conditions, $revision_id = FALSE) {
    $query = $this->buildQuery($ids, $conditions, $revision_id);
    $result = $query->execute();
    $result->setFetchMode(PDO::FETCH_ASSOC);

    // Build the resulting objects ourselves, since the standard PDO ways of
    // doing that are completely useless.
    $objects = array();
    foreach ($result as $row) {
      $row['is_new'] = FALSE;
      $objects[] = $this->create($row);
    }
    return $objects;
  }

  /**
   * Overrides EntityAPIController::create().
   */
  public function create(array $values = array()) {
    if (!isset($values[$this->entityInfo['entity keys']['bundle']])) {
      throw new Exception(t('No bundle provided to EntityBundlePluginEntityController::create().'));
    }

    // Add is_new property if it is not set.
    $values += array('is_new' => TRUE);
    ctools_include('plugins');
    $class = ctools_plugin_load_class($this->entityInfo['module'],
      $this->entityInfo['bundle plugin']['plugin type'],
      $values[$this->entityInfo['entity keys']['bundle']],
      'class');
    if (!class_exists($class)) {
      if (!empty($this->entityInfo['bundle plugin']['broken class'])) {
        $class = $this->entityInfo['bundle plugin']['broken class'];
      }
      else {
        throw new Exception(t('Trying to load an object with a broken plugin type @type', array('@type' => $values[$this->entityInfo['entity keys']['bundle']])));
      }
    }

    return new $class($values, $this->entityType);
  }
}

class EntityBundlePluginEntityControllerExportable extends EntityAPIControllerExportable {

  /**
   * Overrides EntityAPIControllerExportable::query().
   */
  public function query($ids, $conditions, $revision_id = FALSE) {
    $query = $this->buildQuery($ids, $conditions, $revision_id);
    $result = $query->execute();
    $result->setFetchMode(PDO::FETCH_ASSOC);

    // Build the resulting objects ourselves, since the standard PDO ways of
    // doing that are completely useless.
    $objects = array();
    foreach ($result as $row) {
      $row['is_new'] = FALSE;
      $objects[] = $this->create($row);
    }
    return $objects;
  }

  /**
   * Overrides EntityAPIControllerExportable::create().
   */
  public function create(array $values = array()) {
    if (!isset($values[$this->entityInfo['entity keys']['bundle']])) {
      throw new Exception(t('No bundle provided to EntityBundlePluginEntityControllerExportable::create().'));
    }

    // Add is_new property if it is not set.
    $values += array('is_new' => TRUE);
    ctools_include('plugins');
    $class = ctools_plugin_load_class($this->entityInfo['module'],
      $this->entityInfo['bundle plugin']['plugin type'],
      $values[$this->entityInfo['entity keys']['bundle']],
      'class');
    if (!class_exists($class)) {
      if (!empty($this->entityInfo['bundle plugin']['broken class'])) {
        $class = $this->entityInfo['bundle plugin']['broken class'];
      }
      else {
        throw new Exception(t('Trying to load an object with a broken plugin type @type', array('@type' => $values[$this->entityInfo['entity keys']['bundle']])));
      }
    }

    return new $class($values, $this->entityType);
  }
}

/**
 * This plugin implements a validation method for instantiation.
 */
interface EntityBundlePluginValidableInterface {

  /**
   * Defines is this plugin may be used as the bundle for an entity.
   *
   * It is meant to be use when the ability to use the plugin as a
   * bundle depends on run-time information, like the availability of
   * a given module.
   *
   * @return bool
   *   Is the plugin usable as a bundle ?
   */
  public static function isValid();
}

interface EntityBundlePluginProvideFieldsInterface {

  /**
   * Returns the fields that are going to be used by this entity bundle.
   *
   * The definition of the fields must be strictly compatible: a field of
   * defining two fields with the same name must have identical field
   * definitions.
   *
   * @return array
   *   An associative array of field definitions, keyed by field name.
   *   Each field definition is an associated array with two keys:
   *    - 'field': the field definition, as per field_create_field();
   *    - 'instance': the instance definition, as per field_create_instance().
   */
  static function fields();
}
