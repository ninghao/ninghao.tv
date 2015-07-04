<?php

/**
 * @file
 * Rules events Checkout Page Event.
 */

/**
 * Helper function to return the event info to the main module.
 */
function commerce_rules_extra_process_checkout_page_event_info() {
  return array(
    'label' => t('Process a change to the checkout page'),
    'group' => t('Commerce Checkout'),
  );
}

/**
 * Implements hook_commerce_checkout_page_info_alter().
 *
 * Invoke all rules for process_checkout_page event and alter pages properties returned by rules
 */
function commerce_rules_extra_commerce_checkout_page_info_alter(&$checkout_pages) {

  global $_commerce_rules_extra_page_changes;
  if (!$_commerce_rules_extra_page_changes) {
    $_commerce_rules_extra_page_changes = array();
  }
  $urls = array('checkout', 'system');
  drupal_alter('checkout_page_urls', $urls);
  if (in_array(arg(0), $urls)) {

    // As of this issue https://www.drupal.org/node/1855900 it can easily happen
    // that this hook is invoked checkout_pages definitions that don't have any
    // defaults set. To avoid notices we set the defaults on our own. The code
    // below has to exactly match what's done in commerce_checkout_pages().
    $count = 0;
    foreach ($checkout_pages as $page_id => $checkout_page) {
      $defaults = array(
        'page_id' => $page_id,
        'name' => $checkout_page['title'],
        'title' => '',
        'help' => '',
        'status_cart' => TRUE,
        'buttons' => TRUE,
        'back_value' => t('Go back'),
        'submit_value' => t('Continue to next step'),
        'prev_page' => NULL,
        'next_page' => NULL,
      );

      $checkout_pages[$page_id] += $defaults;

      // Set a weight that preserves the order of 0 weighted pages.
      if (empty($checkout_page['weight'])) {
        $checkout_pages[$page_id]['weight'] = $count++ / count($checkout_pages);
      }
    }

    rules_invoke_all('commerce_rules_extra_process_checkout_page');
    foreach ($_commerce_rules_extra_page_changes as $page_id => $changes) {
      if ($changes['title'] != '') {
        $checkout_pages[$page_id]['title'] = $changes['title'];
      }
      if ($changes['help'] != '') {
        $checkout_pages[$page_id]['help'] = $changes['help'];
      }
      if ($changes['submit_value'] != '') {
        $checkout_pages[$page_id]['submit_value'] = $changes['submit_value'];
      }
    }
  }
}
