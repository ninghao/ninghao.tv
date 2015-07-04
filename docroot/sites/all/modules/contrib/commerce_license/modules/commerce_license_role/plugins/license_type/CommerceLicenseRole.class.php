<?php

/**
 * Role license type.
 */
class CommerceLicenseRole extends CommerceLicenseBase  {

  /**
   * Implements CommerceLicenseInterface::isConfigurable().
   */
  public function isConfigurable() {
    return FALSE;
  }

  /**
   * Overrides Entity::save().
   *
   * Maintains the role, adding or removing it from the owner when necessary.
   */
  public function save() {
    if ($this->uid && $this->product_id) {
      $role = $this->wrapper->product->commerce_license_role->value();
      $owner = $this->wrapper->owner->value();
      $save_owner = FALSE;
      if (!empty($this->license_id)) {
        $this->original = entity_load_unchanged('commerce_license', $this->license_id);
        // A plan change occurred. Remove the previous role.
        if ($this->original->product_id && $this->product_id != $this->original->product_id) {
          $previous_role = $this->original->wrapper->product->commerce_license_role->value();
          if (isset($owner->roles[$previous_role])) {
            unset($owner->roles[$previous_role]);
            $save_owner = TRUE;
          }
        }
      }
      // The owner of an active license must have the role.
      if ($this->status == COMMERCE_LICENSE_ACTIVE) {
        if (!isset($owner->roles[$role])) {
          $owner->roles[$role] = $role;
          $save_owner = TRUE;
        }
      }
      elseif ($this->status > COMMERCE_LICENSE_ACTIVE) {
        // The owner of an inactive license must not have the role.
        if (isset($owner->roles[$role])) {
          unset($owner->roles[$role]);
          $save_owner = TRUE;
        }
      }

      // If a role was added or removed, save the owner.
      if ($save_owner) {
        user_save($owner);
      }
    }

    parent::save();
  }
}
