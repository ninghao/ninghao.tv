<?php

/**
 * Example license type.
 */
class CommerceLicenseExample extends CommerceLicenseBase  {

  /**
   * Implements EntityBundlePluginProvideFieldsInterface::fields().
   */
  static function fields() {
    $fields = parent::fields();

    // A real project could use commerce_single_address and just call
    // commerce_single_address_active_profile_load() to get the current
    // billing profile and all the information within, including the customer
    // name.
    $fields['cle_name']['field'] = array(
      'type' => 'text',
      'cardinality' => 1,
    );
    $fields['cle_name']['instance'] = array(
      'label' => t('Name'),
      'required' => 1,
      'settings' => array(
        'text_processing' => '0',
      ),
      'widget' => array(
        'module' => 'text',
        'type' => 'text_textfield',
        'settings' => array(
          'size' => 20,
        ),
      ),
    );

    return $fields;
  }

  /**
   * Implements CommerceLicenseInterface::accessDetails().
   */
  public function accessDetails() {
    // Just display the name field.
    $output = field_view_field('commerce_license', $this, 'cle_name');
    return drupal_render($output);
  }

  /**
   * Implements CommerceLicenseInterface::isConfigurable().
   */
  public function isConfigurable() {
    return TRUE;
  }

  /**
   * Overrides CommerceLicenseBase::formValidate().
   */
  public function formValidate($form, &$form_state) {
    parent::formValidate($form, $form_state);

    $parents_path = implode('][', $form['#parents']);
    $form_values = drupal_array_get_nested_value($form_state['values'], $form['#parents']);
    // Validate the cle_name field value.
    if ($form_values['cle_name'][LANGUAGE_NONE][0]['value'] == 'John Smith') {
      form_set_error($parents_path . '][cle_name][und][0][value', t('John Smith is not allowed to have licenses.'));
    }
  }

  /**
   * Implements CommerceLicenseInterface::checkoutCompletionMessage().
   */
  public function checkoutCompletionMessage() {
    // A real checkoutCompletionMessage() method would also output the result
    // of $this->accessDetails() here.
    $text = 'Congratulations, ' . $this->wrapper->cle_name->value() . '. <br />';
    $text .= "You are now licensed.";
    return $text;
  }
}
