#!/usr/bin/env python3
"""
generate_adminguide.py
======================
Generates a skeleton for doc/radosgw/admin.rst from the command tree JSON.

Two approaches are demonstrated — choose one or combine them:

Generates a skeleton for doc/radosgw/admin.rst from the command tree JSON.

Uses a marker-based approach: human-written narrative is preserved between
special RST comment markers. Safe to regenerate at any time without losing prose.

A 1:1 dictionary keyed by command path guarantees narrative content is never
lost or misplaced. This achieves the same goal as Ceph's confval/YAML pattern
for config options — C++ is the single source of truth — but requires no custom
Sphinx extension, no YAML files, and produces one single file.

Usage:
  ./radosgw_admin_poc --export-tree | python3 generate_adminguide.py
  ./radosgw_admin_poc --export-tree | python3 generate_adminguide.py --existing=admin.rst > admin.rst

Usage:
  ./radosgw_admin_poc --export-tree | python3 generate_adminguide.py
  ./radosgw_admin_poc --export-tree | python3 generate_adminguide.py --approach=includes
  ./radosgw_admin_poc --export-tree | python3 generate_adminguide.py --approach=markers > admin.rst
"""

import json
import sys
import os
import re
import textwrap
import argparse
from datetime import date

# ============================================================
# Shared helpers
# ============================================================

FAMILY_LABELS = {
    "user":      "User Management",
    "bucket":    "Bucket Management",
    "quota":     "Quota Management",
    "ratelimit": "Rate Limit Management",
}

def collect_by_family(node, path_parts=None):
    """Returns dict of {family: [(path, desc, opts), ...]}"""
    if path_parts is None:
        path_parts = []
    name  = node.get("name", "")
    subs  = node.get("subcommands", [])
    opts  = node.get("options", [])
    desc  = node.get("description", "")
    current_path = path_parts + ([name] if name else [])

    if not subs:
        family = current_path[0] if current_path else "other"
        return {family: [(" ".join(current_path), desc, opts)]}

    result = {}
    for sub in subs:
        for fam, cmds in collect_by_family(sub, current_path).items():
            result.setdefault(fam, []).extend(cmds)
    return result

def sort_options(opts):
    req = sorted([o for o in opts if o.get("required")],      key=lambda o: o["name"])
    opt = sorted([o for o in opts if not o.get("required")],  key=lambda o: o["name"])
    return req + opt

def command_ref_block(path, desc, opts):
    """
    Generates the purely mechanical reference block for one command:
    syntax, required options list, optional options list.
    This part is always auto-generated and never hand-edited.
    """
    lines = []

    # Build syntax string showing required vs optional flags
    syntax_parts = ["radosgw-admin", path]
    for o in opts:
        flag = o["name"].split(",")[0]  # take first form e.g. --uid
        if o.get("required"):
            syntax_parts.append(f"{flag}=<value>")
        else:
            syntax_parts.append(f"[{flag}=<value>]")

    lines.append(".. code-block:: bash\n")
    lines.append(f"   {' '.join(syntax_parts)}\n\n")

    sorted_opts = sort_options(opts)
    req_opts = [o for o in sorted_opts if o.get("required")]
    opt_opts = [o for o in sorted_opts if not o.get("required")]

    if req_opts:
        lines.append("**Required options:**\n\n")
        for o in req_opts:
            lines.append(f"``{o['name']}``\n  {o['description']}\n\n")

    if opt_opts:
        lines.append("**Optional options:**\n\n")
        for o in opt_opts:
            lines.append(f"``{o['name']}``\n  {o['description']}\n\n")

    return "".join(lines)

# ============================================================
# Marker-based Approach
#
# The generator writes a single admin.rst file. Human narrative
# goes between special RST comment markers. On subsequent runs,
# the generator extracts everything between markers before
# regenerating, then re-inserts the preserved content.
#
# Markers are RST comments — invisible to readers in HTML/PDF output.
# Empty markers produce no visible output.
#
# Workflow:
#   1. First run: generates skeleton with empty markers
#   2. Developer fills in narrative between the markers
#   3. New command added to C++: re-run generator
#   4. Old narrative is preserved, new command section added
#   5. Developer only needs to write narrative for the new command
# ============================================================

MARKER_BEGIN = ".. # NARRATIVE-BEGIN: {key}"
MARKER_END   = ".. # NARRATIVE-END: {key}"

