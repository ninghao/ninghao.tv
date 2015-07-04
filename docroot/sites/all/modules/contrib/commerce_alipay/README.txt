Commerce Alipay
===============

Implements [Alipay](http://www.alipay.com) payment services for use with
[Drupal Commerce](http://drupal.org/project/commerce).


Features
--------
Unfortunately, there isn't much of an English Alipay document describing the
different services. More information would be available in Chinese at:
[平台商产品 （专为交易平台所提供的收款方案）](https://b.alipay.com/order/productSet.htm)

Currently supports the following Alipay service types:
- Instant Payment / 支付宝即时到账收款接口 (tested and supported)
Through an instant payment channel, an Alipay account owner is able to remit
directly to seller, enabling quick gathering and withdrawal of funds, resulting
in an improved utilization of funds.

More work and testing to be done on:
- Escrow Payment / 支付宝担保交易收款接口
The third-party (Alipay) guarantees that the buyer can receive goods and seller
can obtain the funds. The usually procedure being:
Buyer Pays >> Seller Sends Goods >> Buyer Confirms Goods >> Seller Confirms
Payment.
Due to its more complete workflow, it is considered as one of the most trusted
online payments for buyers.

@TODO: [Commerce Delivery](http://drupal.org/project/commerce)
- Dual-function Payment / 支付宝双功能收款接口
Allows the buyer to select between Instant Payment and Escrow Payment to
process the funds, providing more flexibility for the buyer.

- Delivery Confirmation / 确认发货接口
Enables merchants to confirm the sending of goods by clicking the "Send button"
without logging into their Alipay account. The corresponding product/order
information would be synchronized to the corresponding Alipay account.

Module currently supports "Instant Payment" service account types.
Other service types could be supported to some extent with some customization
through the hook_commerce_alipay_parameter_alter (see API document).

Debug mode:
It is possible to enable a debugging mode, to override all transactions to a
total of 0.01 CNY, which is very useful for testing everything has been
correctly setup and ensuring payments could effectively be received.


Installation and configuration
------------------------------
a. Prerequisites:
Requires Drupal Commerce to be installed and more particularly the Commerce
Payment module to be enabled (more Commerce modules would also be required:
Commerce UI and Order).
More information at: Installation and uninstallation guide for Drupal Commerce
[https://drupal.org/node/1007434]

b. Download the module and copy it into your contributed modules folder:
[for example, your_drupal_path/sites/all/modules] and enable it from the
modules administration/management page.
More information at: Installing contributed modules (Drupal 7)
[http://drupal.org/documentation/install/modules-themes/modules-7]

2 - Configuration:
After successful installation, browse to the "Payment Methods" management page
under: Home » Administration » Store » Configuration » Payment methods
Path: admin/commerce/config/payment-methods or use the "Configure" link
displayed on the Modules management page.

Enable the Alipay payment method, as described in the Drupal Commerce Payments
User Guide at: http://www.drupalcommerce.org/user-guide/payments
Follow all other steps as described in the Payments User Guide, edit the Alipay
payment method (Rule) and then edit the Action "Enable payment method: Alipay".

Configure the form Payment Settings as required with:
- Seller's registered email address
- Partner ID and Key provided by Alipay's API after account registration for
the corresponding type of Service.

For testing configuration settings, feel free to enable the "Debug mode" from
the same settings form. For more information on the "Debug mode", please check
the paragraph above, called "Debug mode".
(Make sure all the settings are saved each time)


Useful Resources
----------------
For any questions, some help could be found on the Alipay official site:
1 - Merchant Help Center:
https://b.alipay.com/support/helperApply.htm?action=consultationApply
Apply for integration consultation, and you could expect a professional
technician to contact you.
2 - User Help Center:
http://help.alipay.com/support/232511-16307/0-16307.htm?sh=Y&info_type=9
3 - Alipay Forum:
http://club.alipay.com/read-htm-tid-8681712.html
4 - Alipay's Hotline: 0571-88158090 (Monday - Friday, 9:00 to 18:00)


Bugs/Features/Patches
---------------------
If you would like to report bugs, feature requests, or submit a patch, please
do so in the project's issue tracker at:
https://drupal.org/project/issues/commerce_alipay


Contributions are welcome!!
---------------------------
Feel free to follow up in the issue queue for any contributions, 
bug reports, feature requests.
Tests, feedback or comments in general are highly appreciated.
