
Commerce Rules Extra (CRE) module for Drupal 7.x.
This module adds many useful rules for Drupal Commerce.

Questions, problems, or issues?  Please file a ticket here:
https://www.drupal.org/project/commerce_rules_extra

REQUIREMENTS
------------
* Drupal Commerce
* Rules version 2.7
* Taxonomy

RULES MODULE
------------
Commerce Rules Extra is only tested on Rules Version 2.7. Rules version 2.8 is not
compatible with Drupal Commerce; therefore, Commerce Guys state that Drupal
Commerce can only run with Rules version 2.7. This issue is documentented in
issues on Drupal.org.
https://www.drupal.org/node/2407897
https://www.drupal.org/node/2410341
https://www.drupal.org/node/2403851

INSTALLATION INSTRUCTIONS
-------------------------
1.  Copy the files included in the tarball into a directory named "commerce_rules_extra" in
    your Drupal sites/all/modules/ directory.
2.  Login as site administrator.
3.  Enable the Commerce Rules Extra module on the Administer -> Modules page 
    (Under the "Commerce (contrib)" category).
4.  Build fantastic rules to generate better sales (see below for more details).

== COMPONENT DETAILS ==

RULES EVENTS
-------------
1. Process change to a checkout pane : Fires when a pane is processed during checkout.
2. Process change to a checkout page : Fires when a page is processed during checkout.
3. Line item quantity has changed (occurs when adding ou removing a product to 
   cart and when a quantity has been modified in cart form).

RULES CONDITIONS
----------------
1. Line item product has term(s) : Test if a line item has a product with one or 
   all specified terms.
2. Total product with term quantity comparison : Total of products with term 
   and with a specified quantity.
3. Total product of type quantity comparison : Total of products of specified 
   type and with a specified quantity.
4. Total product of type amount comparison : Total of products of specified type 
   and with a specified amount.
5. Total quantity of product line items in the cart: This is the total count of
   line items in the cart that are of the type "product".  This does not count
   the quantity of the line items just how many individual product line items
   that are in the cart.  Example: you want to give a discount to someone that
   purchases 5 separate items from your cart but not 5 of one item.

RULES ACTIONS
-------------
1. Change pane properties : Change visibility, page, weight of a pane.
2. Change page properties : Alter page attributes (Title, Help, and Submit 
   Button Text).
3. Get the referencing node from the line item : Set the referenced node 
   (product display node) into a variable to be reused in another action.
4. Add line item to cart: It would appear that this is unnecessary, but if you 
   use the rules built in to add a line item to the order it is not updated 
   properly, see issue #2108669: "Cannot attach line items to order via rules"
   on Drupal.org.
