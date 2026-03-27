# radosgw-admin CLI11 Proof of Concept

Demonstrates how CLI11 solves the documentation and UX problems in
https://tracker.ceph.com/issues/74508

## Files

| File | Purpose |
|---|---|
| `radosgw_admin_poc.cpp` | C++ PoC — replace with real `radosgw-admin.cc` |
| `generate_manpage.py` | Python generator — reads JSON, writes man page RST |
| `generate_adminguide.py` | Python generator — reads JSON, writes admin guide RST |
| `CLI11.hpp` | Download from https://github.com/CLIUtils/CLI11/releases/latest |

## Setup and compile

```bash
# 1. Download CLI11 (single header, no other dependencies)
wget https://github.com/CLIUtils/CLI11/releases/download/v2.4.2/CLI11.hpp

# 2. Compile (C++17 required — for std::optional)
g++ -std=c++17 radosgw_admin_poc.cpp -o radosgw_admin_poc
```

## Usage notes (applies to all commands)

- Options accept values as `--uid=johndoe` OR `--uid johndoe` (both work)
- Values containing spaces must be quoted: `--display-name="John Doe"`
- Both `-h` and `--help` show help for the current command level
- `--help-all` shows the full expanded tree from the current level down
- To avoid `-h` being consumed as a flag value, use `--flag=value` syntax
  (e.g. `--bucket=mybucket --help`, not `--bucket --help`)

---

## Demo 1 — Auto-generated global help
### Problem solved: hand-written help string that drifted from code

```
$ ./radosgw_admin_poc --help

Ceph Object Gateway user administration utility.
See: https://docs.ceph.com/en/latest/radosgw/admin/
Usage: radosgw-admin [OPTIONS] [SUBCOMMAND]

Options:
  -h,--help                               Print this help message and exit
  --export-tree                           Export the full command tree as JSON (for doc generation)
  -c,--conf TEXT                          Path to ceph.conf (default: /etc/ceph/ceph.conf)
  --cluster TEXT                          Cluster name (default: ceph)
  --id TEXT                               ID portion of the client name

Subcommands:
  user                                    User management operations
  bucket                                  Bucket management operations
  quota                                   Quota management for users and buckets
  ratelimit                               Rate limit management for users and buckets
```

Note: `-c,--conf`, `--cluster`, `--id` are TRUE GLOBAL OPTIONS — registered
once on the root app, automatically available to every subcommand. This
demonstrates CLI11's clean handling of global options (boost::program_options
cannot do this natively with nested subcommands).

**Note on "Subcommand" label:** CLI11 uses the word "Subcommand" for all
command groups because from its perspective everything below the root is a
subcommand. This label can be customized with a formatter override if "Commands"
is preferred. For the PoC this is cosmetic and not changed.

---

## Demo 2 — Per-command help
### Problem solved: `radosgw-admin user --help` used to show ALL 200 commands

```
$ ./radosgw_admin_poc user --help

User management operations
Usage: radosgw-admin user [OPTIONS] SUBCOMMAND

Options:
  -h,--help                               Print this help message and exit
  --help-all                              Expand all help

Subcommands:
  create                                  Create a new user
  info                                    Display information for a user including subusers and keys
  modify                                  Modify an existing user
  rm                                      Remove a user and all associated subusers
  list                                    List all users
  suspend                                 Suspend a user, disabling their access
  enable                                  Re-enable a previously suspended user
  stats                                   Show user stats as accounted by the quota subsystem
```

Use `--help-all` to expand the full tree from this level:

