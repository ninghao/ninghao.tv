<?php
/**
 * @file
 * Administrative UI for commerce currency.
 */

/**
 * Form to configure conversion settings.
 */
function commerce_multicurrency_conversion_settings_form($form, &$form_state) {
  $enabled_currencies = commerce_currencies(TRUE);
  $default_currency_code = commerce_default_currency();
  $conversion_settings = variable_get('commerce_multicurrency_conversion_settings', array());
  $use_cross_sync = variable_get('commerce_multicurrency_use_cross_conversion', TRUE);

  $sync_providers = array();
  foreach (commerce_multicurrency_commerce_multicurrency_exchange_rate_sync_provider() as $provider_code => $provider) {
    $sync_providers[$provider_code] = $provider['title'];
  }
  $form['sync_provider'] = array(
    '#type' => 'select',
    '#options' => $sync_providers,
    '#default_value' => variable_get('commerce_multicurrency_sync_provider', NULL),
    '#title' => t('Service to fetch exchange rates from:'),
  );
  $form['use_cross_sync'] = array(
    '#type' => 'checkbox',
    '#default_value' => $use_cross_sync,
    '#title' => t('Use cross conversion between non default currencies.'),
    '#description' => t('If enabled only the rates between the default currency and the other currencies have to be managed. The rates between the other currencies is derived from their rates relative to the default currency.'),
  );
  $form['sync_now'] = array(
    '#type' => 'checkbox',
    '#title' => t('Synchronize rates on save.'),
  );
  $form['demo_amount'] = array(
    '#type' => 'textfield',
    '#title' => t('Amount for example conversions:'),
    '#size' => 5,
    '#default_value' => 100,
  );

  $form['conversion_settings'] = array(
    '#attached' => array(
      'js' => array(
        drupal_get_path('module', 'commerce_multicurrency') . '/commerce_multicurrency.admin.js',
      ),
    ),
    '#tree' => TRUE,
  );

  if (count($enabled_currencies) > 1) {
    foreach ($enabled_currencies as $currency) {
      if ($use_cross_sync && $currency['code'] != $default_currency_code) {
        continue;
      }
      if (!isset($conversion_settings[$currency['code']])) {
        $conversion_settings[$currency['code']] = array();
      }
      $conversion_settings[$currency['code']] += array(
        'sync' => '1',
        'rates' => array(),
      );

      $form['conversion_settings'][$currency['code']] = array(
        '#type' => 'fieldset',
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        '#title' => $currency['name'],
      );
      if ($currency['code'] == $default_currency_code) {
        $form['conversion_settings'][$currency['code']]['#weight'] = -1;
      }
      $form['conversion_settings'][$currency['code']]['sync'] = array(
        '#type' => 'checkbox',
        '#title' => t('Synchronize all rates of this currency.'),
        '#states' => array(
          'checked' => array(
            '#edit-' . $currency['code'] . ' fieldset input[type="checkbox"]' => array('checked' => TRUE),
          ),
        ),
        '#default_value' => $conversion_settings[$currency['code']]['sync'],
      );

      foreach ($enabled_currencies as $conversion_currency) {
        if ($conversion_currency == $currency) {
          continue;
        }
        if (!isset($conversion_settings[$currency['code']]['rates'][$conversion_currency['code']])) {
          $conversion_settings[$currency['code']]['rates'][$conversion_currency['code']] = array();
        }
        $conversion_settings[$currency['code']]['rates'][$conversion_currency['code']] += array(
          'sync' => '1',
          'rate' => 0,
        );

        $form['conversion_settings'][$currency['code']]['sync']['#states']['checked']['input[name="conversion_settings[' . $currency['code'] . '][rates][' . $conversion_currency['code'] . '][sync]"]'] = array('checked' => TRUE);

        $form['conversion_settings'][$currency['code']]['rates'][$conversion_currency['code']] = array(
          '#type' => 'fieldset',
          '#attributes' => array('class' => array('conversion-rates')),
          '#collapsible' => TRUE,
          '#collapsed' => FALSE,
          '#title' => $conversion_currency['code'],
        );
        $form['conversion_settings'][$currency['code']]['rates'][$conversion_currency['code']]['sync'] = array(
          '#type' => 'checkbox',
          '#title' => t('Synchronize this conversion rate.'),
          '#default_value' => $conversion_settings[$currency['code']]['rates'][$conversion_currency['code']]['sync'],
        );
        $form['conversion_settings'][$currency['code']]['rates'][$conversion_currency['code']]['rate'] = array(
          '#type' => 'textfield',
          '#title' => t('Exchange rate'),
          '#attributes' => array('class' => array('conversion-rate')),
          '#description' => t(
            'Exchange rate from @currency_code to @conversion_currency_code.',
            array('@currency_code' => $currency['name'], '@conversion_currency_code' => $conversion_currency['name'])
          ),
          '#states' => array(
            'disabled' => array(
              'input[name="conversion_settings[' . $currency['code'] . '][rates][' . $conversion_currency['code'] . '][sync]"]' => array('checked' => TRUE),
            ),
          ),
          '#size' => 13,
          '#element_validate' => array('commerce_multicurrency_conversion_settings_form_rate_validate'),
          '#default_value' => $conversion_settings[$currency['code']]['rates'][$conversion_currency['code']]['rate'],
          '#field_suffix' => t(
            '* !demo_amount @currency_symbol = !amount @conversion_currency_symbol',
            array(
              '!demo_amount' => '<span class="demo-amount">100</span>',
              '@currency_symbol' => $currency['symbol'],
              '@conversion_currency_symbol' => $conversion_currency['symbol'],
              '!amount' => '<span class="demo-amount-converted">' . (100 * $conversion_settings[$currency['code']]['rates'][$conversion_currency['code']]['rate']) . '</span>',
            )
          ),
        );
      }
    }
  }
  else {
    drupal_set_message(t('Please enable the needed currencies to configure first.'), 'warning', FALSE);
  }

  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save Settings'),
  );

  return $form;
}

