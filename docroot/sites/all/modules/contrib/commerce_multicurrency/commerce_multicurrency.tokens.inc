<?php
/**
 * @file
 * Token callbacks for the commerce_multicurrency module.
 */

/**
 * Implements hook_token_info().
 */
function commerce_multicurrency_token_info() {
  $tokens['commerce-currency-code'] = array(
    'name' => t('The user currency'),
    'description' => t('The currence code of the currency to use for the current user.'),
  );

  return array(
    'tokens' => array('user' => $tokens),
  );
}

/**
 * Implements hook_tokens().
 */
function commerce_multicurrency_tokens($type, $tokens, array $data = array(), array $options = array()) {
  $replacements = array();
  if ($type == 'user') {
    if (isset($tokens['commerce-currency-code'])) {
      $replacements[$tokens['commerce-currency-code']] = commerce_multicurrency_get_user_currency_code();
    }
  }
  return $replacements;
}
