<?php

/**
 * @file
 * Template file for the condition form.
 */
$form['parameter']['term_id']['settings']['term_operator'] = $form['parameter']['term_operator']['settings']['term_operator'];
$form['parameter']['term_id']['settings'] = array_reverse($form['parameter']['term_id']['settings'], TRUE);
unset($form['parameter']['term_operator']);
echo drupal_render_children($form);
