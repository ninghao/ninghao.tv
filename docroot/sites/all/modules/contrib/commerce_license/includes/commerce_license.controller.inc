<?php

/**
 * @file
 * Controller for the commerce_license entity type.
 */

class CommerceLicenseEntityController extends EntityBundlePluginEntityController {

  /**
   * Overrides EntityBundlePluginEntityController::save().
   */
  public function save($entity, DatabaseTransaction $transaction = NULL) {
    $entity->is_new_revision = !empty($entity->is_new_revision) || !empty($entity->revision) || !empty($entity->is_new);
    // New revisions are always set as default.
    if ($entity->is_new_revision) {
      $entity->default_revision = TRUE;
    }
    parent::save($entity, $transaction);

    // Guard against #2291623.
    unset($entity->revision);
  }

  /**
   * Overrides EntityBundlePluginEntityController::saveRevision().
   *
   * Maintains the revision_created and revision_ended timestamps.
   */
  protected function saveRevision($entity) {
    if ($entity->is_new_revision) {
      $current_time = commerce_license_get_time();
      $entity->revision_created = $current_time;
      $entity->revision_ended = 0;

      // A previous revision exists, close it.
      if (!empty($entity->revision_id)) {
        db_update($this->revisionTable)
          ->fields(array('revision_ended'  => $current_time - 1))
          ->condition('revision_id', $entity->revision_id)
          ->execute();
      }
    }

    parent::saveRevision($entity);
  }

  /**
   * Overrides EntityBundlePluginEntityController::delete().
   */
  public function delete($ids, DatabaseTransaction $transaction = NULL) {
    $entities = $ids ? $this->load($ids) : FALSE;
    if (!$entities) {
      // Do nothing, in case invalid or no ids have been passed.
      return;
    }

    try {
      parent::delete($ids, $transaction);

      // Try to delete references to the given licenses.
      // Do this after deleting the entities in question in order to give
      // hook_entity_delete() implementations a chance to do their own thing
      // (for example, deleting both the reference AND the referencing entity).
      foreach ($entities as $entity) {
        commerce_license_delete_references($entity);
      }
    }
    catch (Exception $e) {
      if (isset($transaction)) {
        $transaction->rollback();
      }
      throw $e;
    }
  }
}