```
$ ./radosgw_admin_poc user --help-all

User management operations
Usage: radosgw-admin user [OPTIONS] SUBCOMMAND

Options:
  -h,--help                               Print this help message and exit
  --help-all                              Expand all help

Subcommands:
  create  Create a new user
    Options:
      --uid TEXT REQUIRED                 The user ID
      --display-name TEXT REQUIRED        User display name
      --email TEXT                        The email address of the user
      --admin                             Set the admin flag on the user
      --max-buckets INT                   Maximum number of buckets for the user
      --format TEXT                       Output format for the response: json, xml
      --tenant TEXT                       Tenant name
  info  Display information for a user including subusers and keys
    Options:
      --uid TEXT REQUIRED                 The user ID
      --display-name TEXT                 User display name
      --format TEXT                       Output format for the response: json, xml
      --tenant TEXT                       Tenant name
  ... (all other user subcommands expanded)
```

This shows every subcommand AND all their flags in one view.

Note on `[OPTIONS] SUBCOMMAND` vs `[OPTIONS] [SUBCOMMAND]`:
SUBCOMMAND without brackets means it is required at this level.
This is correct — you must specify a user verb. The brackets appear
only where a subcommand is genuinely optional.

**Known CLI11 cosmetic issue:** CLI11 always shows `[OPTIONS]` in the usage
line even when some options are required. Required status is shown per-option
with `REQUIRED` in the options list below. This is a CLI11 design convention.

---

## Demo 3 — Per-leaf-command help with only relevant options
### Problem solved: options scoped precisely per command, not listed globally

```
$ ./radosgw_admin_poc user create --help

Create a new user
Usage: radosgw-admin user create [OPTIONS]

Options:
  -h,--help                               Print this help message and exit
  --uid TEXT REQUIRED                     The user ID
  --display-name TEXT REQUIRED            User display name
  --email TEXT                            The email address of the user
  --admin                                 Set the admin flag on the user
  --max-buckets INT                       Maximum number of buckets for the user
  --format TEXT                           Output format for the response: json, xml
  --tenant TEXT                           Tenant name
```

Note: `--format` and `--tenant` appear here because they are registered
via `register_format_flag()` and `register_tenant_flag()` — the explicit
list of commands that support them. They do NOT appear in commands that
don't support them (e.g. `user suspend --help` will not show `--format`).

---

## Demo 4 — Hierarchical error for incomplete commands (3 levels deep)
### Problem solved: incomplete commands gave cryptic errors

```
$ ./radosgw_admin_poc bucket logging

A subcommand is required
Run with --help for more information.
```

```
$ ./radosgw_admin_poc bucket logging --help

Bucket logging operations
Usage: radosgw-admin bucket logging [OPTIONS] SUBCOMMAND

Options:
  -h,--help                               Print this help message and exit
  --help-all                              Expand all help

Subcommands:
  flush       Flush pending log records to the log bucket immediately
  info        Get logging configuration for a source bucket, ...
  list        List the log objects pending commit for a source bucket
```

`require_subcommand(1)` on the `logging` node is the single line of code
that produces this behavior automatically — no custom error handling needed.

---

## Demo 5 — Required argument enforcement with automatic errors
### Problem solved: missing required args caused silent failures deep in RGW

```
$ ./radosgw_admin_poc user create
[error] --uid is required
Run with --help for more information.

$ ./radosgw_admin_poc user create --uid=johndoe
[error] --display-name is required
Run with --help for more information.

$ ./radosgw_admin_poc user create --uid=johndoe --display-name="John Doe"
[user create] uid=johndoe display-name="John Doe"

$ ./radosgw_admin_poc user info --uid=johndoe
[user info] uid=johndoe

$ ./radosgw_admin_poc user list
[user list]
```

Note: `--uid` is required for `user create` and `user info` but NOT for
`user list`. The requirement is per-command, not global. Each command
declares exactly what it needs.

---

## Demo 5b — Validators: preventing flag-like values and duplicate flags
### Problem solved: `--uid -h` accidentally setting uid to "-h"; duplicate flags hiding bugs

```
# Flag-like value rejected
$ ./radosgw_admin_poc user create --uid=-h --display-name="Test"
[error] Value '-h' looks like a flag. Use --option=value syntax.

# Duplicate flag rejected
$ ./radosgw_admin_poc user create --uid=alice --uid=bob --display-name="Test"
[error] --uid: option specified more than once but multi_option_policy is Throw

# Correct usage
$ ./radosgw_admin_poc user create --uid=johndoe --display-name="John Doe"
[user create] uid=johndoe display-name="John Doe"
```

