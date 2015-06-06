<?php

/**
 * @file
 * Definition of TranslationString.
 */

/**
 * Defines the locale translation string object.
 *
 * This class represents a translation of a source string to a given language,
 * thus it must have at least a 'language' which is the language code and a
 * 'translation' property which is the translated text of the the source string
 * in the specified language.
 */
class TranslationString extends StringBase {
  /**
   * The language code.
   *
   * @var string
   */
  public $language;

  /**
   * The string translation.
   *
   * @var string
   */
  public $translation;

  /**
   * Integer indicating whether this string is customized.
   *
   * @var int
   */
  public $customized;

  /**
   * Boolean indicating whether the string object is new.
   *
   * @var bool
   */
  protected $is_new;

  /**
   * Overrides StringBase::__construct().
   */
  public function __construct($values = array()) {
    parent::__construct($values);
    if (!isset($this->is_new)) {
      // We mark the string as not new if it is a complete translation.
      // This will work when loading from database, otherwise the storage
      // controller that creates the string object must handle it.
      $this->is_new = !$this->isTranslation();
    }
  }

  /**
   * Sets the string as customized / not customized.
   *
   * @param bool $customized
   *   (optional) Whether the string is customized or not. Defaults to TRUE.
   *
   * @return TranslationString
   *   The called object.
   */
  public function setCustomized($customized = TRUE) {
    $this->customized = $customized ? L10N_UPDATE_CUSTOMIZED : L10N_UPDATE_NOT_CUSTOMIZED;
    return $this;
  }

  /**
   * Implements StringInterface::isSource().
   */
  public function isSource() {
    return FALSE;
  }

  /**
   * Implements StringInterface::isTranslation().
  */
  public function isTranslation() {
    return !empty($this->lid) && !empty($this->language) && isset($this->translation);
  }

  /**
   * Implements StringInterface::getString().
   */
  public function getString() {
    return isset($this->translation) ? $this->translation : '';
  }

  /**
   * Implements StringInterface::setString().
   */
  public function setString($string) {
    $this->translation = $string;
    return $this;
  }

  /**
   * Implements StringInterface::isNew().
   */
  public function isNew() {
    return $this->is_new;
  }

  /**
   * Implements StringInterface::save().
   */
  public function save() {
    parent::save();
    $this->is_new = FALSE;
    return $this;
  }

  /**
   * Implements StringInterface::delete().
   */
  public function delete() {
    parent::delete();
    $this->is_new = TRUE;
    return $this;
  }

}
