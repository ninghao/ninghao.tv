<?php

/**
 * @file
 * Contains commerce_license_plugin_access_sync.
 */

/**
 * Access plugin for the synchronization queue view.
 */
class commerce_license_plugin_access_sync extends views_plugin_access {
  function access($account) {
    return commerce_license_sync_access($account);
  }

  function get_access_callback() {
    return array('commerce_license_sync_access', array());
  }
}