Two validation patterns applied:

**`no_flag_value` validator** — rejects string values starting with `-`.
Applied to `--uid`, `--display-name`, `--bucket`, `--bucket-id`.
Any value starting with `-` is rejected entirely — it is not accepted even if
intended. This is intentional: values that look like flags are almost always
mistakes. Use `--option=value` syntax (e.g. `--uid=johndoe`) to be safe.
In real Ceph, `--uid -h` would silently set uid to "-h" and fail later
with a confusing RGW error — this catches it immediately.

**`multi_option_policy(Throw)`** — rejects duplicate flags for key options.
Without this, CLI11's default is "last value wins": `--uid=alice --uid=bob`
silently uses "bob". With Throw, this is an explicit error. Applied to `--uid`
and `--display-name` as examples.

---

## Demo 6 — Option groups (nested flags)
### Problem solved: related options appear as labeled sections in --help

```
$ ./radosgw_admin_poc ratelimit set --help

Set rate limit parameters for a user or bucket
Usage: radosgw-admin ratelimit set [OPTIONS]

Options:
  -h,--help                               Print this help message and exit
  --uid TEXT                              The user ID
  --display-name TEXT                     User display name
  --bucket TEXT                           The bucket name
  --bucket-id TEXT                        The bucket ID
  --ratelimit-scope TEXT REQUIRED         Scope of the rate limit: bucket, user, or anonymous

Rate limiting options - Controls maximum request/byte rates per accumulation interval per RGW:
  --max-read-ops INT                      Max read requests per interval (0 = unlimited)
  --max-write-ops INT                     Max write requests per interval (0 = unlimited)
  --max-read-bytes INT                    Max read bytes per interval (0 = unlimited)
  --max-write-bytes INT                   Max write bytes per interval (0 = unlimited)
```

The "Rate limiting options" group is defined with `add_option_group()` in C++.
Related flags appear together under a label — this is CLI11's option group feature.
Note: the label "Rate limiting options" appears in the output as a section header.
This is expected and correct behavior — it is not a bug.

---

## Demo 7 — True global options with Ceph-compatible placement
### Shows: --conf, --cluster, --id work with any command, before OR after the verb

```
# Global option BEFORE subcommand (standard CLI11 behavior)
$ ./radosgw_admin_poc --cluster=ceph user create --uid=johndoe --display-name="John Doe"
[user create] uid=johndoe display-name="John Doe"

# Global option AFTER subcommand (Ceph-style, enabled via fallthrough())
$ ./radosgw_admin_poc user create --uid=johndoe --display-name="John Doe" --cluster=ceph
[user create] uid=johndoe display-name="John Doe"

# Both still work
$ ./radosgw_admin_poc --conf=/etc/ceph/ceph.conf bucket list
[bucket list] all buckets
```

Global options are registered once on the root app — never re-registered per command.
`set_fallthrough_all()` recursively enables `fallthrough()` on EVERY node in the tree.
This is critical — without it, `--conf` in `bucket logging flush --conf=x` would fail
because `flush` and `logging` have no fallthrough, so the option never reaches root.
`fallthrough()` on each top-level subcommand allows them to appear after the verb,
matching real Ceph behavior. Unknown options (typos) still produce errors — validation
is fully preserved.

---

## Demo 7b — Global options at any nesting depth (fallthrough at all levels)
### Matches real Ceph behavior: global options work before or after subcommands at any depth

```
# Works: global option before everything
$ ./radosgw_admin_poc --conf=/etc/ceph/ceph.conf bucket logging flush --bucket=b
[bucket logging flush] bucket=b

# Works: global option after 3-level subcommand (bucket logging flush)
$ ./radosgw_admin_poc bucket logging flush --bucket=b --conf=/etc/ceph/ceph.conf
[bucket logging flush] bucket=b

# Without set_fallthrough_all(), the second form would fail with:
# [error] --conf is not a recognized option at this level
```