/**
 * Validate a rate.
 *
 * Heavy borrowed from number module:
 *
 * @see number_field_widget_validate()
 */
function commerce_multicurrency_conversion_settings_form_rate_validate($element, &$form_state) {
  $value = trim($element['#value']);
  // Reject invalid characters.
  if (!empty($value)) {
    $regexp = '@[^0-9\\.]@';
    $message = t('Only numbers and the decimal separator (@separator) allowed.', array('@separator' => '.'));
    if ($value != preg_replace($regexp, '', $value)) {
      form_error($element, $message);
    }
  }
}

/**
 * Submit handler for the conversion settings form.
 */
function commerce_multicurrency_conversion_settings_form_submit($form, &$form_state) {
  variable_set('commerce_multicurrency_conversion_settings', $form_state['values']['conversion_settings']);
  variable_set('commerce_multicurrency_sync_provider', $form_state['values']['sync_provider']);
  variable_set('commerce_multicurrency_use_cross_conversion', $form_state['values']['use_cross_sync']);
  if (!empty($form_state['values']['sync_now'])) {
    commerce_multicurrency_sync_exchange_rates_now();
  }
  else {
    // Ensure the settings are in sync.
    commerce_currencies(FALSE, TRUE);
  }
}


/**
 * Form to configure handling settings.
 */
function commerce_multicurrency_handling_settings_form($form, &$form_state) {
  $enabled_currencies = commerce_currencies(TRUE);

  $form['currency_fields'] = array(
    '#tree' => TRUE,
  );

  // Just provide this settings if there actually are product types.
  if (function_exists('commerce_product_types')) {
    foreach (commerce_product_types() as $commerce_product_type => $type_info) {
      $form['currency_fields'][$commerce_product_type] = array(
        '#title' => t('Enable dedicated price fields for %type:', array('%type' => $type_info['name'])),
        '#type' => 'fieldset',
        '#collapsible' => TRUE,
        '#collapsed' => FALSE,
        '#description' => t('For each enabled currency a dedicated price field in the commerce product type %type will be created.', array('%type' => $type_info['name'])),
      );

      foreach ($enabled_currencies as $currency_code => $currency_info) {
        $field_name = 'commerce_price_' . strtolower($currency_code);
        $field_instance = field_info_instance('commerce_product', $field_name, $commerce_product_type);
        $form['currency_fields'][$commerce_product_type][$currency_code] = array(
          '#title' => t($currency_info['name']),
          '#type' => 'checkbox',
          '#default_value' => (int) !empty($field_instance),
        );
      }
    }
  }

  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save Settings'),
  );

  drupal_set_message(t('Attention: Changing this configuration can lead to data loss.'), 'warning', FALSE);

  return $form;
}

/**
 * Submit handler for the handling settings form.
 */
function commerce_multicurrency_handling_settings_form_submit($form, &$form_state) {
  foreach ($form_state['values']['currency_fields'] as $commerce_product_type => $currency_fields) {
    foreach ($currency_fields as $currency_code => $has_field) {
      $field_name = 'commerce_price_' . strtolower($currency_code);
      $field_instance = field_info_instance('commerce_product', $field_name, $commerce_product_type);
      switch (TRUE) {
        // Field enabled but none created yet.
        case $has_field && !$field_instance:
          // Reuse commerce code to create the initial field instance.
          commerce_price_create_instance($field_name, 'commerce_product', $commerce_product_type, 'Price in ' . $currency_code);
          // Fetch the new instance and modify it.
          $new_field_instance = field_info_instance('commerce_product', $field_name, $commerce_product_type);
          $new_field_instance['widget']['settings']['currency_code'] = $currency_code;
          $new_field_instance['widget']['type'] = 'commerce_price_simple';
          field_update_instance($new_field_instance);
          drupal_set_message(t('Field %fieldname on %type created.', array('%fieldname' => $field_name, '%type' => $commerce_product_type)), 'status', FALSE);
          break;

        // Field enabled and already created.
        case $has_field && !$field_instance:
          // Done.
          break;

        // Field disabled but still there.
        case !$has_field && $field_instance:
          field_delete_instance($field_instance);
          drupal_set_message(t('Field %fieldname on %type deleted.', array('%fieldname' => $field_name, '%type' => $commerce_product_type)), 'status', FALSE);
          break;

        // Field disabled and already removed.
        case !$has_field && $field_instance:
          // Done.
          break;
      }
    }
  }
}
