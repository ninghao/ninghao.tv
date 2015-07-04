<?php

/**
 * @file
 * Rules events Quantity Changed.
 */

/**
 * Helper function to return the event info to the main module.
 */
function commerce_rules_extra_line_item_quantity_changed_event_info() {
  return array(
    'label' => t('Quantity of line item has changed'),
    'group' => t('Commerce Checkout'),
    'variables' => array(
      'commerce_order' => array(
        'type' => 'commerce_order',
        'label' => t('Shopping cart order'),
      ),
      'commerce_product' => array(
        'label' => t('Product'),
        'type' => 'commerce_product',
      ),
      'commerce_line_item' => array(
        'label' => t('Line item'),
        'type' => 'commerce_line_item',
      ),
      'commerce_line_item_old_quantity' => array(
        'label' => t('Commerce item quantity before changes'),
        'type' => 'decimal',
      ),
    ),
  );
}

/**
 * Implements hook_FORM_submit().
 */
function commerce_rules_extra_cart_submit($form, &$form_state) {
  $order = $form_state['order'];
  $line_items = $form_state['line_items'];
  $i = 0;
  foreach ($line_items as $line_item) {
    $line_item_wrapper = entity_metadata_wrapper('commerce_line_item', $line_item);
    $product = $line_item_wrapper->commerce_product->value();
    if (floatval($form['edit_quantity'][$i]['#value']) != floatval($line_item->quantity)) {
      $old_quantity = floatval($line_item->quantity);
      $line_item->quantity = $form['edit_quantity'][$i]['#value'];
      rules_invoke_event('commerce_rules_extra_line_item_quantity_changed', $order, $product, $line_item, $old_quantity);
    }
    $i++;
  }
}

/**
 * Implements hook_commerce_cart_product_add().
 */
function commerce_rules_extra_commerce_cart_product_add($order, $product, $quantity, $line_item) {
  rules_invoke_event('commerce_rules_extra_line_item_quantity_changed', $order, $product, $line_item, floatval($line_item->quantity) - floatval($quantity));
}

/**
 * Implements hook_commerce_product_remove().
 */
function commerce_rules_extra_commerce_cart_product_remove($order, $product, $quantity, $line_item) {
  $old_quantity = floatval($line_item->quantity);
  $line_item->quantity = 0;
  rules_invoke_event('commerce_rules_extra_line_item_quantity_changed', $order, $product, $line_item, $old_quantity);
}

/**
 * Implements hook_form_alter().
 */
function commerce_rules_extra_form_alter(&$form, &$form_state, $form_id) {
  if (strpos($form_id, 'views_form_commerce_cart_form_') === 0 && array_key_exists('submit', $form['actions'])) {
    array_unshift($form['actions']['submit']['#submit'], 'commerce_rules_extra_cart_submit');
  }
}
