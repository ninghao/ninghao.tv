<?php

/**
 * @file
 * Definition of SourceString.
 */

/**
 * Defines the locale source string object.
 *
 * This class represents a module-defined string value that is to be translated.
 * This string must at least contain a 'source' field, which is the raw source
 * value, and is assumed to be in English language.
 */
class SourceString extends StringBase {
  /**
   * Implements StringInterface::isSource().
   */
  public function isSource() {
    return isset($this->source);
  }

  /**
   * Implements StringInterface::isTranslation().
   */
  public function isTranslation() {
    return FALSE;
  }

  /**
   * Implements LocaleString::getString().
   */
  public function getString() {
    return isset($this->source) ? $this->source : '';
  }

  /**
   * Implements LocaleString::setString().
   */
  public function setString($string) {
    $this->source = $string;
    return $this;
  }

  /**
   * Implements LocaleString::isNew().
   */
  public function isNew() {
    return empty($this->lid);
  }

}
