#!/usr/bin/env python3
"""
generate_manpage.py
===================
Reads the JSON command tree exported by:

    ./radosgw_admin_poc --export-tree | python3 generate_manpage.py

Or to save to a file:

    ./radosgw_admin_poc --export-tree > commands.json
    ./radosgw_admin_poc --export-tree | python3 generate_manpage.py > radosgw-admin.rst

Produces a radosgw-admin.rst man page in Ceph's RST format,
matching the structure of doc/man/8/radosgw-admin.rst.

This script never needs to be edited when commands are added or changed.
The C++ binary is the single source of truth.
"""

import json
import sys
import textwrap
from datetime import date

# ============================================================
# RST formatting helpers
# ============================================================

def rst_heading(text, char):
    return f"{text}\n{char * len(text)}\n\n"

def rst_subheading(text, char="~"):
    return f"\n{text}\n{char * len(text)}\n\n"

def rst_option(name, description, required=False):
    """Formats a single option entry with consistent spacing.
    Extra blank line after each entry improves readability in the rendered page.
    """
    req = " *(required)*" if required else ""
    lines = [f".. option:: {name}{req}", ""]
    wrapped = textwrap.fill(description, width=70)
    for line in wrapped.splitlines():
        lines.append(f"   {line}")
    # Two blank lines after each option for visual separation
    lines.append("")
    lines.append("")
    return "\n".join(lines) + "\n"

def rst_command_entry(path, description):
    """One-line command entry as in the current man page Commands section."""
    return f":command:`{path}`\n  {description}\n\n"

# ============================================================
# Tree walking
# ============================================================

# Command family groupings for section headers in the man page.
# The key is the top-level subcommand name.
FAMILY_LABELS = {
    "user":      "User Commands",
    "bucket":    "Bucket Commands",
    "quota":     "Quota Commands",
    "ratelimit": "Rate Limit Commands",
}

def collect_commands(node, path_parts=None):
    """
    Recursively collect all leaf commands as (path, description, options) tuples.
    A leaf command is one with no further subcommands.
    """
    if path_parts is None:
        path_parts = []

    name = node.get("name", "")
    desc = node.get("description", "")
    subs = node.get("subcommands", [])
    opts = node.get("options", [])

    current_path = path_parts + ([name] if name else [])

    if not subs:
        return [(" ".join(current_path), desc, opts)]

    results = []
    for sub in subs:
        results.extend(collect_commands(sub, current_path))
    return results

def collect_all_options(node):
    """
    Collect all unique options across the entire tree,
    deduplicating by option name.
    """
    seen = {}
    def walk(n):
        for opt in n.get("options", []):
            name = opt["name"]
            if name not in seen:
                seen[name] = opt
        for sub in n.get("subcommands", []):
            walk(sub)
    walk(node)
    return list(seen.values())

def sort_options(opts, alphabetical=False):
    """
    Sort options: required first, then optional.
    When alphabetical=False (default): preserves registration order within each group.
    This keeps --help output and man page in the same order (both use registration order).
    When alphabetical=True: sorts each group alphabetically (used for global options section).
    """
    required = [o for o in opts if o.get("required")]
    optional  = [o for o in opts if not o.get("required")]
    if alphabetical:
        required = sorted(required, key=lambda o: o["name"])
        optional = sorted(optional, key=lambda o: o["name"])
    return required + optional

def group_commands_by_family(commands):
    """
    Group (path, desc, opts) tuples by their top-level command family.
    Returns an ordered list of (family_label, [commands]) pairs.
    """
    families = {}
    order = []
    for path, desc, opts in commands:
        top = path.split()[0] if path else "other"
        label = FAMILY_LABELS.get(top, top.title() + " Commands")
        if label not in families:
            families[label] = []
            order.append(label)
        families[label].append((path, desc, opts))
    return [(label, families[label]) for label in order]

# ============================================================
# RST generation
# ============================================================

