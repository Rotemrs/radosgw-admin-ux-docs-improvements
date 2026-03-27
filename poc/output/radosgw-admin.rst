:orphan:

.. _man-radosgw-admin:

=================================================================
 radosgw-admin -- rados REST gateway user administration utility
=================================================================

.. program:: radosgw-admin

.. note::

   Auto-generated on March 27, 2026 from ``radosgw-admin --export-tree``.
   **Do not edit manually.**
   Edit command definitions in ``src/tools/radosgw-admin.cc``
   then regenerate: ``radosgw-admin --export-tree | python3 generate_manpage.py``

Synopsis
========

| **radosgw-admin** *command* [ *options* *...* ]

Description
===========

Ceph Object Gateway user administration utility.
See: https://docs.ceph.com/en/latest/radosgw/admin/

Commands
========

:program:`radosgw-admin` provides the following commands.
For per-command option details, see the `Per-Command Options`_ section below.


Radosgw-Admin Commands
----------------------

:command:`radosgw-admin user create`
  Create a new user

:command:`radosgw-admin user info`
  Display information for a user including subusers and keys

:command:`radosgw-admin user modify`
  Modify an existing user

:command:`radosgw-admin user rm`
  Remove a user and all associated subusers

:command:`radosgw-admin user list`
  List all users

:command:`radosgw-admin user suspend`
  Suspend a user, disabling their access

:command:`radosgw-admin user enable`
  Re-enable a previously suspended user

:command:`radosgw-admin user stats`
  Show user stats as accounted by the quota subsystem

:command:`radosgw-admin bucket list`
  List buckets, or objects within a bucket if --bucket is specified

:command:`radosgw-admin bucket stats`
  Return statistics and internal information about a bucket

:command:`radosgw-admin bucket rm`
  Remove a bucket

:command:`radosgw-admin bucket link`
  Link a bucket to a specified user, changing ownership

:command:`radosgw-admin bucket unlink`
  Unlink a bucket from a specified user

:command:`radosgw-admin bucket check`
  Check bucket index by verifying size and object count stats

:command:`radosgw-admin bucket logging flush`
  Flush pending log records to the log bucket immediately

:command:`radosgw-admin bucket logging info`
  Get logging configuration for a source bucket, or list sources logging to a specific log bucket

:command:`radosgw-admin bucket logging list`
  List the log objects pending commit for a source bucket

:command:`radosgw-admin quota set`
  Set quota parameters for a user, bucket, or account

:command:`radosgw-admin quota enable`
  Enable quota enforcement for a user or bucket

:command:`radosgw-admin quota disable`
  Disable quota enforcement for a user or bucket

:command:`radosgw-admin ratelimit set`
  Controls maximum request/byte rates per accumulation interval per RGW

:command:`radosgw-admin ratelimit get`
  Get current rate limit parameters for a user or bucket

:command:`radosgw-admin ratelimit enable`
  Enable rate limiting for a user or bucket

:command:`radosgw-admin ratelimit disable`
  Disable rate limiting for a user or bucket

Options
=======

The following options are available globally and can be used with any command.

.. option:: -c ceph.conf, --conf=ceph.conf

   Use ``ceph.conf`` instead of the default ``/etc/ceph/ceph.conf``.

.. option:: --cluster=<name>

   Set the cluster name (default: ceph).

.. option:: --id=<id>

   Set the ID portion of the client name.

.. option:: --display-name

   User display name


.. option:: --quota-scope

   Scope of the quota: bucket, user, or account


.. option:: --ratelimit-scope

   Scope of the rate limit: bucket, user, or anonymous


.. option:: --uid

   The user ID


.. option:: --admin

   Set the admin flag on the user


.. option:: --allow-unordered

   Faster unsorted listing, removes ordering requirement


.. option:: --bucket

   The bucket name


.. option:: --bucket-id

   The bucket ID


.. option:: --email

   The email address of the user


.. option:: --fix

   Fix the bucket index in addition to checking it


