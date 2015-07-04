<?php

/**
 * @file
 * Rules condition Has Terms.
 */

/**
 * Helper function to return the condition info to the main module.
 */
function commerce_rules_extra_rules_condition_has_terms_condition_info() {
  $info = array(
    'group' => t('Commerce Line Item'),
    'label' => t('Line item product has term(s)'),
    'parameter' => array(
      'line_item' => array(
        'type' => 'commerce_line_item',
        'label' => t('Commere Line Item'),
      ),
    ),
  );
  $condition = array_merge_recursive($info, commerce_rules_extra_terms_parameters());
  return $condition;
}

/**
 * Creates a list of term parameters.
 */
function commerce_rules_extra_terms_parameters($required = TRUE) {
  $paramaters = array(
    'parameter' => array(
      'voc_name' => array(
        'type' => 'text',
        'label' => t('Term Reference Field'),
        'description' => t('The machine-name of the expected product\'s term reference field'),
        'options list' => ($required ? 'commerce_rules_extra_term_fields_options_list' : 'commerce_rules_extra_term_fields_optional_options_list'),
        'restriction' => 'input',
        'optional' => !$required,
      ),
      'term_id' => array(
        'type' => 'list<integer>',
        'label' => t('Taxonomy Term(s)'),
        'description' => t('The term(s) being checked for'),
        'restriction' => 'input',
        'options list' => 'commerce_rules_extra_empty_options_list',
        'optional' => TRUE,
      ),
      'product_display' => array(
        'type' => 'boolean',
        'label' => t('Search terms in product display, not in product'),
        'description' => t('If checked search will be made on product display.'),
        'restriction' => 'input',
      ),
      'term_operator' => array(
        'type' => 'text',
        'label' => t('Operator'),
        'description' => t('Selection mode of terms'),
        'options list' => 'commerce_rules_extra_get_operators',
        'restriction' => 'input',
        'optional' => !$required,
      ),
    ),
  );
  
  return $paramaters;
}

/**
 * Implements hook_form_alter().
 */
function commerce_rules_extra_rules_condition_has_terms_form_alter(&$form, &$form_state, $options, RulesAbstractPlugin $element) {
  // Require the term reference field be selected first so that the term_ids list can be populated.
  // Change the list parameter to be not editable any more.
  $form['parameter']['voc_name']['settings']['voc_name']['#ajax'] = array(
    'event' => 'change',
    'callback' => 'commerce_rules_extra_ajax_load_terms',
    'wrapper' => 'cre_terms',
  );
  $form['parameter']['term_operator']['settings']['term_operator']['#prefix'] = '<div style="float:left">';
  $form['parameter']['term_operator']['settings']['term_operator']['#title'] = 'Operator';
  $form['parameter']['term_operator']['settings']['term_operator']['#suffix'] = '</div>';
  $form['parameter']['term_id']['settings']['term_id']['#prefix'] = '<div id="cre_terms" style="float:left; margin-left:10px;">';
  $form['parameter']['term_id']['settings']['term_id']['#options'] = (empty($element->settings['voc_name']) ? array() : commerce_rules_extra_get_terms_list($element->settings['voc_name']));
  $form['parameter']['term_id']['settings']['term_id']['#suffix'] = '</div>';
  $form["#theme"] = 'form--cre_condition';
}

/**
 * Callback function for Has Terms condition to check if a line item has term ids.
 */
