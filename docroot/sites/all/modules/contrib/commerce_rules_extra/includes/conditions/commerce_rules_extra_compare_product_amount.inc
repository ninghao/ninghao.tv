<?php

/**
 * @file
 * Rules condition Compare Product Amount.
 */

/**
 * Helper function to return the condition info to the main module.
 */
function commerce_rules_extra_compare_product_amount_condition_info() {
  $conditions = commerce_rules_extra_compare_product_quantity_condition_info();
  $conditions['label'] = t('Total amount of selected products comparison');
  $conditions['parameter']['value'] = array(
    'type' => 'integer',
    'label' => t('Total amount'),
    'default value' => 100,
    'description' => t('The value to compare against the total amount of products of the type on the order. Amount without decimal point (ie : 50.65 US ==> 5065'),
  );
  return $conditions;
}

/**
 * Implements hook_form_alter().
 */
function commerce_rules_extra_compare_product_amount_form_alter(&$form, &$form_state, $options, RulesAbstractPlugin $element) {
  return commerce_rules_extra_rules_condition_has_terms_form_alter($form, $form_state, $options, $element);
}

/**
 * Callback function for Compare Product Amount condition.
 */
function commerce_rules_extra_compare_product_amount($order, $product_type, $exclude, $operator, $value, $voc_name, $term_ids, $product_display, $term_operator) {
  return commerce_rules_extra_compare_product($order, $voc_name, $term_ids, $product_display, $term_operator, $product_type, $exclude, $operator, $value, 'amount');
}
