<?php

/**
 * @file
 * API implementation for Commerce Rules Extra.
 */

/**
 *  Implements hook_checkout_pane_urls_alter().
 *  Change urls where "Process_checkout_pane" rule event is triggered
 *  $urls : Array of urls
 *  
 */
function commerce_rules_extra_rules_hook_checkout_pane_urls_alter(&$urls) {
  // Also trigger event on /cart url
  $urls[] = 'cart';
}
