<?php
/**
* @file
* Default theme implementation for the last time we checked for update data.
*
* Available variables:
* - $last_checked: User interface string with the formatted time ago when the
 *  site last checked for available updates.
* - $link: A link to manually check available updates.
*
* @see template_preprocess_l10n_update_last_check()
*
* @ingroup themeable
*/
?>
<div class="l10n_update-checked">
  <p><?php print $last_checked; ?> <span class="check-manually">(<?php print $link; ?>)</span></p>
</div>
