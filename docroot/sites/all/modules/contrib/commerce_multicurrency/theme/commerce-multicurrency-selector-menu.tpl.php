<?php
/**
 * @file
 * Currency selector menu.
 *
 * Available variables:
 * $enabled_currencies
 * $user_currency
 */
?>
<ul class="currency_select_menu">
<?php foreach($enabled_currencies as $currency) : ?>
  <li class="<?php print $currency['code'] . (($currency['code'] == $user_currency) ? ' active' : NULL); ?>">
    <a href="<?php print url('commerce_currency_select/' . $currency['code'], array('query' => drupal_get_destination())); ?>"><?php print $currency['code']; ?></a>
  </li>
<?php endforeach;?>
</ul>
