<?php

/**
 * @file
 * Definition of StringDatabaseStorage.
 */

/**
 * Defines the locale string class.
 *
 * This is the base class for SourceString and TranslationString.
 */
class StringDatabaseStorage implements StringStorageInterface {

  /**
   * Additional database connection options to use in queries.
   *
   * @var array
   */
  protected $options = array();

  /**
   * Constructs a new StringStorage controller.
   *
   * @param array $options
   *   (optional) Any additional database connection options to use in queries.
   */
  public function __construct(array $options = array()) {
    $this->options = $options;
  }

  /**
   * Implements StringStorageInterface::getStrings().
   */
  public function getStrings(array $conditions = array(), array $options = array()) {
    return $this->dbStringLoad($conditions, $options, 'SourceString');
  }

  /**
   * Implements StringStorageInterface::getTranslations().
   */
  public function getTranslations(array $conditions = array(), array $options = array()) {
    return $this->dbStringLoad($conditions, array('translation' => TRUE) + $options, 'TranslationString');
  }

  /**
   * Implements StringStorageInterface::findString().
   */
  public function findString(array $conditions) {
    $values = $this->dbStringSelect($conditions)
      ->execute()
      ->fetchAssoc();

    if (!empty($values)) {
      $string = new SourceString($values);
      $string->setStorage($this);
      return $string;
    }
  }

  /**
   * Implements StringStorageInterface::findTranslation().
   */
  public function findTranslation(array $conditions) {
    $values = $this->dbStringSelect($conditions, array('translation' => TRUE))
      ->execute()
      ->fetchAssoc();

    if (!empty($values)) {
      $string = new TranslationString($values);
      $this->checkVersion($string, VERSION);
      $string->setStorage($this);
      return $string;
    }
  }

  /**
   * Implements StringStorageInterface::countStrings().
   */
  public function countStrings() {
    return $this->dbExecute("SELECT COUNT(*) FROM {locales_source}")->fetchField();
  }

  /**
   * Implements StringStorageInterface::countTranslations().
   */
  public function countTranslations() {
    return $this->dbExecute("SELECT t.language, COUNT(*) AS translated FROM {locales_source} s INNER JOIN {locales_target} t ON s.lid = t.lid GROUP BY t.language")->fetchAllKeyed();
  }

  /**
   * Implements StringStorageInterface::save().
   */
  public function save($string) {
    if ($string->isNew()) {
      $result = $this->dbStringInsert($string);
      if ($string->isSource() && $result) {
        // Only for source strings, we set the locale identifier.
        $string->setId($result);
      }
      $string->setStorage($this);
    }
    else {
      $this->dbStringUpdate($string);
    }
    return $this;
  }

  /**
   * Checks whether the string version matches a given version, fix it if not.
   *
   * @param StringInterface $string
   *   The string object.
   * @param string $version
   *   Drupal version to check against.
   */
  protected function checkVersion($string, $version) {
    if ($string->getId() && $string->getVersion() != $version) {
      $string->setVersion($version);
      db_update('locales_source', $this->options)
      ->condition('lid', $string->getId())
      ->fields(array('version' => $version))
      ->execute();
    }
  }

  /**
   * Implements StringStorageInterface::delete().
   */
  public function delete($string) {
    if ($keys = $this->dbStringKeys($string)) {
      $this->dbDelete('locales_target', $keys)->execute();
      if ($string->isSource()) {
        $this->dbDelete('locales_source', $keys)->execute();
        $this->dbDelete('locales_location', $keys)->execute();
        $string->setId(NULL);
      }
    }
    else {
      throw new StringStorageException(format_string('The string cannot be deleted because it lacks some key fields: @string', array(
        '@string' => $string->getString()
      )));
    }
    return $this;
  }

  /**
   * Implements StringStorageInterface::deleteLanguage().
   */
  public function deleteStrings($conditions) {
    $lids = $this->dbStringSelect($conditions, array('fields' => array('lid')))->execute()->fetchCol();
    if ($lids) {
      $this->dbDelete('locales_target', array('lid' => $lids))->execute();
      $this->dbDelete('locales_source',  array('lid' => $lids))->execute();
      $this->dbDelete('locales_location',  array('sid' => $lids))->execute();
    }
  }

  /**
   * Implements StringStorageInterface::deleteLanguage().
   */
  public function deleteTranslations($conditions) {
    $this->dbDelete('locales_target', $conditions)->execute();
  }

  /**
   * Implements StringStorageInterface::createString().
   */
  public function createString($values = array()) {
    return new SourceString($values + array('storage' => $this));
  }

  /**
   * Implements StringStorageInterface::createTranslation().
   */
  public function createTranslation($values = array()) {
    return new TranslationString($values + array(
      'storage' => $this,
      'is_new' => TRUE
    ));
  }

