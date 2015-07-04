<?php

/**
 * @file
 * Contains commerce_license_handler_field_access_details.
 */

/**
 * Displays the access details of activated licenses.
 */
class commerce_license_handler_field_access_details extends entity_views_handler_field_text {

  /**
   * Render a single field value.
   */
  public function render_single_value($value, $values) {
    return commerce_license_get_access_details($value);
  }

}