This works because `set_fallthrough_all()` recursively sets `fallthrough()` on
EVERY node in the tree — not just top-level subcommands. The option travels:
`flush → logging → bucket → root app` where `--conf` is registered.

Without recursive fallthrough, options at leaf level (flush, info, list) would
not pass up to root, breaking the global option behavior for 3-level commands.

---

## Demo 8 — JSON tree export for doc generation
### Problem solved: man page and admin guide drifted from code

Save the tree to a file for inspection:

```bash
$ ./radosgw_admin_poc --export-tree > commands.json
```

The `commands.json` file contains the complete command tree with names,
descriptions, options, required flags, and types — everything needed to
generate both the man page and the admin guide automatically.

Excerpt of `commands.json`:
```json
[
  {
    "name": "radosgw-admin",
    "description": "Ceph Object Gateway user administration utility...",
    "options": [
      {
        "name": "-c,--conf",
        "description": "Path to ceph.conf (default: /etc/ceph/ceph.conf)",
        "required": false,
        "type": "TEXT"
      }
    ],
    "subcommands": [
      {
        "name": "user",
        "description": "User management operations",
        "options": [],
        "subcommands": [
          {
            "name": "create",
            "description": "Create a new user",
            "options": [
              {
                "name": "--uid",
                "description": "The user ID",
                "required": true,
                "type": "TEXT"
              },
              {
                "name": "--display-name",
                "description": "User display name",
                "required": true,
                "type": "TEXT"
              }
            ],
            "subcommands": []
          }
        ]
      }
    ]
  }
]
```

---

## Demo 9 — Full man page pipeline

Save the generated man page to a file:

```bash
$ ./radosgw_admin_poc --export-tree | python3 generate_manpage.py > radosgw-admin.rst
```

The generated `radosgw-admin.rst` has the same structure as the current
`doc/man/8/radosgw-admin.rst` in the Ceph repo:

- Synopsis section
- Description section
- Commands section — grouped by family (User Commands, Bucket Commands, etc.)
- Options section — global options + all unique options (required first, then alphabetical)
- Per-Command Options section — each command's specific options with family group headers
- Availability section
- See Also section

**Ordering consistency:** Per-command options in the man page use the same registration
order as the C++ code, which is the same order shown in `--help` output. This means
`--help` and the man page always show options in the same order — no confusion when
switching between them. The global Options section uses alphabetical order for easy
scanning of the full list.

---

## Demo 10 — Admin guide pipeline

### Marker-based approach (verified working)

Human narrative is preserved between invisible RST comment markers.
Safe to regenerate at any time — narrative is never overwritten.

**How preservation works — verified with automated test, all checks pass:**

1. Before regenerating, the script reads the existing `admin.rst`
2. It extracts all content between every `NARRATIVE-BEGIN/END` marker pair
   using a regex, storing it in a **1:1 dictionary keyed by command path**
3. It regenerates the entire skeleton from the JSON tree
4. It reinserts each narrative by looking up its unique key in the dictionary
5. New commands get empty markers — developers fill them in

**Why the 1:1 dictionary guarantees correctness:**
Every command has a unique path string as its key — for example `user-create`,
`bucket-list`, `bucket-logging-flush`. No two commands share a key. When
reinserting, the generator calls `narratives.get("user-create")` and puts
exactly that content back between the `user-create` markers. It is structurally
impossible for narrative content to be assigned to the wrong command or lost.

Result: old narratives are never deleted. New commands get empty markers.
The order is stable because the tree always walks in the same command order.

