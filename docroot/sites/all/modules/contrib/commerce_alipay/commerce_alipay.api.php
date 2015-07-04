<?php

/**
 * @file
 * API documentation for the Commerce Alipay module.
 */

/**
 * Allows modules to alter defined Alipay API transaction parameters.
 *
 * The Commerce Alipay module defines by default the necessary properties to
 * provide support for the "Direct Pay" Alipay service type.
 * However, Alipay provides other types of services which may require different
 * properties and parameters to be passed when redirecting users for payment.
 * This hook allows other modules to alter these parameters to support other
 * service types, such as "DualFun", by providing additional keyed properties
 * as requested by the Alipay API.
 *
 * A few examples of the possible parameters that could be used along with this
 * hook are described in the implementation example below.
 * For more complete information on the Alipay API, please see:
 * http://club.alipay.com/read-htm-tid-9976972.html
 *
 * @param array $data
 *   An array of keyed properties to be passed to Alipay's payment gateway, in
 *   compliance with its API. Some of the possible keyed properties have been
 *   described in the implementation example below. To disable extensions or
 *   any key of the array, unset it or assign the parameter to null.
 * @param array $settings
 *   An array of the current settings configured for the payment method.
 * @param stdClass $order
 *   If available, the order object for which the payment should be processed.
 *
 * @see commerce_alipay_redirect_form()
 */
