# radosgw-admin UX and Documentation Improvements

GSoC 2026 proposal work for the Ceph project.
Tracker issue: [#74508](https://tracker.ceph.com/issues/74508)
Mentors: Yuval Lifshitz, Jacques Heunis

---

## Problem

`radosgw-admin` documentation is maintained in three separate places —
the `--help` output, the man page (`doc/man/8/radosgw-admin.rst`), and the
admin guide (`doc/radosgw/admin.rst`) — with no mechanism to keep them in sync.
They drift apart as commands are added or changed, leaving users with incomplete
or inaccurate documentation.

---

## Proposed Solution

Replace hand-written documentation with a programmatic pipeline using
[CLI11](https://github.com/CLIUtils/CLI11) as the single source of truth:

```
radosgw-admin.cc  (CLI11 definitions)
        │
        ├── --help / user --help       → auto-generated at runtime
        └── --export-tree → commands.json
                ├── generate_manpage.py    → doc/man/8/radosgw-admin.rst
                └── generate_adminguide.py → doc/radosgw/admin.rst
```

---

## Repository Structure

```
├── audit/
│   └── radosgw-admin-audit.md       Systematic mismatch analysis between
│                                    --help output, man page, admin guide,
│                                    and test file (help.t)
│
└── poc/
    ├── radosgw_admin_poc.cpp        C++ PoC using CLI11
    ├── generate_manpage.py          Generates man page RST from command tree JSON
    ├── generate_adminguide.py       Generates admin guide RST from command tree JSON
    ├── README.md                    Full demo guide with expected output
    └── output/
        ├── commands.json            Generated command tree
        ├── radosgw-admin.rst        Generated man page
        └── admin.rst                Generated admin guide skeleton
```

---

## Audit

[`audit/radosgw-admin-audit.md`](audit/radosgw-admin-audit.md)

Systematic comparison across four documentation sources: `--help` output,
the man page, the admin guide, and the `help.t` test file.

---

## Proof of Concept

[`poc/README.md`](poc/README.md) — full instructions and expected output for every demo.

Pre-generated outputs are available in [`poc/output/`](poc/output/) for inspection
without running the binary.

---

## Background

GSoC 2026 application to the Ceph project.
Prior merged PR in Ceph from earlier work with the mentor Yuval Lifshitz on the TBB iterator migration.
