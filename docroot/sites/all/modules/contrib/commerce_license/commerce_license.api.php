<?php

/**
 * @file
 * Hooks provided by the Commerce License module.
 */

/**
 * Alter the list of available license types shown on the product form.
 *
 * Allows modules to make their license types available only to certain
 * licensable product types.
 *
 * @param $types
 *   List of license types.
 * @param $product
 *   The parent product.
 */
function hook_commerce_license_types_list_alter(&$types, $product) {
  if (!in_array($product->type, commerce_file_product_types())) {
    unset($types['file']);
  }
}
