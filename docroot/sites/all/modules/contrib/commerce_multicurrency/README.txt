
-- SUMMARY --

This module enhances some of the multi-currency capabilities of commerce
http://drupal.org/project/commerce

It provides these features:
 - UI for fine granular definition of exchange rates.
   (Every currency combination can be defined separately if needed)
 - Syncronization of currency exchange rates directly from the European Central Bank (ECB).
 - UI to specify which rates shall be syncornized and which are handled manually.
 - Generation of currency specific price fields inclusive generation of rule-set to handle them.
 - hooks for easy integration of custom currency exchange rate sources.


-- REQUIREMENTS --

Commerce: http://drupal.org/project/commerce


-- INSTALLATION --

* Install as usual, see http://drupal.org/documentation/install/modules-themes/modules-7 for further information.

* Configure the currency conversion: admin/commerce/config/currency/conversion and/or
  the currency handling: admin/commerce/config/currency/handling

* Run cron or sync manually to synchronize the rates.


-- API --
It is possible to add own functions / services to sync the conversion rates.
Check commerce_multicurrency.api.php and commerce_multicurrency.ecb.inc for further
information and examples.

The sync is triggered manually or by cron.