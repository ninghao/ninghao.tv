<?php
/**
 * @file
 * This file contains no working PHP code; it exists to provide additional
 * documentation for doxygen as well as to document hooks in the standard
 * Drupal manner.
 */

/**
 * Defines currency exchange rate sync providers.
 *
 * @return array
 *   An array of information about the callback.
 *   The array contains a sub-array for each callback, with a machine name
 *   as the key.
 *   Possible attributes for each sub-array are:
 *   - title: The human readable title displayed in the settings form. Should
 *      be wrapped in t().
 *   - callback: The function to call.
 *      The function will recive the source currency as first and an array of
 *      target currencies as second parameter.
 *      As return an assiocative array keyed by the currency code with the rate
 *      as value is expected.
 *   - file: Optional. A file to include.
 */
function hook_commerce_multicurrency_exchange_rate_sync_provider_info() {
  return array(
    'ecb' => array(
      'title' => t('European Central Bank'),
      'callback' => 'commerce_multicurrency_exchange_rate_sync_provider_ecb',
      'file' => drupal_get_path('module', 'commerce_multicurrency') . '/commerce_multicurrency.ecb.inc',
    ),
  );
}

/**
 * Alter currency exchange rate sync providers.
 *
 * This hook allows you to change the formatting properties of existing
 * definitions.
 *
 * @see hook_commerce_multicurrency_exchange_rate_sync_provider_info()
 */
function hook_commerce_multicurrency_exchange_rate_sync_provider_info_alter(&$providers) {
  $currencies['ecb']['callback'] = 'commerce_multicurrency_exchange_rate_sync_provider_ecb_different';
}