  /**
   * Gets table alias for field.
   *
   * @param string $field
   *   Field name to find the table alias for.
   *
   * @return string
   *   Either 's', 't' or 'l' depending on whether the field belongs to source,
   *   target or location table table.
   */
  protected function dbFieldTable($field) {
    if (in_array($field, array('language', 'translation', 'customized'))) {
      return 't';
    }
    elseif (in_array($field, array('type', 'name'))) {
      return 'l';
    }
    else {
      return 's';
    }
  }

  /**
   * Gets table name for storing string object.
   *
   * @param StringInterface $string
   *   The string object.
   *
   * @return string
   *   The table name.
   */
  protected function dbStringTable($string) {
    if ($string->isSource()) {
      return 'locales_source';
    }
    elseif ($string->isTranslation()) {
      return 'locales_target';
    }
  }

  /**
   * Gets keys values that are in a database table.
   *
   * @param StringInterface $string
   *   The string object.
   *
   * @return array
   *   Array with key fields if the string has all keys, or empty array if not.
   */
  protected function dbStringKeys($string) {
    if ($string->isSource()) {
      $keys = array('lid');
    }
    elseif ($string->isTranslation()) {
      $keys = array('lid', 'language');
    }
    if (!empty($keys) && ($values = $string->getValues($keys)) && count($keys) == count($values)) {
      return $values;
    }
    else {
      return array();
    }
  }

  /**
   * Loads multiple string objects.
   *
   * @param array $conditions
   *   Any of the conditions used by dbStringSelect().
   * @param array $options
   *   Any of the options used by dbStringSelect().
   * @param string $class
   *   Class name to use for fetching returned objects.
   *
   * @return array
   *   Array of objects of the class requested.
   */
  protected function dbStringLoad(array $conditions, array $options, $class) {
    $strings = array();
    $result = $this->dbStringSelect($conditions, $options)->execute();
    foreach ($result as $item) {
      $string = new $class($item);
      $string->setStorage($this);
      $strings[] = $string;
    }
    return $strings;
  }

  /**
   * Builds a SELECT query with multiple conditions and fields.
   *
   * The query uses both 'locales_source' and 'locales_target' tables.
   * Note that by default, as we are selecting both translated and untranslated
   * strings target field's conditions will be modified to match NULL rows too.
   *
   * @param array $conditions
   *   An associative array with field => value conditions that may include
   *   NULL values. If a language condition is included it will be used for
   *   joining the 'locales_target' table.
   * @param array $options
   *   An associative array of additional options. It may contain any of the
   *   options used by StringStorageInterface::getStrings() and these additional
   *   ones:
   *   - 'translation', Whether to include translation fields too. Defaults to
   *     FALSE.
   * @return SelectQuery
   *   Query object with all the tables, fields and conditions.
   */
  protected function dbStringSelect(array $conditions, array $options = array()) {
    // Change field 'customized' into 'l10n_status'. This enables the Drupal 8
    // backported code to work with the Drupal 7 style database tables.
    if (isset($conditions['customized'])) {
      $conditions['l10n_status'] = $conditions['customized'];
      unset($conditions['customized']);
    }
    if (isset($options['customized'])) {
      $options['l10n_status'] = $options['customized'];
      unset($options['customized']);
    }
    // Start building the query with source table and check whether we need to
    // join the target table too.
    $query = db_select('locales_source', 's', $this->options)
      ->fields('s');

    // Figure out how to join and translate some options into conditions.
    if (isset($conditions['translated'])) {
      // This is a meta-condition we need to translate into simple ones.
      if ($conditions['translated']) {
        // Select only translated strings.
        $join = 'innerJoin';
      }
      else {
        // Select only untranslated strings.
        $join = 'leftJoin';
        $conditions['translation'] = NULL;
      }
      unset($conditions['translated']);
    }
    else {
      $join = !empty($options['translation']) ? 'leftJoin' : FALSE;
    }

    if ($join) {
      if (isset($conditions['language'])) {
        // If we've got a language condition, we use it for the join.
        $query->$join('locales_target', 't', "t.lid = s.lid AND t.language = :langcode", array(
          ':langcode' => $conditions['language']
        ));
        unset($conditions['language']);
      }
      else {
        // Since we don't have a language, join with locale id only.
        $query->$join('locales_target', 't', "t.lid = s.lid");
      }
      if (!empty($options['translation'])) {
        // We cannot just add all fields because 'lid' may get null values.
        $query->addField('t', 'language');
        $query->addField('t', 'translation');
        $query->addField('t', 'l10n_status', 'customized');
      }
    }

    // If we have conditions for location's type or name, then we need the
    // location table, for which we add a subquery.
    if (isset($conditions['type']) || isset($conditions['name'])) {
      $subquery = db_select('locales_location', 'l', $this->options)
        ->fields('l', array('sid'));
      foreach (array('type', 'name') as $field) {
        if (isset($conditions[$field])) {
          $subquery->condition('l.' . $field, $conditions[$field]);
          unset($conditions[$field]);
        }
      }
      $query->condition('s.lid', $subquery, 'IN');
    }

    // Add conditions for both tables.
    foreach ($conditions as $field => $value) {
      $table_alias = $this->dbFieldTable($field);
      $field_alias = $table_alias . '.' . $field;
      if (is_null($value)) {
        $query->isNull($field_alias);
      }
      elseif ($table_alias == 't' && $join === 'leftJoin') {
        // Conditions for target fields when doing an outer join only make
        // sense if we add also OR field IS NULL.
        $query->condition(db_or()
            ->condition($field_alias, $value)
            ->isNull($field_alias)
        );
      }
      else {
        $query->condition($field_alias, $value);
      }
    }

    // Process other options, string filter, query limit, etc...
    if (!empty($options['filters'])) {
      if (count($options['filters']) > 1) {
        $filter = db_or();
        $query->condition($filter);
      }
      else {
        // If we have a single filter, just add it to the query.
        $filter = $query;
      }
      foreach ($options['filters'] as $field => $string) {
        $filter->condition($this->dbFieldTable($field) . '.' . $field, '%' . db_like($string) . '%', 'LIKE');
      }
    }

    if (!empty($options['pager limit'])) {
      $query = $query->extend('PagerDefault')->limit($options['pager limit']);
    }

    return $query;
  }

