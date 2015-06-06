<?php
/**
* @file
* Default theme implementation for displaying translation status information.
*
* Displays translation status information per language.
*
* Available variables:
* - module_list: A list of names of modules that have available translation
*   updates.
* - details: Rendered list of the translation details.
* - missing_updates_status: If there are any modules that are missing
*   translation updates, this variable will contain text indicating how many
*   modules are missing translations.
*
* @see template_preprocess_l10n_update_update_info()
*
* @ingroup themeable
*/
?>
<div class="inner" tabindex="0" role="button">
  <span class="update-description-prefix visually-hidden">Show description</span>
  <?php if($module_list): ?>
  <span class="text"><?php print $module_list; ?></span>
  <?php elseif($missing_updates_status): ?>
  <span class="text"><?php print $missing_updates_status; ?></span>
  <?php endif; ?>
  <?php if($details): ?>
  <div class="details"><?php print drupal_render($details); ?></div>
  <?php endif; ?>
</div>
