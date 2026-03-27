.. _radosgw-admin-guide:

============
 Admin Guide
============

.. note::

   Command reference blocks auto-generated on March 27, 2026.
   Narrative sections between ``NARRATIVE-BEGIN/END`` markers
   are preserved across regenerations and hand-edited by authors.

After the Ceph Object Storage service is up and running, it can be
administered with user management, access controls, quotas, and usage tracking.

Radosgw-Admin
=============

.. # NARRATIVE-BEGIN: section-radosgw-admin
.. # NARRATIVE-END: section-radosgw-admin

User Create a Radosgw-Admin
---------------------------

.. _radosgw-admin-radosgw-admin-user-create:

.. # NARRATIVE-BEGIN: radosgw-admin-user-create
.. # NARRATIVE-END: radosgw-admin-user-create

.. code-block:: bash
   radosgw-admin radosgw-admin user create --uid=<value> --display-name=<value> [--email=<value>] [--admin=<value>] [--max-buckets=<value>] [--format=<value>] [--tenant=<value>]

**Required options:**

``--display-name``
  User display name

``--uid``
  The user ID

**Optional options:**

``--admin``
  Set the admin flag on the user

``--email``
  The email address of the user

``--format``
  Output format for the response: json, xml

``--max-buckets``
  Maximum number of buckets for the user (server default: 1000 if not specified)

``--tenant``
  Tenant name

User Info a Radosgw-Admin
-------------------------

.. _radosgw-admin-radosgw-admin-user-info:

.. # NARRATIVE-BEGIN: radosgw-admin-user-info
.. # NARRATIVE-END: radosgw-admin-user-info

.. code-block:: bash
   radosgw-admin radosgw-admin user info --uid=<value> [--display-name=<value>] [--format=<value>] [--tenant=<value>]

**Required options:**

``--uid``
  The user ID

**Optional options:**

``--display-name``
  User display name

``--format``
  Output format for the response: json, xml

``--tenant``
  Tenant name

User Modify a Radosgw-Admin
---------------------------

.. _radosgw-admin-radosgw-admin-user-modify:

.. # NARRATIVE-BEGIN: radosgw-admin-user-modify
.. # NARRATIVE-END: radosgw-admin-user-modify

.. code-block:: bash
   radosgw-admin radosgw-admin user modify --uid=<value> [--display-name=<value>] [--email=<value>] [--admin=<value>] [--format=<value>] [--tenant=<value>]

**Required options:**

``--uid``
  The user ID

**Optional options:**

``--admin``
  Set or clear the admin flag on the user

``--display-name``
  User display name

``--email``
  New email address for the user

``--format``
  Output format for the response: json, xml

``--tenant``
  Tenant name

User Rm a Radosgw-Admin
-----------------------

.. _radosgw-admin-radosgw-admin-user-rm:

.. # NARRATIVE-BEGIN: radosgw-admin-user-rm
.. # NARRATIVE-END: radosgw-admin-user-rm

.. code-block:: bash
   radosgw-admin radosgw-admin user rm --uid=<value> [--display-name=<value>] [--tenant=<value>] [--purge-data=<value>]

**Required options:**

``--uid``
  The user ID

**Optional options:**

``--display-name``
  User display name

``--purge-data``
  Also purge all data associated with the user

``--tenant``
  Tenant name

User List a Radosgw-Admin
-------------------------

.. _radosgw-admin-radosgw-admin-user-list:

.. # NARRATIVE-BEGIN: radosgw-admin-user-list
.. # NARRATIVE-END: radosgw-admin-user-list

.. code-block:: bash
   radosgw-admin radosgw-admin user list [--format=<value>]

**Optional options:**

``--format``
  Output format for the response: json, xml

User Suspend a Radosgw-Admin
----------------------------

.. _radosgw-admin-radosgw-admin-user-suspend:

.. # NARRATIVE-BEGIN: radosgw-admin-user-suspend
.. # NARRATIVE-END: radosgw-admin-user-suspend

