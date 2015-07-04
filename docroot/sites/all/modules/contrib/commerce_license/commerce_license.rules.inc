<?php

/**
 * @file
 * Commerce License rules integration.
 */

/**
 * Implements hook_rules_event_info().
 */
function commerce_license_rules_event_info() {
  $events = array();
  $events['commerce_license_synchronize'] = array(
    'label' => t('After successfuly synchronizing a license'),
    'group' => t('Commerce License'),
    'variables' => array(
      'commerce_license' => array(
        'type' => 'commerce_license',
        'label' => t('The license'),
      ),
    ),
  );
  $events['commerce_license_synchronize_failed'] = array(
    'label' => t('After failing to synchronize a license'),
    'group' => t('Commerce License'),
    'variables' => array(
      'commerce_license' => array(
        'type' => 'commerce_license',
        'label' => t('The license'),
      ),
    ),
  );

  return $events;
}

/**
 * Implements hook_rules_action_info().
 */
function commerce_license_rules_action_info() {
  $actions = array();

  $actions['commerce_license_activate_order_licenses'] = array(
    'label' => t('Activates all licenses of the provided order'),
    'parameter' => array(
      'commerce_order' => array(
        'type' => 'commerce_order',
        'label' => t('Order in checkout'),
      ),
    ),
    'group' => t('Commerce License'),
    'callbacks' => array(
      'execute' => 'commerce_license_activate_order_licenses',
    ),
  );

  return $actions;
}
