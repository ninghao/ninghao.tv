<?php

/**
 * @file
 * Rules integration for the Commerce Rules Extra Module.
 */

/**
 * Implements hook_rules_event_info().
 */
function commerce_rules_extra_rules_event_info() {
  global $_commerce_rules_extra_cre_includes;
  foreach ($_commerce_rules_extra_cre_includes['events'] as $file) {
    $rule = basename($file, '.inc');
    $f = $rule . '_event_info';
    $events[$rule] = $f();
  }

  return $events;
}

/**
 * Implements hook_rules_conditions_info().
 */
function commerce_rules_extra_rules_condition_info() {
  global $_commerce_rules_extra_cre_includes;
  foreach ($_commerce_rules_extra_cre_includes['conditions'] as $file) {
    $rule = basename($file, '.inc');
    $f = $rule . '_condition_info';
    $conditions[$rule] = $f();
  }

  return $conditions;
}

/**
 * Implements hook_rules_action_info().
 */
function commerce_rules_extra_rules_action_info() {
  global $_commerce_rules_extra_cre_includes;
  $actions = array();
  foreach ($_commerce_rules_extra_cre_includes['actions'] as $file) {
    $rule = basename($file, '.inc');
    $f = $rule . '_action_info';
    $actions[$rule] = $f();
  }

  return $actions;
}

/**
 * Option list callback for commerce_rules_extra_change_pane.
 *
 * Return list of checkout pages.
 */
function commerce_rules_extra_get_pages() {
  $pages = commerce_checkout_pages();
  $return = array('<same>' => '<do not change>');
  foreach ($pages as $page_id => $page) {
    $return[$page_id] = $page['name'];
  }
  return $return;
}