.. code-block:: bash
   radosgw-admin radosgw-admin user suspend --uid=<value> [--display-name=<value>]

**Required options:**

``--uid``
  The user ID

**Optional options:**

``--display-name``
  User display name

User Enable a Radosgw-Admin
---------------------------

.. _radosgw-admin-radosgw-admin-user-enable:

.. # NARRATIVE-BEGIN: radosgw-admin-user-enable
.. # NARRATIVE-END: radosgw-admin-user-enable

.. code-block:: bash
   radosgw-admin radosgw-admin user enable --uid=<value> [--display-name=<value>]

**Required options:**

``--uid``
  The user ID

**Optional options:**

``--display-name``
  User display name

User Stats a Radosgw-Admin
--------------------------

.. _radosgw-admin-radosgw-admin-user-stats:

.. # NARRATIVE-BEGIN: radosgw-admin-user-stats
.. # NARRATIVE-END: radosgw-admin-user-stats

.. code-block:: bash
   radosgw-admin radosgw-admin user stats --uid=<value> [--display-name=<value>] [--sync-stats=<value>]

**Required options:**

``--uid``
  The user ID

**Optional options:**

``--display-name``
  User display name

``--sync-stats``
  Update user stats with current stats from bucket indexes

Bucket List a Radosgw-Admin
---------------------------

.. _radosgw-admin-radosgw-admin-bucket-list:

.. # NARRATIVE-BEGIN: radosgw-admin-bucket-list
.. # NARRATIVE-END: radosgw-admin-bucket-list

.. code-block:: bash
   radosgw-admin radosgw-admin bucket list [--bucket=<value>] [--bucket-id=<value>] [--allow-unordered=<value>] [--format=<value>] [--tenant=<value>]

**Optional options:**

``--allow-unordered``
  Faster unsorted listing, removes ordering requirement

``--bucket``
  The bucket name

``--bucket-id``
  The bucket ID

``--format``
  Output format for the response: json, xml

``--tenant``
  Tenant name

Bucket Stats a Radosgw-Admin
----------------------------

.. _radosgw-admin-radosgw-admin-bucket-stats:

.. # NARRATIVE-BEGIN: radosgw-admin-bucket-stats
.. # NARRATIVE-END: radosgw-admin-bucket-stats

.. code-block:: bash
   radosgw-admin radosgw-admin bucket stats --bucket=<value> [--bucket-id=<value>] [--format=<value>] [--tenant=<value>]

**Required options:**

``--bucket``
  The bucket name

**Optional options:**

``--bucket-id``
  The bucket ID

``--format``
  Output format for the response: json, xml

``--tenant``
  Tenant name

Bucket Rm a Radosgw-Admin
-------------------------

.. _radosgw-admin-radosgw-admin-bucket-rm:

.. # NARRATIVE-BEGIN: radosgw-admin-bucket-rm
.. # NARRATIVE-END: radosgw-admin-bucket-rm

.. code-block:: bash
   radosgw-admin radosgw-admin bucket rm --bucket=<value> [--bucket-id=<value>] [--purge-objects=<value>]

**Required options:**

``--bucket``
  The bucket name

**Optional options:**

``--bucket-id``
  The bucket ID

``--purge-objects``
  Remove all objects before deleting (required for non-empty buckets)

Bucket Link a Radosgw-Admin
---------------------------

.. _radosgw-admin-radosgw-admin-bucket-link:

.. # NARRATIVE-BEGIN: radosgw-admin-bucket-link
.. # NARRATIVE-END: radosgw-admin-bucket-link

.. code-block:: bash
   radosgw-admin radosgw-admin bucket link --bucket=<value> [--bucket-id=<value>] --uid=<value> [--display-name=<value>] [--tenant=<value>]

**Required options:**

``--bucket``
  The bucket name

``--uid``
  The user ID

**Optional options:**

