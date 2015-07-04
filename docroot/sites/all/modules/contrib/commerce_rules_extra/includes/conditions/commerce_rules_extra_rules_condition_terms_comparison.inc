<?php

/**
 * @file
 * Rules condition Terms Comparison.
 */

/**
 * Helper function to return the condition info to the main module.
 */
function commerce_rules_extra_rules_condition_terms_comparison_condition_info() {
  return array(
    'group' => t('Commerce Line Item'),
    'label' => t('Line item product and another entity have matching terms'),
    'parameter' => array(
      'line_item' => array(
        'type' => 'commerce_line_item',
        'label' => t('Commerce Line Item'),
      ),
      'voc_name' => array(
        'type' => 'text',
        'label' => t('Term Reference Field'),
        'description' => t('The machine-name of the expected product\'s term reference field'),
        'options list' => 'commerce_rules_extra_term_fields_options_list',
        'restriction' => 'input',
      ),
      'entity' => array(
        'type' => 'integer',
        'label' => t('Entity to be compared'),
      ),
    ),
  );
}

/**
 * Callback function for the Terms Comparison condition.
 *
 * Check if a line item's product and another entity both reference the same term id.
 *
 * @param object $line_item
 *    A commerce_line_item containing the product being checked.
 * @param string $voc_name
 *    A string containing the machine name of a Taxonomy reference field.
 * @param int $entity
 *    The entity to be compared (e.g. site:current-user:your-field:tid).
 *
 * @return bool
 *    TRUE if the product and entity have matching terms in the specified field otherwise FALSE.
 */
function commerce_rules_extra_rules_condition_terms_comparison($line_item, $voc_name, $entity) {
  if (!empty($line_item)) {
    $wrapper = entity_metadata_wrapper('commerce_line_item', $line_item);
    $product = $wrapper->commerce_product->value();
    $product_terms = array();
    foreach (commerce_rules_extra_get_fields_for_vocabulary($voc_name) as $field) {
      if (isset($product->$field)) {
        $product_terms = $wrapper->commerce_product->$field->value();
        break;
      }
    }
    if (isset($product_terms->tid)) {
      if ($product_terms->tid == $entity) {
        return TRUE;
      }
    }
  }
  return FALSE;
}
