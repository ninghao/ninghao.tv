<?php

/**
 * Provides views integration for licenses.
 */

/**
 * Implements hook_views_plugins().
 */
function commerce_license_views_plugins() {
  return array(
    'access' => array(
      'commerce_license_sync' => array(
        'title' => t('Synchronization queue'),
        'handler' => 'commerce_license_plugin_access_sync',
        'uses options' => FALSE,
      ),
    ),
  );
}
