<?php

/**
 * @file
 * Rules action Add Line Item to Cart.
 */

/**
 * Helper function to return the action info to the main module.
 */
function commerce_rules_extra_add_line_item_to_cart_action_info() {
  return array(
    'label' => t('Add line item to cart'),
    'parameter' => array(
      'order' => array(
        'type' => 'commerce_order',
        'label' => t('Commerce Order'),
      ),
      'line_item' => array(
        'type' => 'commerce_line_item',
        'label' => t('Commerce Line Item'),
      ),
    ),
    'group' => t('Commerce Line Item'),
    'base' => 'commerce_rules_extra_add_line_item_to_cart_action',
    'callbacks' => array('execute' => 'commerce_rules_extra_add_line_item_to_cart_action'),
  );
}

/**
 * Callback function for CRE Add Line Item to Cart Action.
 */
function commerce_rules_extra_add_line_item_to_cart_action($order, $line_item) {
  if (!($order instanceof EntityMetadataWrapper)) {
    $order = entity_metadata_wrapper('commerce_order', $order);
  }
  if (!($line_item instanceof EntityMetadataWrapper)) {
    $line_item = entity_metadata_wrapper('commerce_line_item', $line_item);
  }
  $order->commerce_line_items[] = $line_item;
  commerce_order_save($order);
}
