<?php
/**
 * @file
 * Sample EntityBundlePlugin implementation, 2 of 2.
 */

$plugin = array(
  'title' => t('Bundle 2'),
  'class' => 'EntityBundlePluginTestBundle2',
  'weight' => 1,
);

class EntityBundlePluginTestBundle2 implements EntityBundlePluginProvideFieldsInterface {
  /**
   * @see EntityBundlePluginProvideFieldsInterface::fields()
   */
  public static function fields() {
    $fields['ebp_title']['field'] = array(
      'type' => 'text',
      'cardinality' => 1,
      'translatable' => FALSE,
      'settings' => array(
        'max_length' => 255,
      ),
    );
    $fields['ebp_title']['instance'] = array(
      'label' => 'Title',
      'required' => TRUE,
      'widget' => array(
        'type' => 'text_textfield',
        'settings' => array(
          'size' => 60,
        ),
      ),
    );

    $fields['ebp_description']['field'] = array(
      'type' => 'text',
      'cardinality' => 1,
      'translatable' => FALSE,
      'settings' => array(
        'max_length' => 255,
      ),
    );
    $fields['ebp_description']['instance'] = array(
      'label' => 'Description',
      'required' => TRUE,
      'widget' => array(
        'type' => 'text_textfield',
        'settings' => array(
          'size' => 60,
        ),
      ),
    );

    return $fields;
  }
}