def extract_narratives(existing_content):
    """
    Parses existing admin.rst and extracts all human-written content
    between NARRATIVE-BEGIN and NARRATIVE-END markers.

    Returns a 1:1 dictionary of {command_path_key: narrative_text}.

    The dictionary is 1:1 because every command has a unique path key
    (e.g. "user-create", "bucket-list", "bucket-logging-flush").
    No two commands share a key, so narrative content can never be
    assigned to the wrong command or accidentally overwritten.

    When reinserting, the generator calls narratives.get("user-create")
    and puts exactly that content back between the user-create markers.
    """
    narratives = {}
    pattern = re.compile(
        r'\.\. # NARRATIVE-BEGIN: (\S+)\n(.*?)\n\.\. # NARRATIVE-END: \1',
        re.DOTALL
    )
    for match in pattern.finditer(existing_content):
        key  = match.group(1)
        text = match.group(2).strip()
        if text:
            narratives[key] = text
    return narratives

def generate_adminguide(root, existing_file=None):
    """
    Generates admin.rst using the marker approach.
    If existing_file is provided, preserves narrative from it.
    """
    narratives = {}
    if existing_file and os.path.exists(existing_file):
        with open(existing_file) as f:
            narratives = extract_narratives(f.read())

    today = date.today().strftime("%B %d, %Y")
    out = []

    out.append(".. _radosgw-admin-guide:\n\n")
    out.append("============\n Admin Guide\n============\n\n")
    out.append(
        f".. note::\n\n"
        f"   Command reference blocks auto-generated on {today}.\n"
        f"   Narrative sections between ``NARRATIVE-BEGIN/END`` markers\n"
        f"   are preserved across regenerations and hand-edited by authors.\n\n"
    )
    out.append(
        "After the Ceph Object Storage service is up and running, it can be\n"
        "administered with user management, access controls, quotas, and usage tracking.\n\n"
    )

    families = collect_by_family(root)

    for family_key, cmds in families.items():
        section_label = FAMILY_LABELS.get(family_key, family_key.title())
        out.append(f"{section_label}\n{'=' * len(section_label)}\n\n")

        # Section-level narrative (e.g. overview of user management)
        section_key = f"section-{family_key}"
        narrative   = narratives.get(section_key, "")
        out.append(MARKER_BEGIN.format(key=section_key) + "\n")
        if narrative:
            out.append(narrative + "\n")
        out.append(MARKER_END.format(key=section_key) + "\n\n")

        for path, desc, opts in cmds:
            # Command-level heading
            verb_parts = path.split()[1:]  # e.g. ["create"] from "user create"
            heading = " ".join(w.title() for w in verb_parts) + " a " + family_key.title() \
                      if verb_parts else desc
            # Use description as heading if path is just one word
            heading = desc[:50] if len(path.split()) <= 1 else heading
            out.append(f"{heading}\n{'-' * len(heading)}\n\n")

            # Stable anchor for cross-referencing from other docs
            anchor = ".. _radosgw-admin-" + path.replace(" ", "-") + ":"
            out.append(f"{anchor}\n\n")

            # Command-level narrative (preserved from previous run)
            cmd_key   = path.replace(" ", "-")
            narrative = narratives.get(cmd_key, "")
            out.append(MARKER_BEGIN.format(key=cmd_key) + "\n")
            if narrative:
                out.append(narrative + "\n")
            out.append(MARKER_END.format(key=cmd_key) + "\n\n")

            # Auto-generated reference block (always regenerated)
            out.append(command_ref_block(path, desc, opts))

    return "".join(out)

# ============================================================
# Entry point
# ============================================================

def main():
    parser = argparse.ArgumentParser(description="Generate admin.rst skeleton")
    parser.add_argument(
        "--existing",
        default="admin.rst",
        help="Existing admin.rst to preserve narratives from (markers approach only)"
    )
    args = parser.parse_args()

    raw = sys.stdin.read().strip()
    if not raw:
        print(
            "ERROR: No input.\n"
            "Run: ./radosgw_admin_poc --export-tree | python3 generate_adminguide.py",
            file=sys.stderr
        )
        sys.exit(1)

    try:
        tree = json.loads(raw)
    except json.JSONDecodeError as e:
        print(f"ERROR: Invalid JSON: {e}", file=sys.stderr)
        sys.exit(1)

    root = tree[0] if isinstance(tree, list) else tree

    rst = generate_adminguide(root, existing_file=args.existing)
    print(rst)

if __name__ == "__main__":
    main()
