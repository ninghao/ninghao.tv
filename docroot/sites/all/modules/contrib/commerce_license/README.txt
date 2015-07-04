Provides a framework for selling access to local or remote resources.

Features
--------
- Any kind of local or remote resource be licensed.
  Licenses are entities. License types are implemented as classes
  (thanks to Entity Bundle Plugin) and contain all relevant logic.
- A license can be configurable, allowing its fields to be edited straight on
  the add to cart form (through Inline Entity Form), or through a checkout pane.
- Remote licenses are synchronizable, allowing a remote service to be contacted
  each time the status changes (to create / suspend an account, or anything else).
- A checkout pane for the "complete" step shows the access details of the bought
  licenses, and in the case of remote licenses, refreshes itself until
  advanced queue processes the sync.
- Licenses can be time limited, and are automatically expired (by cron) once
  that time passes.

Submodules
----------
- commerce_license_example: demonstrates the API for both local and remote licenses.
- commerce_license_role: allows roles to be licensed. The customer receives the role
  referenced by the purchased license product. Changing $license->product_id (from "Basic Membership" to "Premium Membership", for instance) changes the owner's role.

See https://drupal.org/node/2039687 for information on getting started.
