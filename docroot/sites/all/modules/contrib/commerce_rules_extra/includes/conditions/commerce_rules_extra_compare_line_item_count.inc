<?php

/**
 * @file
 * Rules condition Compare Total Product Quantity.
 */

/**
 * Helper function to return the condition info to the main module.
 */
function commerce_rules_extra_compare_line_item_count_condition_info() {
  $info = array(
    'group' => t('Commerce Order'),
    'label' => t('Total quantity of product line items in the cart'),
    'parameter' => array(
      'order' => array(
        'type' => 'commerce_order',
        'label' => t('Order'),
        'description' => t('The order whose product line items you want counted. If the specified order does not exist, the comparison will act as if it is against a quantity of 0.'),
      ),
      'operator' => array(
        'type' => 'text',
        'label' => t('Operator'),
        'description' => t('The comparison operator to use against the total line item count.'),
        'default value' => '>=',
        'options list' => 'commerce_numeric_comparison_operator_options_list',
        'restriction' => 'input',
      ),
      'value' => array(
        'type' => 'integer',
        'label' => t('Quantity'),
        'default value' => 1,
        'description' => t('The value to compare against the total line item count in the order.'),
      ),
    ),
    'callbacks' => array('execute' => 'commerce_rules_extra_compare_line_item_count'),
  );
  return $info;
}

/**
 * Callback function for Commerce Rules Extra Compare Total Products.
 */
function commerce_rules_extra_compare_line_item_count($order, $operator, $value) {
  $quantity = 0;
  // Don't do anything without a valid order.

  if (!empty($order)) {
    $order_wrapper = entity_metadata_wrapper('commerce_order', $order);
    $line_items = $order_wrapper->commerce_line_items->value();
    // We only want to count line items that are of the type "product".

    if (!empty($line_items)) {
      // Holds line items that have products with $term_id
      $termed_line_items = array();
      foreach ($line_items as $line_item) {
        if ($line_item->type == 'product') {
          $quantity++;
        }
      }
    }
    // Make a quantity comparison based on the operator.
    switch ($operator) {
      case '<':
        return $quantity < $value;

        break;

      case '<=':
        return $quantity <= $value;

        break;

      case '=':
        return $quantity == $value;

        break;

      case '>=':
        return $quantity >= $value;

        break;

      case '>':
        return $quantity > $value;

        break;
    }
    return FALSE;
  }
}
