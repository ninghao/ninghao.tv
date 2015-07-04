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
function commerce_multicurrency_exchange_rate_sync_provider_ecb($currency_code, $target_currencies) {
  $data = cache_get(__FUNCTION__, 'cache');

  if (!$data) {
    $ecb_rates = array();
    if (($xml = @simplexml_load_file('http://www.ecb.europa.eu/stats/eurofxref/eurofxref-daily.xml')) && @count($xml->Cube->Cube->Cube)) {
      foreach ($xml->Cube->Cube->Cube as $rate) {
        $ecb_rates[(string) $rate["currency"]] = (string) $rate["rate"];
      }
      // Cache six hours.
      cache_set(__FUNCTION__, $ecb_rates, 'cache', time() + (3600 * 6));
    }
    else {
      watchdog(
        'commerce_multicurrency', 'Rate provider ECB: Unable to fetch / process the currency data of @url',
        array('@url' => 'http://www.ecb.europa.eu/stats/eurofxref/eurofxref-daily.xml'),
        WATCHDOG_ERROR
      );
    }
  }
  else {
    $ecb_rates = $data->data;
  }

  $rates = array();
  foreach ($target_currencies as $target_currency_code) {
    if ($currency_code == 'EUR' && isset($ecb_rates[$target_currency_code])) {
      $rates[$target_currency_code] = $ecb_rates[$target_currency_code];
    }
    elseif (isset($ecb_rates[$currency_code]) && $target_currency_code == 'EUR') {
      // Reverse rate calculation.
      $rates[$target_currency_code] = 1 / $ecb_rates[$currency_code];
    }
    elseif (isset($ecb_rates[$currency_code]) && isset($ecb_rates[$target_currency_code])) {
      // Cross rate calculation.
      $rates[$target_currency_code] = $ecb_rates[$target_currency_code] / $ecb_rates[$currency_code];
    }
  }
  return $rates;
}
