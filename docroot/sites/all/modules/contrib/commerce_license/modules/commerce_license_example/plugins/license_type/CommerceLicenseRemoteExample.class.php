<?php

/**
 * Remote example license type.
 */
class CommerceLicenseRemoteExample extends CommerceLicenseRemoteBase  {

  /**
   * Implements EntityBundlePluginProvideFieldsInterface::fields().
   */
  static function fields() {
    $fields = parent::fields();

    // This field stores the api key returned by the remote service.
    // Such a field shouldn't be editable by the customer, of course.
    // Since this license type is not configurable, it's not a problem because
    // there's no form. However, if the license type was configurable,
    // this field instance would need to use the field_extrawidgets_hidden
    // widget provided by the field_extrawidgets module, or set
    // $form['cle_api_key']['#access'] = FALSE in form().
    $fields['cle_api_key']['field'] = array(
      'type' => 'text',
      'cardinality' => 1,
    );
    $fields['cle_api_key']['instance'] = array(
      'label' => t('API Key'),
      'required' => 1,
      'settings' => array(
        'text_processing' => '0',
      ),
    );

    return $fields;
  }

  /**
   * Implements CommerceLicenseInterface::accessDetails().
   */
  public function accessDetails() {
    $output = field_view_field('commerce_license', $this, 'cle_api_key');
    return drupal_render($output);
  }

  /**
   * Implements CommerceLicenseInterface::isConfigurable().
   */
  public function isConfigurable() {
    return FALSE;
  }

  /**
   * Implements CommerceLicenseSynchronizableInterface::synchronize().
   */
  public function synchronize() {
    // The license is being activated.
    if ($this->status == COMMERCE_LICENSE_PENDING) {
      // Simulate a 2s delay in synchronization, as if the service call was done.
      sleep(2);
      // Imagine that the service call returned an api key. Set it.
      $this->wrapper->cle_api_key = sha1($this->license_id);
    }
    elseif ($this->status == COMMERCE_LICENSE_EXPIRED) {
      // The license was just expired. Do something.
    }
    elseif ($this->status == COMMERCE_LICENSE_REVOKED) {
      // The license was just revoked. Do something.
    }

    // Alternatively, set COMMERCE_LICENSE_SYNC_FAILED if the sync failed,
    // or COMMERCE_LICENSE_SYNC_FAILED_RETRY if the sync failed and should
    // be retried.
    $this->wrapper->sync_status = COMMERCE_LICENSE_SYNCED;
    $this->save();
  }
}