.. option:: --format

   Output format for the response: json, xml


.. option:: --max-buckets

   Maximum number of buckets for the user (server default: 1000 if not
   specified)


.. option:: --max-objects

   Maximum number of objects (negative value to disable)


.. option:: --max-read-bytes

   Max read bytes per interval (0 = unlimited)


.. option:: --max-read-ops

   Max read requests per interval (0 = unlimited)


.. option:: --max-size

   Maximum storage size in B/K/M/G/T (negative value to disable)


.. option:: --max-write-bytes

   Max write bytes per interval (0 = unlimited)


.. option:: --max-write-ops

   Max write requests per interval (0 = unlimited)


.. option:: --purge-data

   Also purge all data associated with the user


.. option:: --purge-objects

   Remove all objects before deleting (required for non-empty buckets)


.. option:: --sync-stats

   Update user stats with current stats from bucket indexes


.. option:: --tenant

   Tenant name


Per-Command Options
===================

The following details the specific options accepted by each command.
Options marked *(required)* must be provided or the command will error.


Radosgw-Admin Commands
----------------------

``radosgw-admin user create``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Create a new user

.. option:: --uid *(required)*

   The user ID


.. option:: --display-name *(required)*

   User display name


.. option:: --email

   The email address of the user


.. option:: --admin

   Set the admin flag on the user


.. option:: --max-buckets

   Maximum number of buckets for the user (server default: 1000 if not
   specified)


.. option:: --format

   Output format for the response: json, xml


.. option:: --tenant

   Tenant name


``radosgw-admin user info``
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Display information for a user including subusers and keys

.. option:: --uid *(required)*

   The user ID


.. option:: --display-name

   User display name


.. option:: --format

   Output format for the response: json, xml


.. option:: --tenant

   Tenant name


``radosgw-admin user modify``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Modify an existing user

.. option:: --uid *(required)*

   The user ID


.. option:: --display-name

   User display name


.. option:: --email

   New email address for the user


.. option:: --admin

   Set or clear the admin flag on the user


.. option:: --format

   Output format for the response: json, xml


.. option:: --tenant

   Tenant name


``radosgw-admin user rm``
~~~~~~~~~~~~~~~~~~~~~~~~~

Remove a user and all associated subusers

.. option:: --uid *(required)*

   The user ID


.. option:: --display-name

   User display name


.. option:: --tenant

   Tenant name


.. option:: --purge-data

   Also purge all data associated with the user


``radosgw-admin user list``
~~~~~~~~~~~~~~~~~~~~~~~~~~~

List all users

.. option:: --format

   Output format for the response: json, xml


``radosgw-admin user suspend``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Suspend a user, disabling their access

.. option:: --uid *(required)*

   The user ID


.. option:: --display-name

   User display name


``radosgw-admin user enable``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Re-enable a previously suspended user

.. option:: --uid *(required)*

   The user ID


.. option:: --display-name

   User display name


``radosgw-admin user stats``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Show user stats as accounted by the quota subsystem

.. option:: --uid *(required)*

   The user ID


.. option:: --display-name

   User display name


.. option:: --sync-stats

   Update user stats with current stats from bucket indexes


``radosgw-admin bucket list``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

List buckets, or objects within a bucket if --bucket is specified

.. option:: --bucket

   The bucket name


.. option:: --bucket-id

   The bucket ID


.. option:: --allow-unordered

   Faster unsorted listing, removes ordering requirement


.. option:: --format

   Output format for the response: json, xml


.. option:: --tenant

   Tenant name


``radosgw-admin bucket stats``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Return statistics and internal information about a bucket

.. option:: --bucket *(required)*

   The bucket name


.. option:: --bucket-id

   The bucket ID


.. option:: --format

   Output format for the response: json, xml


.. option:: --tenant

   Tenant name


``radosgw-admin bucket rm``
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Remove a bucket

.. option:: --bucket *(required)*

   The bucket name


.. option:: --bucket-id

   The bucket ID


