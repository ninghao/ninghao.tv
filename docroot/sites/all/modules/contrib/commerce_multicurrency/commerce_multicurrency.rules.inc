<?php
/**
 * @file
 * Commerce Multicurrency rules integration.
 */

/**
 * Implements hook_rules_event_info().
 */
function commerce_multicurrency_rules_event_info() {
  $events = array();

  $events['commerce_multicurrency_user_currency_set'] = array(
    'label' => t('After the user currency has been set'),
    'group' => t('Commerce Multicurrency'),
    'variables' => array(
      'currency_code' => array(
        'type' => 'text',
        'label' => t('The currency code to use'),
      ),
      'old_currency_code' => array(
        'type' => 'text',
        'label' => t('The currency code used before'),
      ),
    ),
    'access callback' => 'commerce_order_rules_access',
  );

  $events['commerce_multicurrency_set_display_price'] = array(
    'label' => t('Set the display price'),
    'group' => t('Commerce Multicurrency'),
    'variables' => array(
      'commerce_line_item' => array(
        'label' => t('Product line item'),
        'type' => 'commerce_line_item',
        'skip save' => TRUE,
      ),
      'field_name' => array(
        'type' => 'text',
        'label' => t('The name of the handled price field.'),
        'skip save' => TRUE,
      ),
    ),
    'access callback' => 'commerce_order_rules_access',
  );
  return $events;
}

/**
 * Implements hook_rules_action_info().
 */
function commerce_multicurrency_rules_action_info() {
  $actions = array();

  // Register method to set currency code as rules action.
  $actions['commerce_multicurrency_set_user_currency_code'] = array(
    'label' => t('Set the user currency to use'),
    'parameter' => array(
      'currency_code' => array(
        'type' => 'text',
        'label' => t('The currency code to use'),
        'options list' => 'commerce_currency_code_options_list',
        'save' => FALSE,
      ),
      'overwrite_cookie' => array(
        'type' => 'boolean',
        'label' => t("Set currency even if it's defined in a cookie."),
        'optional' => TRUE,
        'default value' => TRUE,
        'save' => FALSE,
      ),
    ),
    'group' => t('Commerce Multicurrency'),
  );
  return $actions;
}
