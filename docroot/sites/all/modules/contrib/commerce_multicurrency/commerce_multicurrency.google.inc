<?php

/**
 * @file
 * Default currency sync callback
 */

/**
 * Fetch the currency exchange rates for the requested currency combination.
 *
 * Return an array with the array(target_currency_code => rate) combination.
 *
 * @param string $currency_code
 *   Source currency code.
 * @param array $target_currencies
 *   Array with the target currency codes.
 *
 * @return array
 *   Array with the array(target_currency_code => rate) combination.
 */
function commerce_multicurrency_exchange_rate_sync_provider_google($currency_code, $target_currencies) {
  $rates = array();
  foreach ($target_currencies as $target_currency_code) {
    $result = drupal_http_request('http://rate-exchange.appspot.com/currency?from=' . $currency_code . '&to=' . $target_currency_code);
    if ($result->code == 200) {
      $result_data = drupal_json_decode($result->data);
      $rates[$target_currency_code] = $result_data['rate'];
    }
    else {
      watchdog(
        'commerce_multicurrency', 'Rate provider Google: Unable to fetch / process the currency data and returned !response',
        array('!response' => print_r($result, TRUE)),
        WATCHDOG_ERROR
      );
    }
  }
  return $rates;
}