  /**
   * Createds a database record for a string object.
   *
   * @param StringInterface $string
   *   The string object.
   *
   * @return bool|int
   *   If the operation failed, returns FALSE.
   *   If it succeeded returns the last insert ID of the query, if one exists.
   *
   * @throws StringStorageException
   *   If the string is not suitable for this storage, an exception ithrown.
   */
  protected function dbStringInsert($string) {
    if ($string->isSource()) {
      $string->setValues(array('context' => '', 'version' => 'none'), FALSE);
      $fields = $string->getValues(array('source', 'context', 'version', 'textgroup'));
    }
    elseif ($string->isTranslation()) {
      $string->setValues(array('customized' => 0), FALSE);
      $fields = $string->getValues(array('lid', 'language', 'translation', 'customized'));
    }
    if (!empty($fields)) {
      // Change field 'customized' into 'l10n_status'. This enables the Drupal 8
      // backported code to work with the Drupal 7 style database tables.
      if (isset($fields['customized'])) {
        $fields['l10n_status'] = $fields['customized'];
        unset($fields['customized']);
      }

      return db_insert($this->dbStringTable($string), $this->options)
        ->fields($fields)
        ->execute();
    }
    else {
      throw new StringStorageException(format_string('The string cannot be saved: @string', array(
          '@string' => $string->getString()
      )));
    }
  }

  /**
   * Updates string object in the database.
   *
   * @param StringInterface $string
   *   The string object.
   *
   * @return bool|int
   *   If the record update failed, returns FALSE. If it succeeded, returns
   *   SAVED_NEW or SAVED_UPDATED.
   *
   * @throws StringStorageException
   *   If the string is not suitable for this storage, an exception is thrown.
   */
  protected function dbStringUpdate($string) {
    if ($string->isSource()) {
      $values = $string->getValues(array('source', 'context', 'version'));
    }
    elseif ($string->isTranslation()) {
      $values = $string->getValues(array('translation', 'customized'));
    }
    if (!empty($values) && $keys = $this->dbStringKeys($string)) {
      // Change field 'customized' into 'l10n_status'. This enables the Drupal 8
      // backported code to work with the Drupal 7 style database tables.
      if (isset($keys['customized'])) {
        $keys['l10n_status'] = $keys['customized'];
        unset($keys['customized']);
      }
      if (isset($values['customized'])) {
        $values['l10n_status'] = $values['customized'];
        unset($values['customized']);
      }

      return db_merge($this->dbStringTable($string), $this->options)
        ->key($keys)
        ->fields($values)
        ->execute();
    }
    else {
      throw new StringStorageException(format_string('The string cannot be updated: @string', array(
          '@string' => $string->getString()
      )));
    }
  }

  /**
   * Creates delete query.
   *
   * @param string $table
   *   The table name.
   * @param array $keys
   *   Array with object keys indexed by field name.
   *
   * @return DeleteQuery
   *   Returns a new DeleteQuery object for the active database.
   */
  protected function dbDelete($table, $keys) {
    $query = db_delete($table, $this->options);
    // Change field 'customized' into 'l10n_status'. This enables the Drupal 8
    // backported code to work with the Drupal 7 style database tables.
    if (isset($keys['customized'])) {
      $keys['l10n_status'] = $keys['customized'];
      unset($keys['customized']);
    }

    foreach ($keys as $field => $value) {
      $query->condition($field, $value);
    }
    return $query;
  }

  /**
   * Executes an arbitrary SELECT query string.
   */
  protected function dbExecute($query, array $args = array()) {
    return db_query($query, $args, $this->options);
  }
}