``--bucket-id``
  The bucket ID

``--display-name``
  User display name

``--tenant``
  Tenant name

Bucket Unlink a Radosgw-Admin
-----------------------------

.. _radosgw-admin-radosgw-admin-bucket-unlink:

.. # NARRATIVE-BEGIN: radosgw-admin-bucket-unlink
.. # NARRATIVE-END: radosgw-admin-bucket-unlink

.. code-block:: bash
   radosgw-admin radosgw-admin bucket unlink --bucket=<value> [--bucket-id=<value>] --uid=<value> [--display-name=<value>]

**Required options:**

``--bucket``
  The bucket name

``--uid``
  The user ID

**Optional options:**

``--bucket-id``
  The bucket ID

``--display-name``
  User display name

Bucket Check a Radosgw-Admin
----------------------------

.. _radosgw-admin-radosgw-admin-bucket-check:

.. # NARRATIVE-BEGIN: radosgw-admin-bucket-check
.. # NARRATIVE-END: radosgw-admin-bucket-check

.. code-block:: bash
   radosgw-admin radosgw-admin bucket check [--bucket=<value>] [--bucket-id=<value>] [--fix=<value>]

**Optional options:**

``--bucket``
  The bucket name

``--bucket-id``
  The bucket ID

``--fix``
  Fix the bucket index in addition to checking it

Bucket Logging Flush a Radosgw-Admin
------------------------------------

.. _radosgw-admin-radosgw-admin-bucket-logging-flush:

.. # NARRATIVE-BEGIN: radosgw-admin-bucket-logging-flush
.. # NARRATIVE-END: radosgw-admin-bucket-logging-flush

.. code-block:: bash
   radosgw-admin radosgw-admin bucket logging flush --bucket=<value>

**Required options:**

``--bucket``
  The source bucket to flush logs for

Bucket Logging Info a Radosgw-Admin
-----------------------------------

.. _radosgw-admin-radosgw-admin-bucket-logging-info:

.. # NARRATIVE-BEGIN: radosgw-admin-bucket-logging-info
.. # NARRATIVE-END: radosgw-admin-bucket-logging-info

.. code-block:: bash
   radosgw-admin radosgw-admin bucket logging info [--bucket=<value>]

**Optional options:**

``--bucket``
  The source bucket to get logging info for

Bucket Logging List a Radosgw-Admin
-----------------------------------

.. _radosgw-admin-radosgw-admin-bucket-logging-list:

.. # NARRATIVE-BEGIN: radosgw-admin-bucket-logging-list
.. # NARRATIVE-END: radosgw-admin-bucket-logging-list

.. code-block:: bash
   radosgw-admin radosgw-admin bucket logging list --bucket=<value>

**Required options:**

``--bucket``
  The source bucket

Quota Set a Radosgw-Admin
-------------------------

.. _radosgw-admin-radosgw-admin-quota-set:

.. # NARRATIVE-BEGIN: radosgw-admin-quota-set
.. # NARRATIVE-END: radosgw-admin-quota-set

.. code-block:: bash
   radosgw-admin radosgw-admin quota set [--uid=<value>] [--display-name=<value>] [--bucket=<value>] [--bucket-id=<value>] --quota-scope=<value> [--max-objects=<value>] [--max-size=<value>] [--format=<value>]

**Required options:**

``--quota-scope``
  Scope of the quota: bucket, user, or account

**Optional options:**

``--bucket``
  The bucket name

``--bucket-id``
  The bucket ID

``--display-name``
  User display name

``--format``
  Output format for the response: json, xml

``--max-objects``
  Maximum number of objects (negative value to disable)

``--max-size``
  Maximum storage size in B/K/M/G/T (negative value to disable)

``--uid``
  The user ID

Quota Enable a Radosgw-Admin
----------------------------

.. _radosgw-admin-radosgw-admin-quota-enable:

