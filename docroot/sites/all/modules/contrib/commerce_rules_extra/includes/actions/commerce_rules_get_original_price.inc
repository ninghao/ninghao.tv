<?php
function commerce_rules_get_original_price_action_info() {
    return array (
      'label' => t('Get the original price of the product from the line item'),
      'group' => t('Commerce Line Item'),
      'parameter' => array(
        'line_item' => array(
          'type' => 'commerce_line_item',
          'label' => t('Commerce Line Item'),
        ),
      ),
      'provides' => array(
        'orig_price' => array(
          'type' => 'integer',
          'label' => t('Original Price'),
        ),
      ),
    );
}

/**
 * Callback function for rule commerce_rules_get_original_price
 *
 * Return original price from line item.
 */
function commerce_rules_get_original_price($line_item) {
  $price = NULL;

  $old_price = $line_item->commerce_unit_price[LANGUAGE_NONE][0]['data']['components'][0]['price']['amount'];
  if (isset($old_price)) {
    $price = $old_price;
  }
  return array('orig_price' => $price);
}