<?php

/**
 * @file
 * Rules condition Compare Product Quantity.
 */

/**
 * Helper function to return the condition info to the main module.
 */
function commerce_rules_extra_compare_product_quantity_condition_info() {
  $info = array(
    'group' => t('Commerce Order'),
    'label' => t('Total quantity of selected product comparison'),
    'parameter' => array(
      'order' => array(
        'type' => 'commerce_order',
        'label' => t('Order'),
        'description' => t('The order whose product line item quantities should be totalled. If the specified order does not exist, the comparison will act as if it is against a quantity of 0.'),
      ),
      'product_type' => array(
        'type' => 'list<integer>',
        'label' => t('Product Type(s)'),
        'description' => t('The type(s) of the product to look for on the order.'),
        'options list' => 'commerce_rules_extra_product_type_options_list',
        'restriction' => 'input',
        'optional' => TRUE,
      ),
      'exclude' => array(
        'type' => 'boolean',
        'label' => t('Exclude Selected Content'),
        'description' => t('If checked the total of all products on the order excluding the product type specified above will be calculated. Negates previous selection.'),
        'restriction' => 'input',
      ),
      'operator' => array(
        'type' => 'text',
        'label' => t('Operator'),
        'description' => t('The comparison operator to use against the total number of products matching the term on the order.'),
        'default value' => '>=',
        'options list' => 'commerce_numeric_comparison_operator_options_list',
        'restriction' => 'input',
      ),
      'value' => array(
        'type' => 'integer',
        'label' => t('Quantity'),
        'default value' => 1,
        'description' => t('The value to compare against the total quantity of products matching the term on the order.'),
      ),
    ),
  );
  $condition = array_merge_recursive($info, commerce_rules_extra_terms_parameters(FALSE));
  return $condition;
}

/**
 * Implements hook_form_alter().
 *
 * Alters the form for commerce_rules_extra_compare_termed_product_quantity so that we
 * can require the term reference field be selected first so that the term_id list can be populated.
 */
function commerce_rules_extra_compare_product_quantity_form_alter(&$form, &$form_state, $options, RulesAbstractPlugin $element) {
  return commerce_rules_extra_rules_condition_has_terms_form_alter($form, $form_state, $options, $element);
}

/**
 * Callback function for Compare Product Quantity.
 *
 * Calculates the quantity of products in an order that have the term $term_id
 * and compares it to a given value.
 */
function commerce_rules_extra_compare_product_quantity($order, $product_type, $exclude, $operator, $value, $voc_name, $term_id, $product_display, $term_operator) {
  return commerce_rules_extra_compare_product($order, $voc_name, $term_id, $product_display, $term_operator, $product_type, $exclude, $operator, $value, 'quantity');
}

/**
 * Callback function for Commerce Rules Extra Compare Product.
 */
function commerce_rules_extra_compare_product($order, $voc_name, $term_ids, $product_display, $term_operator, $product_type, $exclude, $operator, $value, $type = 'quantity') {
  $quantity = 0;
  $voc_name = trim($voc_name);
  if (!empty($order)) {
    $wrapper = entity_metadata_wrapper('commerce_order', $order);
    $line_items = $wrapper->commerce_line_items->value();
    if (!empty($line_items)) {
      // Holds line items that have products with $term_id
      $termed_line_items = array();
      foreach ($line_items as $line_item) {
        if (in_array($line_item->type, commerce_product_line_item_types())) {
          $pwrapper = entity_metadata_wrapper('commerce_line_item', $line_item);
          $product = $pwrapper->commerce_product->value();
          if ((empty($product_type) || array_key_exists('', $product_type)) || (!$exclude && array_key_exists($product->type, $product_type)) || ($exclude && !array_key_exists($product->type, $product_type))) {
            if (empty($term_ids) || commerce_rules_extra_rules_condition_has_terms($line_item, $voc_name, $term_ids, $product_display, $term_operator)) {
              $termed_line_items[] = $line_item;
            }
          }
        }
      }
      if (!empty($termed_line_items)) {
        if ($type == 'quantity') {
          $quantity = commerce_line_items_quantity($termed_line_items, commerce_product_line_item_types());
        }
        else {
          $result = commerce_line_items_total($termed_line_items, commerce_product_line_item_types());
          $quantity = $result['amount'];
        }
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

/**
 * Callback function to create empty products option list.
 */
function commerce_rules_extra_product_type_options_list() {
  return array('' => '<' . t('none') . '>') + commerce_product_type_options_list();
}
