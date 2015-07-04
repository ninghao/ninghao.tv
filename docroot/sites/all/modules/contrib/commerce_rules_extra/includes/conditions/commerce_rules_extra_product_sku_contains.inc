<?php

/**
 * @file
 * Rules condition Compare Product Amount.
 */

/**
 * Helper function to return the condition info to the main module.
 */
function commerce_rules_extra_product_sku_contains_condition_info() {
  $conditions = array(
    'group' => t('Commerce Order'),
    'label' => t('Order has one or more products where the sku contains.'),
    'parameter' => array(
      'commerce_order' => array(
        'type' => 'commerce_order',
        'label' => t('Order'),
        'description' => t('The order whose line items should be checked for the specified product. If the specified order does not exist, the comparison will act as if it is against a quantity of 0.'),
      ),
      'product_id' => array(
        'type' => 'text',
        'label' => t('Product SKU'),
        'description' => t('The SKU of the product must contain this information.'),
      ),
      'operator' => array(
        'type' => 'text',
        'label' => t('Operator'),
        'description' => t('The operator used with the quantity value below to compare the quantity of the specified product on the order.'),
        'default value' => '>=',
        'options list' => 'commerce_numeric_comparison_operator_options_list',
        'restriction' => 'input',
      ),
      'value' => array(
        'type' => 'text',
        'label' => t('Quantity'),
        'default value' => '1',
        'description' => t('The value to compare against the quantity of the specified product on the order.'),
      ),
    ),
    'callbacks' => array('execute' => 'commerce_rules_extra_partial_sku'),
  );
  return $conditions;
}

/**
 * Condition callback: checks to see if a particular product exists on an order
 * in the specified quantity, based on a sku fragment.
 */
function commerce_rules_extra_partial_sku($order, $sku, $operator, $value) {
  // Initialize variable to zero.
  $total_quantity = 0;

  // Don't do anything without a valid order.
  if (!empty($order)) {
    $order_wrapper = entity_metadata_wrapper('commerce_order', $order);

    // Populate the array of the quantities of the products on the order.
    foreach ($order_wrapper->commerce_line_items as $delta => $line_item_wrapper) {
      if (in_array($line_item_wrapper->type->value(), commerce_product_line_item_types())) {
        // Extract SKU and quantity from line item.
        $line_item_sku = $line_item_wrapper->commerce_product->sku->value();
        $quantity = $line_item_wrapper->quantity->value();

        // Does the SKU exist partially in the order SKU?
        if (strpos($line_item_sku, $sku) !== FALSE) {
          // Update quantity.
          $total_quantity += $quantity;
        }
      }
    }
  }

  // Comparison based on operator selected.
  switch ($operator) {
    case '<':
      return $total_quantity < $value;

    case '<=':
      return $total_quantity <= $value;

    case '=':
      return $total_quantity == $value;

    case '>=':
      return $total_quantity >= $value;

    case '>':
      return $total_quantity > $value;
  }
  // If everything failed then return FALSE.
  return FALSE;
}
