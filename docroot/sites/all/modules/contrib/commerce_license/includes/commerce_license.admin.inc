<?php

/**
 * @file
 * Contains admin menu callbacks for the Commerce License module.
 */

/**
 * Settings form callback.
 */
function commerce_license_settings_form($form, &$form_state) {
  $form['help'] = array(
    '#markup' => t('This form allows you to enable licensing for your product and line item types.'),
  );
  $form['commerce_license_product_types'] = array(
    '#title' => t('Product types'),
    '#type' => 'checkboxes',
    '#default_value' => variable_get('commerce_license_product_types', array()),
    '#options' => commerce_product_type_options_list(),
  );
  $line_item_types = array();
  foreach (commerce_line_item_types() as $type => $line_item_type) {
    // Only allow product line item types to be selected.
    if (!empty($line_item_type['product'])) {
      $line_item_types[$type] = $line_item_type['name'];
    }
  }
  // The recurring line item type is special, don't show it here.
  if (module_exists('commerce_license_billing')) {
    unset($line_item_types['recurring']);
  }
  $form['commerce_license_line_item_types'] = array(
    '#title' => t('Line item types'),
    '#type' => 'checkboxes',
    '#default_value' => variable_get('commerce_license_line_item_types', array_keys($line_item_types)),
    '#options' => $line_item_types,
  );

  if (module_exists('advancedqueue')) {
    $form['refresh_settings'] = array(
      '#type' => 'fieldset',
      '#title' => t('Refresh settings'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    $form['refresh_settings']['help'] = array(
      '#markup' => t('If a license is synchronizable, the "License completion message" checkout pane will refresh itself periodically, allowing the customer to follow the synchronization process.'),
    );
    $form['refresh_settings']['commerce_license_refresh_rate'] = array(
      '#type' => 'textfield',
      '#title' => t('Refresh rate'),
      '#field_suffix' => t('seconds'),
      '#description' => t('How often should the checkout pane refresh (e.g. every 2 seconds).'),
      '#default_value' => variable_get('commerce_license_refresh_rate', 2),
      '#size' => 4,
      '#element_validate' => array('element_validate_number'),
    );
    $form['refresh_settings']['commerce_license_refresh_timeout'] = array(
      '#type' => 'textfield',
      '#title' => t('Refresh timeout'),
      '#field_suffix' => t('seconds'),
      '#description' => t('How long should the refresh be performed until it is stopped (e.g. 60 seconds).'),
      '#default_value' => variable_get('commerce_license_refresh_timeout', 60),
      '#size' => 4,
      '#element_validate' => array('element_validate_number'),
    );
  }

  $form = system_settings_form($form);
  $form['#submit'][] = 'commerce_license_settings_form_submit';
  return $form;
}

/**
 * Submit callback of the license settings form.
 */
function commerce_license_settings_form_submit($form, &$form_state) {
  // This will create the required fields on any newly selected types,
  // and remove them from any newly deselected types.
  drupal_flush_all_caches();
}
