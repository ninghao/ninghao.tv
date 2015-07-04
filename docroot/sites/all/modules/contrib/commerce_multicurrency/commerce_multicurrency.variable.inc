<?php
/**
 * @file
 * Commerce variables.
 */

/**
 * Implements hook_variable_info().
 */
function commerce_multicurrency_variable_info($options) {

  $currencies = array();
  foreach (commerce_currencies(TRUE, TRUE) as $currency_code => $currency) {
    $currencies[$currency_code] = t(
      '@code - !name',
      array(
        '@code' => $currency['code'],
        '@symbol' => $currency['symbol'],
        '!name' => $currency['name'],
      )
    );

    if (!empty($currency['symbol'])) {
      $currencies[$currency_code] .= ' - ' . check_plain($currency['symbol']);
    }
  }

  $variable['commerce_default_currency'] = array(
    'type' => 'select',
    'title' => t('Default store currency'),
    'description' => t('The default store currency will be used as the default for all price fields.'),
    'multidomain' => TRUE,
    'token' => TRUE,
    'options' => $currencies,
    'default_value' => commerce_default_currency(),
  );

  return $variable;
}
