<?php

/**
 * @file
 * Definition of Drupal\Component\Gettext\PoReaderInterface.
 */

/**
 * Shared interface definition for all Gettext PO Readers.
 */
interface PoReaderInterface extends PoMetadataInterface {

  /**
   * Reads and returns a PoItem (source/translation pair).
   *
   * @return PoItem
   *   Wrapper for item data instance.
   */
  public function readItem();

}
