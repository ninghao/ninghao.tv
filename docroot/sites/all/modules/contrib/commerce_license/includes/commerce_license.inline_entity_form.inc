<?php

/**
 * @file
 * Defines the IEF controller for the commerce_license entity type.
 */

class CommerceLicenseInlineEntityFormController extends EntityInlineEntityFormController {

  /**
   * Overrides EntityInlineEntityFormController::settingsForm().
   */
  public function settingsForm($field, $instance) {
    $form = parent::settingsForm($field, $instance);

    // Adding existing entities is not supported for licenses.
    $form['allow_existing']['#access'] = FALSE;
    $form['match_operator']['#access'] = FALSE;

    return $form;
  }

  /**
   * Overrides EntityInlineEntityFormController::entityForm().
   */
  public function entityForm($entity_form, &$form_state) {
    $license = $entity_form['#entity'];
    // Only show the form if the license is configurable.
    if ($license->isConfigurable()) {
      $license->form($entity_form, $form_state);

      // Restore any form errors and show them above the form.
      if (!empty($form_state['errors'])) {
        $form_errors = &drupal_static('form_set_error', array());
        $form_errors = $form_state['errors'];
        $error_messages = array('error' => array_values($form_errors));
        $_SESSION['messages'] = array_merge_recursive($error_messages, drupal_get_messages());

        $entity_form['messages'] = array(
          '#markup' => theme('status_messages'),
          '#weight' => -99,
        );
      }
    }

    return $entity_form;
  }

  /**
   * Overrides EntityInlineEntityFormController::entityFormValidate().
   */
  public function entityFormValidate($entity_form, &$form_state) {
    $license = $entity_form['#entity'];
    // Only validate the form if the license is configurable.
    if ($license->isConfigurable()) {
      $license->formValidate($entity_form, $form_state);

      // If validation failed, save the errors, clear them, and rebuild the form.
      // This allows them to be displayed by entityForm() in a more appropriate
      // place than the top of the page.
      $form_errors = &drupal_static('form_set_error', array());
      if (!empty($form_errors)) {
        drupal_get_messages('error', TRUE);
        $form_state['errors'] = $form_errors;
        $form_errors = array();
        $form_state['rebuild'] = TRUE;
      }
      else {
        $form_state['errors'] = array();
      }
    }
  }

  /**
   * Overrides EntityInlineEntityFormController::entityFormSubmit().
   */
  public function entityFormSubmit(&$entity_form, &$form_state) {
    $license = $entity_form['#entity'];
    // If IEF is being shown on an order form, use it to get the uid.
    if (!empty($form_state['commerce_order'])) {
      $license->uid = $form_state['commerce_order']->uid;
    }
    else {
      $license->uid = $GLOBALS['user']->uid;
    }
    // $license->product_id is maintained by
    // commerce_license_commerce_line_item_presave().

    // Only submit the form if the license is configurable.
    if ($license->isConfigurable()) {
      $license->formSubmit($entity_form, $form_state);
    }
  }
}