.. # NARRATIVE-BEGIN: radosgw-admin-quota-enable
.. # NARRATIVE-END: radosgw-admin-quota-enable

.. code-block:: bash
   radosgw-admin radosgw-admin quota enable [--uid=<value>] [--display-name=<value>] --quota-scope=<value>

**Required options:**

``--quota-scope``
  Scope: bucket or user

**Optional options:**

``--display-name``
  User display name

``--uid``
  The user ID

Quota Disable a Radosgw-Admin
-----------------------------

.. _radosgw-admin-radosgw-admin-quota-disable:

.. # NARRATIVE-BEGIN: radosgw-admin-quota-disable
.. # NARRATIVE-END: radosgw-admin-quota-disable

.. code-block:: bash
   radosgw-admin radosgw-admin quota disable [--uid=<value>] [--display-name=<value>] --quota-scope=<value>

**Required options:**

``--quota-scope``
  Scope: bucket or user

**Optional options:**

``--display-name``
  User display name

``--uid``
  The user ID

Ratelimit Set a Radosgw-Admin
-----------------------------

.. _radosgw-admin-radosgw-admin-ratelimit-set:

.. # NARRATIVE-BEGIN: radosgw-admin-ratelimit-set
.. # NARRATIVE-END: radosgw-admin-ratelimit-set

.. code-block:: bash
   radosgw-admin radosgw-admin ratelimit set [--max-read-ops=<value>] [--max-write-ops=<value>] [--max-read-bytes=<value>] [--max-write-bytes=<value>]

**Optional options:**

``--max-read-bytes``
  Max read bytes per interval (0 = unlimited)

``--max-read-ops``
  Max read requests per interval (0 = unlimited)

``--max-write-bytes``
  Max write bytes per interval (0 = unlimited)

``--max-write-ops``
  Max write requests per interval (0 = unlimited)

Ratelimit Get a Radosgw-Admin
-----------------------------

.. _radosgw-admin-radosgw-admin-ratelimit-get:

.. # NARRATIVE-BEGIN: radosgw-admin-ratelimit-get
.. # NARRATIVE-END: radosgw-admin-ratelimit-get

.. code-block:: bash
   radosgw-admin radosgw-admin ratelimit get [--uid=<value>] [--display-name=<value>] [--bucket=<value>] [--bucket-id=<value>] --ratelimit-scope=<value> [--format=<value>]

**Required options:**

``--ratelimit-scope``
  Scope: bucket, user, or anonymous

**Optional options:**

``--bucket``
  The bucket name

``--bucket-id``
  The bucket ID

``--display-name``
  User display name

``--format``
  Output format for the response: json, xml

``--uid``
  The user ID

Ratelimit Enable a Radosgw-Admin
--------------------------------

.. _radosgw-admin-radosgw-admin-ratelimit-enable:

.. # NARRATIVE-BEGIN: radosgw-admin-ratelimit-enable
.. # NARRATIVE-END: radosgw-admin-ratelimit-enable

.. code-block:: bash
   radosgw-admin radosgw-admin ratelimit enable [--uid=<value>] [--display-name=<value>] --ratelimit-scope=<value>

**Required options:**

``--ratelimit-scope``
  Scope: bucket, user, or anonymous

**Optional options:**

``--display-name``
  User display name

``--uid``
  The user ID

Ratelimit Disable a Radosgw-Admin
---------------------------------

.. _radosgw-admin-radosgw-admin-ratelimit-disable:

.. # NARRATIVE-BEGIN: radosgw-admin-ratelimit-disable
.. # NARRATIVE-END: radosgw-admin-ratelimit-disable

.. code-block:: bash
   radosgw-admin radosgw-admin ratelimit disable [--uid=<value>] [--display-name=<value>] --ratelimit-scope=<value>

**Required options:**

``--ratelimit-scope``
  Scope: bucket, user, or anonymous

**Optional options:**

``--display-name``
  User display name

``--uid``
  The user ID