```bash
# First run: generates skeleton with empty markers
$ ./radosgw_admin_poc --export-tree | python3 generate_adminguide.py > admin.rst

# Developer edits admin.rst, filling in narrative between markers:
#   .. # NARRATIVE-BEGIN: user-create
#   To create a user, you also get an access_key and secret_key automatically...
#   .. # NARRATIVE-END: user-create

# Later: new command added to C++, regenerate preserving existing narrative
$ ./radosgw_admin_poc --export-tree | python3 generate_adminguide.py \
    --existing=admin.rst > admin_new.rst
$ mv admin_new.rst admin.rst
```

The markers are RST comments — completely invisible to readers in HTML/PDF output.

**Why the marker approach:** The goal — C++ as single source of truth, docs auto-generated
from it — is the same as Ceph's existing `confval`/YAML pattern for config options.
The marker approach achieves this without a custom Sphinx extension: one file, no YAML,
just Python. A 1:1 dictionary keyed by command path guarantees narrative is never lost.

---

## How to add a new command (complete workflow)

This is the key improvement: a developer adds a command once and everything updates.

### Step 1 — Add the C++ definition

In `radosgw_admin_poc.cpp` (or `radosgw-admin.cc` in real Ceph):

```cpp
// Inside the relevant command family, e.g. under 'user':
auto user_rename = user->add_subcommand("rename",
    "Rename a user to a new user ID");
add_user_identity_opts(user_rename, user_opts);
user_rename->get_option("--uid")->required();
std::string new_uid;
user_rename->add_option("--new-uid", new_uid,
    "The new user ID")->required();
```

### Step 2 — Recompile

```bash
g++ -std=c++17 radosgw_admin_poc.cpp -o radosgw_admin_poc
```

### Step 3 — Verify --help updated automatically

```bash
./radosgw_admin_poc user --help
# 'rename' now appears in the list automatically

./radosgw_admin_poc user rename --help
# Shows --uid (required) and --new-uid (required) automatically
```

### Step 4 — Regenerate man page

```bash
./radosgw_admin_poc --export-tree | python3 generate_manpage.py > radosgw-admin.rst
```

The man page now includes `user rename` with its options. No manual editing.

### Step 5 — Regenerate admin guide

```bash
# Existing narrative is preserved automatically
./radosgw_admin_poc --export-tree | python3 generate_adminguide.py \
    --existing=admin.rst > admin_new.rst && mv admin_new.rst admin.rst
```

A new section for `user rename` appears with accurate command syntax and options.
The developer optionally adds narrative prose for the new command.

### Step 6 — If adding to a cross-cutting flag list

If `user rename` should support `--format`, add it to the registration call:

```cpp
register_format_flag({
    user_create, user_info, user_modify, user_list,
    user_rename,   // ← add here
    bucket_list, bucket_stats, quota_set, ratelimit_get
}, global_opts.format);
```

That's all. `--format` now appears in `user rename --help` automatically.

---

## CI enforcement — making drift structurally impossible

The final piece of the system is a CI check that runs on every pull request.
It prevents the man page from ever going out of sync again.

The check is a shell script added to Ceph's CI configuration:

```bash
#!/bin/bash
# ci_check_manpage.sh — runs in CI on every PR that touches radosgw-admin.cc

# Build the binary
ninja -C build radosgw-admin

# Generate the man page from the binary
./build/bin/radosgw-admin --export-tree | \
    python3 doc/scripts/generate_manpage.py > /tmp/generated_manpage.rst

# Compare against the committed version
if ! diff -u doc/man/8/radosgw-admin.rst /tmp/generated_manpage.rst; then
    echo "ERROR: radosgw-admin.rst is out of sync with the binary."
    echo "Run: radosgw-admin --export-tree | python3 generate_manpage.py > doc/man/8/radosgw-admin.rst"
    exit 1
fi
echo "Man page is in sync."
```

**What this enforces:**

- **Recompile forgotten:** The CI script runs `ninja -C build radosgw-admin` as its
  very first step. It always rebuilds the binary from source before doing anything else.
  So even if a developer forgot to recompile locally, CI does it correctly. The check
  is always against a freshly built binary, never a stale one.
