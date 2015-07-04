<?php

/**
 * @file
 * Default rule configurations for License.
 */

/**
 * Implements hook_default_rules_configuration().
 */
function commerce_license_default_rules_configuration() {
  $rules = array();

  // Add a reaction rule to activate the free licenses of an order.
  $rule = rules_reaction_rule();

  $rule->label = t('Activates licenses of a free order');
  $rule->tags = array('Commerce License');
  $rule->active = TRUE;
  // Run after the rules that modify the order owner.
  $rule->weight = 9;

  $rule
    ->event('commerce_checkout_complete')
    ->condition('commerce_payment_order_balance_comparison', array('commerce_order:select' => 'commerce_order', 'operator' => '=', 'value' => 0))
    ->action('commerce_license_activate_order_licenses', array(
      'commerce_order:select' => 'commerce-order',
    ));

  $rules['commerce_license_activate_free_licenses'] = $rule;

  // Add a reaction rule to activate the licenses of an order that has been paid
  // in full.
  $rule = rules_reaction_rule();

  $rule->label = t('Activates licenses of an order');
  $rule->tags = array('Commerce License');
  $rule->active = TRUE;
  // Run after the rules that modify the order owner.
  $rule->weight = 9;

  $rule
    ->event('commerce_payment_order_paid_in_full')
    ->action('commerce_license_activate_order_licenses', array(
      'commerce_order:select' => 'commerce-order',
    ));

  $rules['commerce_license_activate_licenses'] = $rule;

  return $rules;
}