.. option:: --purge-objects

   Remove all objects before deleting (required for non-empty buckets)


``radosgw-admin bucket link``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Link a bucket to a specified user, changing ownership

.. option:: --bucket *(required)*

   The bucket name


.. option:: --uid *(required)*

   The user ID


.. option:: --bucket-id

   The bucket ID


.. option:: --display-name

   User display name


.. option:: --tenant

   Tenant name


``radosgw-admin bucket unlink``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Unlink a bucket from a specified user

.. option:: --bucket *(required)*

   The bucket name


.. option:: --uid *(required)*

   The user ID


.. option:: --bucket-id

   The bucket ID


.. option:: --display-name

   User display name


``radosgw-admin bucket check``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Check bucket index by verifying size and object count stats

.. option:: --bucket

   The bucket name


.. option:: --bucket-id

   The bucket ID


.. option:: --fix

   Fix the bucket index in addition to checking it


``radosgw-admin bucket logging flush``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Flush pending log records to the log bucket immediately

.. option:: --bucket *(required)*

   The source bucket to flush logs for


``radosgw-admin bucket logging info``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Get logging configuration for a source bucket, or list sources logging to a specific log bucket

.. option:: --bucket

   The source bucket to get logging info for


``radosgw-admin bucket logging list``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

List the log objects pending commit for a source bucket

.. option:: --bucket *(required)*

   The source bucket


``radosgw-admin quota set``
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Set quota parameters for a user, bucket, or account

.. option:: --quota-scope *(required)*

   Scope of the quota: bucket, user, or account


.. option:: --uid

   The user ID


.. option:: --display-name

   User display name


.. option:: --bucket

   The bucket name


.. option:: --bucket-id

   The bucket ID


.. option:: --max-objects

   Maximum number of objects (negative value to disable)


.. option:: --max-size

   Maximum storage size in B/K/M/G/T (negative value to disable)


.. option:: --format

   Output format for the response: json, xml


``radosgw-admin quota enable``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Enable quota enforcement for a user or bucket

.. option:: --quota-scope *(required)*

   Scope: bucket or user


.. option:: --uid

   The user ID


.. option:: --display-name

   User display name


``radosgw-admin quota disable``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Disable quota enforcement for a user or bucket

.. option:: --quota-scope *(required)*

   Scope: bucket or user


.. option:: --uid

   The user ID


.. option:: --display-name

   User display name


``radosgw-admin ratelimit set``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Controls maximum request/byte rates per accumulation interval per RGW

.. option:: --max-read-ops

   Max read requests per interval (0 = unlimited)


.. option:: --max-write-ops

   Max write requests per interval (0 = unlimited)


.. option:: --max-read-bytes

   Max read bytes per interval (0 = unlimited)


.. option:: --max-write-bytes

   Max write bytes per interval (0 = unlimited)


``radosgw-admin ratelimit get``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Get current rate limit parameters for a user or bucket

.. option:: --ratelimit-scope *(required)*

   Scope: bucket, user, or anonymous


.. option:: --uid

   The user ID


.. option:: --display-name

   User display name


.. option:: --bucket

   The bucket name


.. option:: --bucket-id

   The bucket ID


.. option:: --format

   Output format for the response: json, xml


``radosgw-admin ratelimit enable``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Enable rate limiting for a user or bucket

.. option:: --ratelimit-scope *(required)*

   Scope: bucket, user, or anonymous


.. option:: --uid

   The user ID


.. option:: --display-name

   User display name


``radosgw-admin ratelimit disable``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Disable rate limiting for a user or bucket

.. option:: --ratelimit-scope *(required)*

   Scope: bucket, user, or anonymous


.. option:: --uid

   The user ID


.. option:: --display-name

   User display name


Availability
============

:program:`radosgw-admin` is part of Ceph, a massively scalable, open-source, distributed storage system. Please refer to the Ceph documentation at https://docs.ceph.com for more information.

See Also
========

:doc:`ceph <ceph>`\(8)
:doc:`radosgw <radosgw>`\(8)