def generate_manpage(tree):
    root = tree[0] if isinstance(tree, list) else tree
    today = date.today().strftime("%B %d, %Y")

    out = []

    # ---- Header ----
    out.append(":orphan:\n\n")
    out.append(".. _man-radosgw-admin:\n\n")

    title = "radosgw-admin -- rados REST gateway user administration utility"
    bar = "=" * (len(title) + 2)
    out.append(f"{bar}\n {title}\n{bar}\n\n")
    out.append(".. program:: radosgw-admin\n\n")
    out.append(f".. note::\n\n")
    out.append(f"   Auto-generated on {today} from ``radosgw-admin --export-tree``.\n")
    out.append(f"   **Do not edit manually.**\n")
    out.append(f"   Edit command definitions in ``src/tools/radosgw-admin.cc``\n")
    out.append(f"   then regenerate: ``radosgw-admin --export-tree | python3 generate_manpage.py``\n\n")

    # ---- Synopsis ----
    out.append(rst_heading("Synopsis", "="))
    out.append("| **radosgw-admin** *command* [ *options* *...* ]\n\n")

    # ---- Description ----
    out.append(rst_heading("Description", "="))
    desc = root.get("description", "").replace("\\n", " ")
    out.append(f"{desc}\n\n")

    # ---- Commands section ----
    out.append(rst_heading("Commands", "="))
    out.append(
        ":program:`radosgw-admin` provides the following commands.\n"
        "For per-command option details, see the `Per-Command Options`_ section below.\n\n"
    )

    all_commands = collect_commands(root)
    families = group_commands_by_family(all_commands)

    # Commands listed under family subheadings
    for family_label, cmds in families:
        out.append(rst_subheading(family_label, "-"))
        for path, desc, _ in cmds:
            out.append(rst_command_entry(path, desc))

    # ---- Global Options section ----
    out.append(rst_heading("Options", "="))
    out.append(
        "The following options are available globally and can be used with any command.\n\n"
    )

    # Always include the standard Ceph infrastructure options first
    out.append(".. option:: -c ceph.conf, --conf=ceph.conf\n\n")
    out.append("   Use ``ceph.conf`` instead of the default ``/etc/ceph/ceph.conf``.\n\n")
    out.append(".. option:: --cluster=<name>\n\n")
    out.append("   Set the cluster name (default: ceph).\n\n")
    out.append(".. option:: --id=<id>\n\n")
    out.append("   Set the ID portion of the client name.\n\n")

    # Remaining unique options from tree, sorted required-first then alphabetical
    all_opts = collect_all_options(root)
    # Exclude options we already wrote above and internal ones
    skip = {"--conf", "-c,--conf", "--cluster", "--id", "--help", "--help-all", "--export-tree"}
    all_opts = [o for o in all_opts if o["name"] not in skip]
    all_opts = sort_options(all_opts, alphabetical=True)  # global section: alphabetical

    for opt in all_opts:
        # Never mark required in global Options section — required status is
        # per-command, not global. --uid is required for 'user create' but
        # optional for 'user modify'. The Per-Command Options section below
        # correctly marks required per command.
        out.append(rst_option(opt["name"], opt["description"], required=False))

    # ---- Per-Command Options section ----
    out.append(rst_heading("Per-Command Options", "="))
    out.append(
        "The following details the specific options accepted by each command.\n"
        "Options marked *(required)* must be provided or the command will error.\n\n"
    )

    for family_label, cmds in families:
        # Family group heading
        out.append(rst_subheading(family_label, "-"))

        for path, desc, opts in cmds:
            if not opts:
                # Command takes no options beyond the global ones
                heading = f"``{path}``"
                out.append(f"{heading}\n{'~' * len(heading)}\n\n")
                out.append(f"{desc}\n\n")
                out.append("*No command-specific options.*\n\n")
                continue

            heading = f"``{path}``"
            out.append(f"{heading}\n{'~' * len(heading)}\n\n")
            out.append(f"{desc}\n\n")

            sorted_opts = sort_options(opts)  # registration order = same as --help
            for opt in sorted_opts:
                out.append(rst_option(
                    opt["name"],
                    opt["description"],
                    opt.get("required", False)
                ))

    # ---- Availability ----
    out.append(rst_heading("Availability", "="))
    out.append(
        ":program:`radosgw-admin` is part of Ceph, a massively scalable, "
        "open-source, distributed storage system. Please refer to the Ceph "
        "documentation at https://docs.ceph.com for more information.\n\n"
    )

    # ---- See Also ----
    out.append(rst_heading("See Also", "="))
    out.append(":doc:`ceph <ceph>`\\(8)\n")
    out.append(":doc:`radosgw <radosgw>`\\(8)\n")

    return "".join(out)

# ============================================================
# Entry point
# ============================================================

def main():
    raw = sys.stdin.read().strip()
    if not raw:
        print(
            "ERROR: No input.\n"
            "Run: ./radosgw_admin_poc --export-tree | python3 generate_manpage.py\n"
            "Or:  ./radosgw_admin_poc --export-tree > commands.json\n"
            "     python3 generate_manpage.py < commands.json",
            file=sys.stderr
        )
        sys.exit(1)

    try:
        tree = json.loads(raw)
    except json.JSONDecodeError as e:
        print(f"ERROR: Invalid JSON from --export-tree: {e}", file=sys.stderr)
        sys.exit(1)

    rst = generate_manpage(tree)
    print(rst)

if __name__ == "__main__":
    main()
