# RadosGW-admin: Command and Parameter Documentation Audit

## Summary

* ~220 commands identified across all sources

* 218 commands appear in `--help`

* Only 30 commands (~14%) are documented in `admin.rst`

* A significant portion of commands is not documented in the man page

* ~160 parameters identified across all source

* 157 parameters appear in `--help`

* Only 32 parameters (~20%) are documented in `admin.rst`

* A significant portion of parameters is not documented in the man page

    

* Multiple inconsistencies exist between `--help`, the man page, and the source code

---

## General Findings

### Completely undocumented parameter

Exists in the source code but missing from both `--help` and the man page:

* `--group-id`

### Duplicate flags in `--help`

The following flags appear twice:

* `--max-entries`
* `--op-mask`

---

## Comparing `--help` to the Man Page

### Commands in `--help` but missing from the man page (68)

These commands appear in `--help` and in the source code but are not documented in the man page (`doc/man/8/radosgw-admin.rst`):

* user policy attach
* user policy detach
* user policy list attached
* dedup stats
* dedup estimate
* dedup exec
* dedup abort
* dedup pause
* dedup resume
* dedup throttle
* account create
* account modify
* account get
* account stats
* account rm
* account list
* bucket check olh
* bucket check unlinked
* bucket set-min-shards
* bucket sync checkpoint
* object put
* objects expire-stale list
* objects expire-stale rm
* ratelimit get
* ratelimit set
* ratelimit enable
* ratelimit disable
* global ratelimit get
* global ratelimit set
* global ratelimit enable
* global ratelimit disable
* realm default rm
* zonegroup placement get
* zone placement get
* lc reshard fix
* mdlog autotrim
* bilog status
* bilog autotrim
* datalog type
* datalog semaphore list
* datalog semaphore reset
* reshardlog list
* reshardlog purge
* reshard stale-instances list
* reshard stale-instances delete
* role-trust-policy modify
* role-policy delete *(“role-policy rm” in man page)*
* role policy attach
* role policy detach
* role policy list attached
* role update
* mfa create
* mfa list
* mfa get
* mfa remove
* mfa check
* mfa resync
* script put
* script get
* script rm
* script-package add
* script-package rm
* script-package list
* script-package reload
* notification list
* notification get
* notification rm
* usage clear

---

### Commands in the man page but missing from `--help`

* object manifest *(exists in source code)*
* role modify *(likely renamed to **`role-trust-policy modify`**)*

---

### Parameters in `--help` but missing from the man page (46)

These parameters exist in the source code and in `--help`, but are not documented in the man page:

* `--user_ns`
* `--account-name`
* `--account-id`
* `--max-users`
* `--max-roles`
* `--max-groups`
* `--max-access-keys`
* `--key-active`
* `--op-mask`
* `--objects-file`
* `--object-version`
* `--count`
* `--gen`
* `--reset-stats`
* `--redirect-zone`
* `--storage-class`
* `--totp-serial`, `--totp-seed`, `--totp-seconds`, `--totp-window`, `--totp-pin`
* `--description`, `--policy-arn`
* `--notification-id`
* `--max-read-ops`, `--max-read-bytes`, `--max-write-ops`, `--max-write-bytes`, `--max-list-ops`, `--max-delete-ops`, `--ratelimit-scope`
* `--detail`
* `--context`, `--package`, `--allow-compilation`
* `--min-age-hours`, `--dump-keys`, `--hide-progress`
* `--rgw-obj-fs`
* `--trim-delay-ms`
* `--enable-feature`, `--disable-feature`
* `--max-bucket-index-ops`, `--max-metadata-ops`, `--stat`
* `--marker` *(unclear usage)*
* `--group-id`

---

### Parameters in the man page but missing from `--help`

* `--subscription`
* `--event-id`

---

## Comparing `--help` to admin.rst

### Commands

All commands documented in `admin.rst` appear in `--help`.

Only 30 commands from `--help` are documented in admin.rst (about 14% of all commands). Approximately 188 commands (about 86%) are missing from the admin guide.

👉 This means only ~14% of commands are documented in the admin guide.

---

### Commands present in admin.rst (30)

* user create
* user modify
* user info
* user rm
* user suspend
* user enable
* user stats
* caps add
* caps rm
* subuser create
* subuser modify
* subuser rm
* key create
* key rm
* period update
* quota set
* quota enable
* quota disable
* ratelimit get
* ratelimit set
* ratelimit enable
* ratelimit disable
* global quota get
* global quota set
* global quota enable
* global ratelimit get
* global ratelimit set
* global ratelimit enable
* usage show
* usage trim

---

### Parameters

All parameters documented in `admin.rst` appear in `--help`.

Only 32 parameters from `--help` are documented. 125 parameters are missing.

👉 This means ~80% of parameters are undocumented in the admin guide.

---

### Parameters present in admin.rst (32)

* `--access`
* `--access-key`
* `--admin`
* `--bucket`
* `--caps`
* `--commit`
* `--display-name`
* `--email`
* `--end-date`
* `--gen-access-key`
* `--gen-secret`
* `--generate-key`
* `--max-read-ops`
* `--max-read-bytes`
* `--max-write-ops`
* `--max-write-bytes`
* `--quota-scope`
* `--max-size`
* `--max-objects`
* `--enabled`
* `--secret-key`
* `--subuser`
* `--uid`
* `--start-date`
* `--temp-url-key`
* `--temp-url-key2`
* `--system`
* `--suspended`
* `--purge-data`
* `--purge-keys`
* `--purge-objects`
* `--format`

---

## Notes

* Counts are based on `--help` output, with additional validation from the source code

---

## Conclusion

There is a significant documentation gap between the CLI (`--help`), the man page, and the admin guide. Most commands and parameters are not documented. Improving documentation coverage and consistency would significantly enhance usability and maintainability.
