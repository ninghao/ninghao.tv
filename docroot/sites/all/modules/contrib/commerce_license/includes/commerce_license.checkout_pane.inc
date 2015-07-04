<?php
/**
 * @file
 * Checkout pane callbacks.
 */

/**
 * Checkout pane callback: License edit.
 *
 * Outputs a pane for each found license.
 */
function commerce_license_information_checkout_form($form, &$form_state, $checkout_pane, $order) {
  $pane_form = array(
    '#type' => 'container',
  );
  $licenses = commerce_license_get_order_licenses($order, TRUE);
  foreach ($licenses as $license) {
    $license_id = $license->license_id;
    $title_arguments = array(
      '@product_title' => $license->wrapper->product->title->value(),
    );
    $pane_form[$license_id] = array(
      '#type' => $checkout_pane['fieldset'] ? 'fieldset' : 'container',
      '#title' => t('License information - @product_title', $title_arguments),
      '#collapsible' => $checkout_pane['collapsible'],
      '#collapsed' => $checkout_pane['collapsed'],
      '#parents' => array($license_id),
      '#tree' => TRUE,
    );
    $license->form($pane_form[$license_id], $form_state);
  }

  return $pane_form;
}

/**
 * Checkout pane callback: License edit pane validate.
 */
function commerce_license_information_checkout_form_validate($form, &$form_state, $checkout_pane, $order) {
  $licenses = commerce_license_get_order_licenses($order, TRUE);
  foreach ($licenses as $license) {
    $license_form = $form[$checkout_pane['pane_id']][$license->license_id];
    $license->formValidate($license_form, $form_state);
  }

  $form_errors = &drupal_static('form_set_error', array());
  if (empty($form_state['errors']) && empty($form_errors)) {
    return TRUE;
  }
  return FALSE;
}

/**
 * Checkout pane callback: license edit pane submit callback.
 */
function commerce_license_information_checkout_form_submit($form, &$form_state, $checkout_pane, $order) {
  $licenses = commerce_license_get_order_licenses($order, TRUE);
  foreach ($licenses as $license) {
    $license_form = $form[$checkout_pane['pane_id']][$license->license_id];
    $license->formSubmit($license_form, $form_state);
    $license->save();
  }
}

/**
 * Checkout pane callback: License completion message.
 *
 * Outputs a pane for each found license.
 * Synchronizable licenses are shown in a pane that refreshes itself
 * periodically (every 2s by default), allowing the customer to follow the synchronization process.
 * If the license doesn't get synchronized within $refresh_timeout seconds
 * (60s by default), the refresh process is stopped, and the license
 * synchronization is marked as failed.
 */
function commerce_license_complete_checkout_form($form, &$form_state, $checkout_pane, $order) {
  $pane_form = array(
    '#type' => 'container',
  );
  $balance = commerce_payment_order_balance($order);
  $order_paid = ($balance && $balance['amount'] <= 0);
  $js_attached = FALSE;

  // Show a pane for each found license.
  $licenses = commerce_license_get_order_licenses($order);
  foreach ($licenses as $license) {
    $synchronizable = ($license instanceof CommerceLicenseSynchronizableInterface);
    // If this is a synchronizable license, attach the JS for the refresh.
    if (!$js_attached && $synchronizable) {
      $pane_form['#attached']['library'][] = array('system', 'drupal.ajax');
      $pane_form['#attached']['js'] = array(
        drupal_get_path('module', 'commerce_license') . '/commerce_license.js',
      );
      $js_attached = TRUE;
    }
    // Get the message to show.
    $message = t('Your license will be activated once your payment has been processed.');
    if ($order_paid) {
      $refresh = $synchronizable ? TRUE : FALSE;
      $message = commerce_license_checkout_competion_message($license, $refresh);
    }

    $title_arguments = array(
      '@product_title' => $license->wrapper->product->title->value(),
    );
    $pane_form[$license->license_id] = array(
      '#type' => $checkout_pane['fieldset'] ? 'fieldset' : 'container',
      '#title' => t('License information - @product_title', $title_arguments),
      '#collapsible' => $checkout_pane['collapsible'],
      '#collapsed' => $checkout_pane['collapsed'],
      'output' => array(
        '#markup' => $message,
      ),
    );
  }

  return $pane_form;
}

/**
 * Ajax callback for the license completion message checkout pane.
 *
 * Called once every $refresh_rate seconds (2s by default), until the
 * synchronization is marked as successful / failed, or until it timeouts
 * (60s by default).
 */
function commerce_license_complete_checkout_ajax_callback($license) {
  $session_key = 'commerce_license_refresh_start_' . $license->license_id;
  $current_time = commerce_license_get_time();
  if (!isset($_SESSION[$session_key])) {
    $_SESSION[$session_key] = $current_time;
  }
  // Check if the refresh has timed out.
  $refresh_time = $current_time - $_SESSION[$session_key];
  $refresh_timeout = variable_get('commerce_license_refresh_timeout', 60);
  if ($refresh_time > $refresh_timeout) {
    $license->wrapper->sync_status = COMMERCE_LICENSE_SYNC_FAILED;
    $license->save();
    // Fire a rules event and a hook, allowing developers to respond
    // to a failed synchronization.
    rules_invoke_all('commerce_license_synchronize_failed', $license);
  }
  // Stop the refresh if synchronization has ended (succeeded, failed).
  $refresh = TRUE;
  $sync_status = $license->wrapper->sync_status->value();
  if ($sync_status != COMMERCE_LICENSE_NEEDS_SYNC) {
    unset($_SESSION[$session_key]);
    $refresh = FALSE;
  }

  // Get the message.
  $message = commerce_license_checkout_competion_message($license, $refresh);
  return array(
    '#type' => 'ajax',
    '#commands' => array(
      ajax_command_replace('#commerce-license-checkout-pane-' . $license->license_id, $message),
    ),
  );
}

/**
 * Returns the license checkout completion message.
 *
 * @param $license
 *   The license entity.
 * @param $refresh
 *   Whether the checkout completion message should be refreshed via AJAX
 *   in set intervals.
 *
 * @return
 *   A string containing the success text.
 */
function commerce_license_checkout_competion_message($license, $refresh = FALSE) {
  $element = array(
    '#type' => 'container',
    '#attributes' => array(
      'id' => 'commerce-license-checkout-pane-' . $license->license_id,
      'class' => array(
        'commerce-license-checkout',
      ),
    ),
    'output' => array(
      '#markup' => $license->checkoutCompletionMessage(),
    ),
  );
  // Add data attributes for the refresh process.
  if ($refresh) {
    $element['#attributes'] += array(
      'data-refresh-url' => url('ajax/commerce_license/' . $license->license_id, array('absolute' => TRUE)),
      'data-refresh-rate' => variable_get('commerce_license_refresh_rate', 2),
    );
  }

  return drupal_render($element);
}