function hook_commerce_alipay_parameter_alter(&$data, $settings, $order) {

  switch ($data['service']) {

    // Payment service: Direct Pay (Instant Payment Interface).
    case 'create_direct_pay_by_user':
      $data = array(
        // The PartnerID is a string/key provided by Alipay to use its API.
        'partner' => 'ExamplePartnerID',
        // Payment type is required.
        'payment_type' => '1',

        // Server asynchronous notification page path, required. This is
        // supposed to be the URL where Alipay's callback notifications should
        // be received. It should start with http://, without custom parameters
        // such as ?id+123.
        'notify_url' => 'http://www.example.com/commerce_alipay/notify',

        // Asynchronous redirection notification page path, required. It should
        // start with http://, not http://localhost/, without custom parameters
        // such as ?id+123.
        'return_url' => 'http://www.example.com/create_direct_pay_by_user-PHP-UTF-8/return_url.php',

        // Alipay account seller's email, required.
        'seller_email' => 'seller_email@example.com',

        // Merchant unique order number, required.
        'out_trade_no' => 1357884681,

        // Order title, required.
        'subject' => 'Order number 1357884681',

        // Total fee, decimal amount in CNY, required.
        'total_fee' => 1234,

        // Order description: text displayed on Alipay's payment page.
        'body' => 'Item example 1234',

        // Order URL, starting with http://,
        // for example http://www.www.com/myorder.html.
        'show_url' => 'http://www.example.com/myorder.html',

        // To be called from class file Submit. Not required, but if you would
        // like to improve the security level for the payment to proceed, it is
        // officially recommended by Alipay.
        'anti_phishing_key' => query_timestamp(),

        // Non-intranet IP address, for example 221.0.0.1.
        'exter_invoke_ip' => '221.0.0.1',

        // Character set for the text passed for the transaction.
        '_input_charset' => 'UTF-8',
      );
      break;

    // Payment service: Escrow Pay (Secured Transactions Interface).
    case 'create_partner_trade_by_buyer':
      // Payment service: DualFun (Instant Payment and Secured Transactions).
    case 'trade_create_by_buyer':
      // The PartnerID is a string/key provided by Alipay to use its API.
      $data['partner'] = 'ExamplePartnerID';
      // Payment type is required.
      $data['payment_type'] = '1';

      // Server asynchronous notification page path, required. This is
      // supposed to be the URL where Alipay's callback notifications should
      // be received. It should start with http://, without custom parameters
      // such as ?id+123.
      $data['notify_url'] = 'http://www.example.com/commerce_alipay/notify';

      // Asynchronous redirection notification page path, required. It should
      // start with http://, not http://localhost/, without custom parameters
      // such as ?id+123.
      $data['return_url'] = 'http://www.example.com/create_partner_trade_by_buyer-PHP-UTF-8/return_url.php';

      // Alipay account seller's email, required.
      $data['seller_email'] = 'seller_email@example.com';

      // Merchant unique order number, required.
      $data['out_trade_no'] = 1357884681;

      // Order title, required.
      $data['subject'] = 'Order number 1357884681';

      // Total fee, amount to be paid, required.
      $data['price'] = 100;

      // Product quantity, required. It is recommended to set the default value
      // to 1 to consider a transaction as an order rather than a product.
      $data['quantity'] = 1;

      // Logistics fee or shipping fee, required.
      $data['logistics_fee'] = 5.75;

      // Logistics type, required. Alternatives are EXPRESS, POST and EMS.
      $data['logistics_type'] = 'EXPRESS';

      // Order description: text displayed on Alipay's payment page.
      $data['body'] = 'Item example 1234';

      // Order URL, starting with http://,
      // for example http://www.www.com/myorder.html.
      $data['show_url'] = 'http://www.example.com/myorder.html';

      // Logistics payment, required. Alternatives are SELLER_PAY (paid by
      // sellers) and BUYER_PAY (paid by buyers).
      $data['logistics_payment'] = 'SELLER_PAY';

      // Receiver's name.
      $data['receive_name'] = 'Example Name';

      // Receiver's full address, including information about province, city,
      // district, road, building, room, etc...
      $data['receive_address'] = 'XX省XXX市XXX区XXX路XXX小区XXX栋XXX单元XXX号';

      // Receiver's Postal Code.
      $data['receive_zip'] = '123456';

      // Receiver's telephone number.
      $data['receive_phone'] = '0571-XXXXXXXX';

      // Receiver's mobile phone number.
      $data['receive_mobile'] = '131XXXXXXXX';
      break;

    // Payment service: SendConfirm (Delivery confirmed Interface).
    case 'send_goods_confirm_by_platform':
      // Alipay trade number, required.
      $data['trade_no'] = '1357884681';

      // Alipay trade number, required.
      $data['logistics_name'] = 'Example Provider Name';

      // Invoice number.
      $data['invoice_no'] = '1357884681';

      // Transportation type. Alternatives are POST, EXPRESS and EMS.
      $data['transport_type'] = 'POST';
      break;

    // Default configuration from commerce_alipay_redirect_form.
    default:
      $data = array(
        'service' => $settings['service'],
        'payment_type' => '1',
        'partner' => $settings['partner'],
        'seller_email' => $settings['seller_email'],
        'return_url' => $settings['return'],
        'notify_url' => $settings['notify'],
        '_input_charset' => 'UTF-8',
        'show_url' => $settings['return'],
        'out_trade_no' => $order->order_number,
        'subject' => t('order !order_id', array('!order_id' => $order->order_number)),
        'body' => t('order !order_id', array('!order_id' => $order->order_number)),
        'total_fee' => commerce_currency_amount_to_decimal($amount, 'CNY'),
        'sign_type' => 'MD5',
      );
      break;
  }
}

/**
 * Anti-phishing function.
 *
 * Note: PHP5+ is required. Make sure that the server and local host support
 * DOMDocument and SSL.
 *
 * @param string $encode
 *   Optionally specify a specific character encoding. UTF-8 by default.
 *
 * @return int
 *   Encrypted key.
 *
 * @see hook_commerce_alipay_parameter_alter()
 */
function query_timestamp($encode = 'utf-8') {
  $url = "https://mapi.alipay.com/gateway.do?service=query_timestamp&partner=" . trim(strtolower($encode));
  $encrypt_key = '';
  $doc = new DOMDocument();
  $doc->load($url);
  $item_encrypt_key = $doc->getElementsByTagName('encrypt_key');
  $encrypt_key = $item_encrypt_key->item(0)->nodeValue;
  return $encrypt_key;
}