function commerce_rules_extra_rules_condition_has_terms($line_item, $voc_name, $term_ids, $product_display, $term_operator) {
  if (!empty($line_item)) {
    $wrapper = entity_metadata_wrapper('commerce_line_item', $line_item);
    $product = $wrapper->commerce_product->value();
    $product_terms = array();
    if ($product_display) {
      if (empty($line_item->line_item_id) && (isset($product->display_context))) {
        $line_item->data['context'] = $product->display_context;
      }
      $node = commerce_rules_extra_node_from_line_item($line_item);
      $node = $node['referencing_node'];
      if (NULL == $node) {
        return FALSE;
      }
      $query = db_select('taxonomy_index', 't')->fields('t', array('tid'))->condition('nid', $node->nid, '=');
      $results = $query->execute();
      foreach ($results as $t) {
        $product_terms[] = $t;
      }
    }
    else {
      foreach (commerce_rules_extra_get_fields_for_vocabulary($voc_name) as $field) {
        if (isset($product->$field)) {
          $product_terms = $wrapper->commerce_product->$field->value();
          break;
        }
      }
    }
    if ($term_operator == 'IE') {
      return empty($product_terms);
    }
    if ($term_operator == 'INE') {
      return !empty($product_terms);
    }
    if (!empty($product_terms)) {

      if (!is_array($product_terms)) {
        $t = $product_terms;
        $product_terms = array(0 => $t);
      }
      switch ($term_operator) {
        case 'IAO':
          foreach ($term_ids as $term_id) {
            // If their term reference field is set to allow more than one term.
            // $product_terms will be an array.
            $has_term = FALSE;
            foreach ($product_terms as $product_term) {
              if ($product_term->tid == $term_id) {
                $has_term = TRUE;
              }
            }
            if (!$has_term) {
              return FALSE;
            }
          }
          // If we haven't returned FALSE already then TRUE must be the answer.
          return TRUE;

          break;

        case 'IOO':
          foreach ($term_ids as $term_id) {
            foreach ($product_terms as $product_term) {
              if ($product_term->tid == $term_id) {
                return TRUE;
              }
            }
          }
          break;

        case 'INO':
          foreach ($term_ids as $term_id) {
            $has_term = FALSE;
            foreach ($product_terms as $product_term) {
              if ($product_term->tid == $term_id) {
                return FALSE;
              }
            }
          }
          return TRUE;

          break;
      };
    }
  }

  return FALSE;
}

/**
 * Function to return fields for vocabulary name.
 */
function commerce_rules_extra_get_fields_for_vocabulary($voc) {
  $fields = &drupal_static(__FUNCTION__);

  if (!isset($fields)) {
    $termfields = field_read_fields(array('type' => 'taxonomy_term_reference'));
    foreach ($termfields as $field_name => $infos) {
      $fields[$infos['settings']['allowed_values'][0]['vocabulary']][] = $field_name;
    }
  }
  return $fields[$voc];
}

/**
 * Function to return an array of machine_names for taxonomy term reference fields.
 */
function commerce_rules_extra_term_fields_optional_options_list() {
  $field_options_list = array('' => '<' . t('none') . '>') + commerce_rules_extra_term_fields_options_list();
  return $field_options_list;
}

/**
 * Function to create options lists for the form for has terms.
 */
function commerce_rules_extra_term_fields_options_list() {
  $field_options_list = array();
  $vocs = taxonomy_get_vocabularies();
  foreach ($vocs as $voc) {
    $field_options_list[$voc->machine_name] = $voc->name;
  }

  return $field_options_list;
}

/**
 * Callback function to load terms into form for has terms.
 */
function commerce_rules_extra_ajax_load_terms($form, $form_state) {
  return array(
    '#type' => 'ajax',
    '#commands' => array(ajax_command_insert(NULL, drupal_render($form['parameter']['term_id']['settings']))),
  );
}

/**
 * Function to take the machine name of a Taxonomy reference field and retrieves the terms.
 */
function commerce_rules_extra_get_terms_list($voc_name) {
  if (empty($voc_name)) {
    return array();
  }
  $voc = taxonomy_vocabulary_machine_name_load($voc_name);
  $terms = taxonomy_get_tree($voc->vid);
  $term_list = array();
  if (!empty($terms)) {
    foreach ($terms as $term) {
      $term_list[$term->tid] = str_repeat('-', $term->depth) . $term->name;
    }
  }

  return $term_list;
}

/**
 * Function creates an empty option list.
 *
 * An empty options list so that Rules conditions that need manually populated
 * options sets will have their form variables populated correctly.
 */
function commerce_rules_extra_empty_options_list() {
  return array();
}

/**
 * Funciton to create an option list of operators for forms.
 */
function commerce_rules_extra_get_operators() {
  return array(
    'IOO' => 'is one of',
    'IAO' => 'is all of',
    'INO' => 'is none of',
    'IE' => 'is empty (null)',
    'INE' => 'is not empty(not null)',
  );
}