- If a developer adds a command in C++ but forgets to regenerate the man page, CI fails
- The PR cannot merge until the man page is regenerated
- The failure message tells the developer exactly what command to run
- No reviewer needs to manually check documentation completeness

**Where this goes in Ceph:**
The script is added to `.github/workflows/` or the `qa/` test directory.
This is standard practice in the Ceph project and requires no special permissions.

---

## Robustness improvements over legacy Ceph CLI

These features make the new CLI more robust than the existing radosgw-admin:

### 1. `fallthrough()` at all tree depths
Global options (`--conf`, `--cluster`, `--id`) work at any nesting level.
`set_fallthrough_all()` recursively applies this to every node.
Old Ceph: permissive but inconsistent. New: explicitly correct at all depths.

### 2. `no_flag_value` validator on string options
Prevents accidental flag-as-value errors on `--uid`, `--display-name`,
`--bucket`, `--bucket-id`. In old Ceph, `--uid -h` silently set uid to "-h"
and failed later with a confusing error. Now it fails immediately with a clear
message telling the user to use `--uid=value` syntax.

### 3. `multi_option_policy(Throw)` on key options
Duplicate flags like `--uid=alice --uid=bob` now cause an explicit error.
Old CLI11 default (and Ceph): last value wins silently. This hides bugs.

### 4. `std::optional<int>` for optional numeric options
Distinguishes "user did not pass `--max-buckets`" from "user explicitly passed 1000".
This matches real Ceph API behavior: not passing a value means use the server default,
not a locally defined fallback. `max_buckets.has_value()` is false when the flag
was not provided by the user — this is the correct signal to pass to the RGW layer.

---

## Intentional differences from Ceph (improvements, not bugs)

These behaviors differ from legacy Ceph but are intentional design decisions:

| Behavior | Legacy Ceph | This PoC | Reason |
|---|---|---|---|
| `user cr` (partial match) | Sometimes works | ❌ Error | Ambiguous, not a defined API |
| `create user` (reversed order) | Sometimes works | ❌ Error | CLI must be deterministic |
| `--uid=a --uid=b` (duplicates) | Last value wins silently | ❌ Error | Hides bugs |
| `--uid -x` (flag as value) | Silent wrong behavior | ❌ Error with clear message | Immediate feedback |
| `--cluster` after verb | ⚠️ Sometimes works | ✅ Always works | Explicitly supported via fallthrough |

**Design principle:** Be compatible with Ceph where behavior is intentional and
relied upon, but prefer strict, predictable, and validated parsing otherwise.
The PoC is in many ways more correct than legacy Ceph.

---

## Architecture summary

```
radosgw-admin.cc (CLI11 definitions)       ← SINGLE SOURCE OF TRUTH
        |
        | g++ → binary
        |
        ├── --help                          → terminal help (auto-generated)
        ├── user --help                     → per-command help (auto-generated)
        ├── bucket logging                  → hierarchical error (automatic)
        ├── user create (missing --uid)     → required arg error (automatic)
        └── --export-tree
                |
                | commands.json
                |
                ├── generate_manpage.py     → doc/man/8/radosgw-admin.rst
                └── generate_adminguide.py  → doc/radosgw/admin.rst
                        |
                        | Sphinx
                        |
                        ├── docs.ceph.com (HTML)
                        └── man radosgw-admin (man page)

CI check: regenerate → diff → fail if different → PR blocked until fixed
```

**When a developer adds a new command:**
1. Write the CLI11 definition in C++ with a description string
2. Recompile
3. `--help` updates automatically — nothing else to do for terminal help
4. Run `--export-tree | generate_manpage.py` — man page updates automatically
5. Run `--export-tree | generate_adminguide.py` — admin guide skeleton updates
6. Optionally write narrative prose for the new command
7. CI enforces step 4 was done before the PR merges

**No separate documentation file needs to be manually maintained. Ever.**
