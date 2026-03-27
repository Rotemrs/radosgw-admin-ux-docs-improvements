/**
 * radosgw-admin CLI11 Proof of Concept
 * =====================================
 * Demonstrates how CLI11 solves the documentation and UX problems
 * described in https://tracker.ceph.com/issues/74508
 *
 * Problems demonstrated and solved:
 *   1. Auto-generated --help from code definitions (no hand-written strings)
 *   2. Per-command/subcommand specific help (e.g. radosgw-admin user --help)
 *   3. Hierarchical error listing for incomplete commands (3-level nesting shown)
 *   4. Required argument enforcement with automatic error messages
 *   5. Shared options handled cleanly — two patterns shown:
 *        a. Family helpers: one function registers several options per command family
 *        b. Flag registration: one function registers one flag across many commands
 *   6. True global options (--conf, --id, --cluster) registered at root level
 *   7. Option groups (nested flags) demonstrated with rate-limit options
 *   8. Tree introspection for doc generation (--export-tree flag)
 *   9. Column width fix for readable help output
 *
 * Compile:
 *   g++ -std=c++17 radosgw_admin_poc.cpp -o radosgw_admin_poc
 *
 * Run examples — see README.md for full expected output of each:
 *   ./radosgw_admin_poc --help
 *   ./radosgw_admin_poc user --help
 *   ./radosgw_admin_poc user --help-all
 *   ./radosgw_admin_poc bucket --help
 *   ./radosgw_admin_poc bucket logging
 *   ./radosgw_admin_poc user create --uid=johndoe --display-name="John Doe"
 *   ./radosgw_admin_poc user create                    (shows missing required args)
 *   ./radosgw_admin_poc ratelimit set --help           (shows option group)
 *   ./radosgw_admin_poc --export-tree > commands.json
 *   ./radosgw_admin_poc --export-tree | python3 generate_manpage.py > radosgw-admin.rst
 *
 * Usage notes:
 *   - Options accept values as --uid=johndoe OR --uid johndoe (both work)
 *   - Values containing spaces must be quoted: --display-name="John Doe"
 *   - Both -h and --help show help for the current command level
 *   - --help-all shows the full expanded tree from the current level down
 *   - To avoid -h being consumed as a flag value, use --flag=value syntax
 *     e.g. use --bucket=mybucket -h, not --bucket -h
 */

#include "CLI11.hpp"
#include <iostream>
#include <optional>
#include <string>
#include <vector>

// ============================================================
// JSON serialization
// In real Ceph code this would use the existing json library.
// ============================================================

std::string json_escape(const std::string& s) {
    std::string result;
    for (char c : s) {
        if (c == '"')  result += "\\\"";
        else if (c == '\\') result += "\\\\";
        else if (c == '\n') result += "\\n";
        else result += c;
    }
    return result;
}

// Recursively walks the CLI11 App tree and serializes it to JSON.
// This is the CORE of the doc generation pipeline.
// The same JSON feeds both generate_manpage.py and generate_adminguide.py.
//
// Why this matters: Python cannot read C++ objects directly.
// This function is the bridge that makes automatic doc generation possible.
// Without tree introspection (unique to CLI11 among evaluated frameworks),
// a separate hand-maintained metadata file would be needed — recreating
// the exact drift problem the project is solving.
std::string tree_to_json(const CLI::App* app, int indent = 0) {
    std::string pad(indent * 2, ' ');
    std::string inner(indent * 2 + 2, ' ');
    std::string result;

    result += pad + "{\n";
    result += inner + "\"name\": \"" + json_escape(app->get_name()) + "\",\n";
    result += inner + "\"description\": \"" + json_escape(app->get_description()) + "\",\n";

    // Options: skip built-in CLI11 flags and internal flags
    result += inner + "\"options\": [\n";
    bool first_opt = true;
    for (const CLI::Option* opt : app->get_options()) {
        const std::string name = opt->get_name();
        if (name == "--help" || name == "--help-all" || name == "--export-tree")
            continue;
        if (!first_opt) result += ",\n";
        first_opt = false;
        result += inner + "  {\n";
        result += inner + "    \"name\": \"" + json_escape(name) + "\",\n";
        result += inner + "    \"description\": \"" + json_escape(opt->get_description()) + "\",\n";
        result += inner + "    \"required\": " + (opt->get_required() ? "true" : "false") + ",\n";
        result += inner + "    \"type\": \"" + json_escape(opt->get_type_name()) + "\"\n";
        result += inner + "  }";
    }
    result += "\n" + inner + "],\n";

    // Subcommands: recurse into children
    result += inner + "\"subcommands\": [\n";
    bool first_sub = true;
    for (const CLI::App* sub : app->get_subcommands({})) {
        if (!first_sub) result += ",\n";
        first_sub = false;
        result += tree_to_json(sub, indent + 2);
    }
    result += "\n" + inner + "]\n";
    result += pad + "}";
    return result;
}

// ============================================================
// OPTION STORAGE STRUCTS
// Grouping related option values into structs keeps main() clean
// and makes it obvious which commands share the same underlying data.
// ============================================================

struct GlobalOpts {
    // True global options — registered on root app, available everywhere
    std::string conf;       // --conf / -c
    std::string cluster;    // --cluster
    std::string id;         // --id
    // Cross-cutting options — NOT global (not on root app).
    // Registered only on commands that support them via register_format_flag()
    // and register_tenant_flag(). They appear in --help only for those commands.
    std::string format;     // --format: json, xml
    std::string tenant;     // --tenant
};

struct UserOpts {
    std::string uid;
    std::string display_name;
    std::string email;
    bool admin = false;
    // std::optional distinguishes "user did not provide --max-buckets"
    // from "user explicitly passed 1000". In Ceph, "not provided" means
    // use the server default, not our local default.
    std::optional<int> max_buckets;
};

struct BucketOpts {
    std::string bucket_name;
    std::string bucket_id;
    bool allow_unordered = false;
    bool purge_objects = false;
};

// Rate limit option group — demonstrates CLI11 option groups (nested flags).
// In real radosgw-admin these appear under ratelimit set/get commands.
// Grouping them makes --help output organized and the code self-documenting.
struct RateLimitOpts {
    int max_read_ops    = 0;
    int max_write_ops   = 0;
    int max_read_bytes  = 0;
    int max_write_bytes = 0;
};

// ============================================================
// GENERAL VALIDATOR: prevents flag-like values being passed to string options
//
// Problem: CLI11 allows `--uid -x` where it treats "-x" as the value for --uid.
// This can happen accidentally (e.g. user types --uid -h intending to see help).
// In real Ceph this is also a problem, though Ceph's parser handles it differently.
//
// Solution: validate that string option values don't start with '-'.
// Usage: add ->check(no_flag_value) to any string option where this matters.
// Always use --option=value syntax when the value might start with '-'.
//
// Note: this validator is deliberately NOT applied to every option because
// some legitimate values could start with '-' (e.g. negative numbers, paths).
// Apply selectively to options where a flag-like value would be nonsensical.
// ============================================================
const CLI::Validator no_flag_value = CLI::Validator(
    [](std::string& val) -> std::string {
        if (!val.empty() && val[0] == '-') {
            return "Value '" + val + "' looks like a flag. "
                   "Use --option=value syntax to pass values starting with '-'.";
        }
        return std::string{};
    },
    ""
);

// ============================================================
// OPTION HELPER FUNCTIONS — Pattern A: Family helpers
//
// One function registers SEVERAL options on ONE command.
// Best for options that are conceptually owned by a command family:
//   all user commands share --uid and --display-name
//   all bucket commands share --bucket and --bucket-id
//
// This avoids repeating the same option definition in every command.
// The description string is written once — no risk of drift.
// ============================================================

// Registers user-identity options.
// --uid and --display-name are NOT marked required here because
// different commands have different requirements — callers mark
// ->required() on whichever they need after calling this.
void add_user_identity_opts(CLI::App* cmd, UserOpts& opts) {
    // multi_option_policy(Throw): if --uid appears twice, CLI11 throws an error.
    // Without this, CLI11's default is "last value wins" which can hide bugs.
    // e.g. `user create --uid=alice --uid=bob` would silently use "bob".
    // Throwing makes this an explicit error, which is the correct behavior.
    cmd->add_option("--uid",          opts.uid,          "The user ID")
       ->check(no_flag_value)
       ->multi_option_policy(CLI::MultiOptionPolicy::Throw);
    cmd->add_option("--display-name", opts.display_name, "User display name")
       ->check(no_flag_value)
       ->multi_option_policy(CLI::MultiOptionPolicy::Throw);
}

// Registers bucket-identity options.
void add_bucket_identity_opts(CLI::App* cmd, BucketOpts& opts) {
    cmd->add_option("--bucket",    opts.bucket_name, "The bucket name")
       ->check(no_flag_value);
    cmd->add_option("--bucket-id", opts.bucket_id,   "The bucket ID")
       ->check(no_flag_value);
}

// Registers rate-limit options as a named CLI11 option group.
// This is CLI11's "nested flags" feature: options appear as a labeled
// section in --help output, making it clear they belong together.
// Try: ./radosgw_admin_poc ratelimit set --help
void add_rate_limit_opts(CLI::App* cmd, RateLimitOpts& opts) {
    auto* grp = cmd->add_option_group(
        "Rate limiting options",
        "Controls maximum request/byte rates per accumulation interval per RGW");
    grp->add_option("--max-read-ops",    opts.max_read_ops,
        "Max read requests per interval (0 = unlimited)");
    grp->add_option("--max-write-ops",   opts.max_write_ops,
        "Max write requests per interval (0 = unlimited)");
    grp->add_option("--max-read-bytes",  opts.max_read_bytes,
        "Max read bytes per interval (0 = unlimited)");
    grp->add_option("--max-write-bytes", opts.max_write_bytes,
        "Max write bytes per interval (0 = unlimited)");
}

// ============================================================
// OPTION HELPER FUNCTIONS — Pattern B: Flag-to-many-commands
//
// One function registers ONE flag on MANY commands.
// Best for cross-cutting flags that appear across multiple command families:
//   --format appears in user info, bucket list, bucket stats, zone get, etc.
//   --tenant appears wherever a tenant context is relevant
//
// The explicit command list IS the documentation of which commands
// support this flag. Previously --format was listed once globally in the
// hand-written usage string, silently ignored by most commands.
// Now the list below is the ground truth — no ambiguity.
//
// To add a new command that supports --format: add one entry to the list.
// Nothing else changes. --help updates, man page regenerates automatically.
// ============================================================

void register_format_flag(std::vector<CLI::App*> cmds, std::string& format) {
    for (CLI::App* cmd : cmds) {
        cmd->add_option("--format", format,
            "Output format for the response: json, xml");
    }
}

void register_tenant_flag(std::vector<CLI::App*> cmds, std::string& tenant) {
    for (CLI::App* cmd : cmds) {
        cmd->add_option("--tenant", tenant, "Tenant name");
    }
}

void register_purge_data_flag(std::vector<CLI::App*> cmds, bool& purge_data) {
    for (CLI::App* cmd : cmds) {
        cmd->add_flag("--purge-data", purge_data,
            "Also purge all data associated with the user");
    }
}

// ============================================================
// FORMATTER HELPER
// Sets column width on an app and all its subcommands recursively.
// This fixes the text wrapping issue for long option names like
// --display-name. Must be called AFTER all subcommands are defined.
// ============================================================
void set_column_width(CLI::App* app, int width) {
    app->get_formatter()->column_width(width);
    for (CLI::App* sub : app->get_subcommands({})) {
        set_column_width(sub, width);
    }
}

// ============================================================
// FALLTHROUGH HELPER
// Recursively enables fallthrough() on ALL nodes in the tree.
//
// Why every node needs fallthrough, not just top-level subcommands:
//   radosgw-admin bucket logging flush --conf=ceph.conf
//   The option reaches: flush → logging → bucket → root
//   If any node in the chain lacks fallthrough, the option stops there.
//
// This matches real Ceph behavior: global options work at any depth.
// fallthrough() does NOT weaken validation — unknown options still
// cause errors. It only allows recognized root-level options to
// bubble up through the subcommand chain.
// ============================================================
void set_fallthrough_all(CLI::App* app) {
    app->fallthrough();
    for (CLI::App* sub : app->get_subcommands({})) {
        set_fallthrough_all(sub);
    }
}

// ============================================================
// MAIN
// ============================================================

int main(int argc, char** argv) {

    // Shared option storage
    GlobalOpts    global_opts;
    UserOpts      user_opts;
    BucketOpts    bucket_opts;
    RateLimitOpts ratelimit_opts;
    bool export_tree = false;
    bool purge_data  = false;
    bool sync_stats  = false;
    bool fix_index   = false;
    // quota_scope is reused for both --quota-scope and --ratelimit-scope.
    // Both are single-word scope values (bucket/user/anonymous) stored in
    // the same variable since only one command runs at a time.
    std::string quota_scope;
    int  max_objects = -1;
    std::string max_size;
    std::string source_bucket;

    // --------------------------------------------------------
    // Root application
    // --------------------------------------------------------
    CLI::App app{
        "Ceph Object Gateway user administration utility.\n"
        "See: https://docs.ceph.com/en/latest/radosgw/admin/"
    };
    app.name("radosgw-admin");
    // 0 minimum allows --export-tree and global options without a subcommand
    app.require_subcommand(0, 1);

    // --export-tree: triggers JSON output consumed by Python doc generators.
    // This flag is what makes the entire doc pipeline possible.
    app.add_flag("--export-tree", export_tree,
        "Export the full command tree as JSON (for doc generation)");

    // --help-all: expands the full command tree from the current level.
    // Use instead of --help when you want to see all subcommands and all
    // their flags in one view, without drilling down level by level.
    // e.g. ./radosgw_admin_poc user --help-all
    app.set_help_all_flag("--help-all", "Expand all help");

    // --------------------------------------------------------
    // TRUE GLOBAL OPTIONS
    //
    // Registered on the ROOT app — automatically available to every
    // subcommand without being re-registered anywhere.
    //
    // This demonstrates CLI11's clean handling of global options.
    // boost::program_options cannot do this with nested subcommands
    // because it has no native subcommand model.
    //
    // In real radosgw-admin, --conf/-c, --cluster, and --id come from
    // Ceph's common argument infrastructure and genuinely work with every
    // command — they control which cluster to connect to.
    //
    // CEPH COMPATIBILITY — global options before OR after subcommand:
    // Real radosgw-admin accepts global options both BEFORE and AFTER
    // the subcommand verb at ANY nesting depth. CLI11 by default only
    // accepts them before. set_fallthrough_all() (called after tree
    // construction below) recursively enables fallthrough() on EVERY node,
    // so global options bubble up to root from any depth.
    // Unknown options (typos) still cause errors — validation is preserved.
    //
    //   ./radosgw_admin_poc --cluster=ceph user create --uid=foo  (before)
    //   ./radosgw_admin_poc user create --uid=foo --cluster=ceph  (after)
    // Both work correctly with fallthrough().
    // --------------------------------------------------------
    app.add_option("-c,--conf", global_opts.conf,
        "Path to ceph.conf (default: /etc/ceph/ceph.conf)");
    app.add_option("--cluster", global_opts.cluster,
        "Cluster name (default: ceph)");
    app.add_option("--id", global_opts.id,
        "ID portion of the client name");

    // --------------------------------------------------------
    // USER commands
    //
    // Problem solved: `radosgw-admin user --help` previously showed
    // ALL 200 commands in one wall of text. Now it shows only user
    // commands. Use --help-all to expand the full tree from here down.
    // --------------------------------------------------------
    auto user = app.add_subcommand("user", "User management operations");
    user->require_subcommand(1);
    // Note: fallthrough() is set recursively on ALL nodes after tree
    // construction via set_fallthrough_all() below — see that call.

    // user create — requires both --uid and --display-name
    auto user_create = user->add_subcommand("create", "Create a new user");
    add_user_identity_opts(user_create, user_opts);
    user_create->get_option("--uid")->required();
    user_create->get_option("--display-name")->required();
    user_create->add_option("--email", user_opts.email,
        "The email address of the user");
    user_create->add_flag("--admin", user_opts.admin,
        "Set the admin flag on the user");
    user_create->add_option("--max-buckets", user_opts.max_buckets,
        "Maximum number of buckets for the user (server default: 1000 if not specified)");

    // user info — requires only --uid
    auto user_info = user->add_subcommand("info",
        "Display information for a user including subusers and keys");
    add_user_identity_opts(user_info, user_opts);
    user_info->get_option("--uid")->required();

    // user modify
    auto user_modify = user->add_subcommand("modify", "Modify an existing user");
    add_user_identity_opts(user_modify, user_opts);
    user_modify->get_option("--uid")->required();
    user_modify->add_option("--email", user_opts.email,
        "New email address for the user");
    user_modify->add_flag("--admin", user_opts.admin,
        "Set or clear the admin flag on the user");

    // user rm — --purge-data registered later via register_purge_data_flag
    auto user_rm = user->add_subcommand("rm",
        "Remove a user and all associated subusers");
    add_user_identity_opts(user_rm, user_opts);
    user_rm->get_option("--uid")->required();

    // user list — no required options
    auto user_list = user->add_subcommand("list", "List all users");

    // user suspend
    auto user_suspend = user->add_subcommand("suspend",
        "Suspend a user, disabling their access");
    add_user_identity_opts(user_suspend, user_opts);
    user_suspend->get_option("--uid")->required();

    // user enable
    auto user_enable = user->add_subcommand("enable",
        "Re-enable a previously suspended user");
    add_user_identity_opts(user_enable, user_opts);
    user_enable->get_option("--uid")->required();

    // user stats
    auto user_stats = user->add_subcommand("stats",
        "Show user stats as accounted by the quota subsystem");
    add_user_identity_opts(user_stats, user_opts);
    user_stats->get_option("--uid")->required();
    user_stats->add_flag("--sync-stats", sync_stats,
        "Update user stats with current stats from bucket indexes");

    // --------------------------------------------------------
    // BUCKET commands
    // --------------------------------------------------------
    auto bucket = app.add_subcommand("bucket", "Bucket management operations");
    bucket->require_subcommand(1);

    // bucket list
    auto bucket_list = bucket->add_subcommand("list",
        "List buckets, or objects within a bucket if --bucket is specified");
    add_bucket_identity_opts(bucket_list, bucket_opts);
    bucket_list->add_flag("--allow-unordered", bucket_opts.allow_unordered,
        "Faster unsorted listing, removes ordering requirement");

    // bucket stats
    auto bucket_stats = bucket->add_subcommand("stats",
        "Return statistics and internal information about a bucket");
    add_bucket_identity_opts(bucket_stats, bucket_opts);
    bucket_stats->get_option("--bucket")->required();

    // bucket rm
    auto bucket_rm = bucket->add_subcommand("rm", "Remove a bucket");
    add_bucket_identity_opts(bucket_rm, bucket_opts);
    bucket_rm->get_option("--bucket")->required();
    bucket_rm->add_flag("--purge-objects", bucket_opts.purge_objects,
        "Remove all objects before deleting (required for non-empty buckets)");

    // bucket link
    auto bucket_link = bucket->add_subcommand("link",
        "Link a bucket to a specified user, changing ownership");
    add_bucket_identity_opts(bucket_link, bucket_opts);
    add_user_identity_opts(bucket_link, user_opts);
    bucket_link->get_option("--bucket")->required();
    bucket_link->get_option("--uid")->required();

    // bucket unlink
    auto bucket_unlink = bucket->add_subcommand("unlink",
        "Unlink a bucket from a specified user");
    add_bucket_identity_opts(bucket_unlink, bucket_opts);
    add_user_identity_opts(bucket_unlink, user_opts);
    bucket_unlink->get_option("--bucket")->required();
    bucket_unlink->get_option("--uid")->required();

    // bucket check
    auto bucket_check = bucket->add_subcommand("check",
        "Check bucket index by verifying size and object count stats");
    add_bucket_identity_opts(bucket_check, bucket_opts);
    bucket_check->add_flag("--fix", fix_index,
        "Fix the bucket index in addition to checking it");

    // --------------------------------------------------------
    // BUCKET LOGGING subgroup — demonstrates 3-level nesting
    //
    //   radosgw-admin bucket logging flush
    //   radosgw-admin bucket logging info
    //   radosgw-admin bucket logging list
    //
    // Problem solved: typing `radosgw-admin bucket logging` with no
    // further verb previously gave a cryptic error. Now CLI11 prints
    // "A subcommand is required" and lists the valid next options.
    // require_subcommand(1) is the single line that enables this.
    // --------------------------------------------------------
    auto logging = bucket->add_subcommand("logging", "Bucket logging operations");
    logging->require_subcommand(1);

    auto logging_flush = logging->add_subcommand("flush",
        "Flush pending log records to the log bucket immediately");
    logging_flush->add_option("--bucket", source_bucket,
        "The source bucket to flush logs for")->required();

    auto logging_info = logging->add_subcommand("info",
        "Get logging configuration for a source bucket, "
        "or list sources logging to a specific log bucket");
    logging_info->add_option("--bucket", source_bucket,
        "The source bucket to get logging info for");

    auto logging_list = logging->add_subcommand("list",
        "List the log objects pending commit for a source bucket");
    logging_list->add_option("--bucket", source_bucket,
        "The source bucket")->required();

    // --------------------------------------------------------
    // QUOTA commands
    // --------------------------------------------------------
    auto quota = app.add_subcommand("quota",
        "Quota management for users and buckets");
    quota->require_subcommand(1);

    auto quota_set = quota->add_subcommand("set",
        "Set quota parameters for a user, bucket, or account");
    add_user_identity_opts(quota_set, user_opts);
    add_bucket_identity_opts(quota_set, bucket_opts);
    quota_set->add_option("--quota-scope", quota_scope,
        "Scope of the quota: bucket, user, or account")->required();
    quota_set->add_option("--max-objects", max_objects,
        "Maximum number of objects (negative value to disable)");
    quota_set->add_option("--max-size", max_size,
        "Maximum storage size in B/K/M/G/T (negative value to disable)");

    auto quota_enable = quota->add_subcommand("enable",
        "Enable quota enforcement for a user or bucket");
    add_user_identity_opts(quota_enable, user_opts);
    quota_enable->add_option("--quota-scope", quota_scope,
        "Scope: bucket or user")->required();

    auto quota_disable = quota->add_subcommand("disable",
        "Disable quota enforcement for a user or bucket");
    add_user_identity_opts(quota_disable, user_opts);
    quota_disable->add_option("--quota-scope", quota_scope,
        "Scope: bucket or user")->required();

    // --------------------------------------------------------
    // RATELIMIT commands — demonstrates CLI11 option groups
    //
    // Option groups (add_option_group) are CLI11's "nested flags" feature.
    // They appear as a labeled section in --help, making it immediately
    // clear that a set of related options belong together semantically.
    // Try: ./radosgw_admin_poc ratelimit set --help
    // --------------------------------------------------------
    auto ratelimit = app.add_subcommand("ratelimit",
        "Rate limit management for users and buckets");
    ratelimit->require_subcommand(1);

    auto ratelimit_set = ratelimit->add_subcommand("set",
        "Set rate limit parameters for a user or bucket");
    add_user_identity_opts(ratelimit_set, user_opts);
    add_bucket_identity_opts(ratelimit_set, bucket_opts);
    ratelimit_set->add_option("--ratelimit-scope", quota_scope,
        "Scope of the rate limit: bucket, user, or anonymous")->required();
    add_rate_limit_opts(ratelimit_set, ratelimit_opts);  // option group

    auto ratelimit_get = ratelimit->add_subcommand("get",
        "Get current rate limit parameters for a user or bucket");
    add_user_identity_opts(ratelimit_get, user_opts);
    add_bucket_identity_opts(ratelimit_get, bucket_opts);
    ratelimit_get->add_option("--ratelimit-scope", quota_scope,
        "Scope: bucket, user, or anonymous")->required();

    // ratelimit enable/disable identify the target via --uid OR --bucket.
    // In real radosgw-admin, --bucket is also valid here. For the PoC
    // we show user-scope as the primary example; bucket scope would simply
    // add add_bucket_identity_opts() the same way ratelimit_set does.
    auto ratelimit_enable = ratelimit->add_subcommand("enable",
        "Enable rate limiting for a user or bucket");
    add_user_identity_opts(ratelimit_enable, user_opts);
    ratelimit_enable->add_option("--ratelimit-scope", quota_scope,
        "Scope: bucket, user, or anonymous")->required();

    auto ratelimit_disable = ratelimit->add_subcommand("disable",
        "Disable rate limiting for a user or bucket");
    add_user_identity_opts(ratelimit_disable, user_opts);
    ratelimit_disable->add_option("--ratelimit-scope", quota_scope,
        "Scope: bucket, user, or anonymous")->required();

    // --------------------------------------------------------
    // CROSS-CUTTING FLAG REGISTRATION — Pattern B in action
    //
    // Each call below registers ONE flag on ALL commands that support it.
    //
    // Benefit 1: No duplication — description written once, used everywhere
    // Benefit 2: Explicit truth — this list IS the spec of which commands
    //            support this flag. The old hand-written string listed
    //            --format globally, silently ignored by most commands.
    // Benefit 3: Easy to extend — new command that needs --format?
    //            Add one line here. --help and man page update automatically.
    // Benefit 4: Accurate tree — --export-tree reflects exactly which
    //            commands support which flags, so the generated man page
    //            is precise rather than misleadingly global.
    // --------------------------------------------------------

    // --format: commands that produce formatted output
    register_format_flag({
        user_create,     // returns new user JSON
        user_info,       // returns user info JSON
        user_modify,     // returns modified user JSON
        user_list,       // returns user list JSON
        bucket_list,     // returns bucket/object list
        bucket_stats,    // returns bucket stats JSON
        quota_set,       // returns updated quota JSON
        ratelimit_get    // returns rate limit config JSON
    }, global_opts.format);

    // --tenant: commands where tenant context is meaningful
    register_tenant_flag({
        user_create, user_info, user_modify, user_rm,
        bucket_list, bucket_stats, bucket_link
    }, global_opts.tenant);

    // --purge-data: commands that can permanently delete user data
    register_purge_data_flag({
        user_rm
        // In full radosgw-admin: also subuser rm, etc.
    }, purge_data);

    // --------------------------------------------------------
    // Post-tree setup: called AFTER all subcommands are defined.
    // Both helpers recurse through the entire tree.
    // --------------------------------------------------------

    // Fix text wrapping for long option names like --display-name
    set_column_width(&app, 45);

    // Enable Ceph-compatible global option placement at ALL tree depths.
    // Global options (--conf, --cluster, --id) now work whether placed
    // before OR after any subcommand at any nesting level.
    // e.g. both of these work:
    //   ./radosgw_admin_poc bucket logging flush --bucket=b --conf=ceph.conf
    //   ./radosgw_admin_poc --conf=ceph.conf bucket logging flush --bucket=b
    set_fallthrough_all(&app);

    // --------------------------------------------------------
    // Parse
    // --------------------------------------------------------
    CLI11_PARSE(app, argc, argv);

    // --------------------------------------------------------
    // --export-tree: serialize the live CLI11 object tree to JSON.
    // Pipe into generate_manpage.py or generate_adminguide.py.
    // --------------------------------------------------------
    if (export_tree) {
        std::cout << "[\n" << tree_to_json(&app) << "\n]\n";
        return 0;
    }

    // --------------------------------------------------------
    // Command dispatch
    // In real radosgw-admin this calls into RGW internals.
    // Here we print what would happen to confirm dispatch works.
    // --------------------------------------------------------

    if (*user_create) {
        std::cout << "[user create] uid=" << user_opts.uid
                  << " display-name=\"" << user_opts.display_name << "\""
                  << (user_opts.email.empty() ? "" : " email=" + user_opts.email)
                  << (user_opts.admin ? " [ADMIN]" : "")
                  // std::optional: only show max-buckets if user explicitly provided it
                  << (user_opts.max_buckets.has_value() ?
                        " max-buckets=" + std::to_string(*user_opts.max_buckets) : "")
                  << (global_opts.tenant.empty() ? "" : " tenant=" + global_opts.tenant)
                  << std::endl;
        return 0;
    }
    if (*user_info) {
        std::cout << "[user info] uid=" << user_opts.uid
                  << (global_opts.format.empty() ? "" : " format=" + global_opts.format)
                  << std::endl;
        return 0;
    }
    if (*user_modify) {
        std::cout << "[user modify] uid=" << user_opts.uid << std::endl;
        return 0;
    }
    if (*user_rm) {
        std::cout << "[user rm] uid=" << user_opts.uid
                  << (purge_data ? " --purge-data" : "") << std::endl;
        return 0;
    }
    if (*user_list) {
        std::cout << "[user list]"
                  << (global_opts.format.empty() ? "" : " format=" + global_opts.format)
                  << std::endl;
        return 0;
    }
    if (*user_suspend) {
        std::cout << "[user suspend] uid=" << user_opts.uid << std::endl;
        return 0;
    }
    if (*user_enable) {
        std::cout << "[user enable] uid=" << user_opts.uid << std::endl;
        return 0;
    }
    if (*user_stats) {
        std::cout << "[user stats] uid=" << user_opts.uid
                  << (sync_stats ? " (syncing)" : "") << std::endl;
        return 0;
    }
    if (*bucket_list) {
        std::cout << "[bucket list]"
                  << (bucket_opts.bucket_name.empty() ? " all buckets"
                                                      : " bucket=" + bucket_opts.bucket_name)
                  << (bucket_opts.allow_unordered ? " (unordered)" : "")
                  << (global_opts.format.empty() ? "" : " format=" + global_opts.format)
                  << std::endl;
        return 0;
    }
    if (*bucket_stats) {
        std::cout << "[bucket stats] bucket=" << bucket_opts.bucket_name << std::endl;
        return 0;
    }
    if (*bucket_rm) {
        std::cout << "[bucket rm] bucket=" << bucket_opts.bucket_name
                  << (bucket_opts.purge_objects ? " --purge-objects" : "") << std::endl;
        return 0;
    }
    if (*bucket_link) {
        std::cout << "[bucket link] bucket=" << bucket_opts.bucket_name
                  << " -> uid=" << user_opts.uid << std::endl;
        return 0;
    }
    if (*bucket_unlink) {
        std::cout << "[bucket unlink] bucket=" << bucket_opts.bucket_name
                  << " from uid=" << user_opts.uid << std::endl;
        return 0;
    }
    if (*bucket_check) {
        std::cout << "[bucket check]"
                  << (bucket_opts.bucket_name.empty() ? " all buckets"
                                                      : " bucket=" + bucket_opts.bucket_name)
                  << (fix_index ? " --fix" : "") << std::endl;
        return 0;
    }
    if (*logging_flush) {
        std::cout << "[bucket logging flush] bucket=" << source_bucket << std::endl;
        return 0;
    }
    if (*logging_info) {
        std::cout << "[bucket logging info]"
                  << (source_bucket.empty() ? "" : " bucket=" + source_bucket)
                  << std::endl;
        return 0;
    }
    if (*logging_list) {
        std::cout << "[bucket logging list] bucket=" << source_bucket << std::endl;
        return 0;
    }
    if (*quota_set) {
        std::cout << "[quota set] scope=" << quota_scope
                  << " max-objects=" << max_objects
                  << " max-size=" << (max_size.empty() ? "unlimited" : max_size)
                  << std::endl;
        return 0;
    }
    if (*quota_enable) {
        std::cout << "[quota enable] scope=" << quota_scope
                  << " uid=" << user_opts.uid << std::endl;
        return 0;
    }
    if (*quota_disable) {
        std::cout << "[quota disable] scope=" << quota_scope
                  << " uid=" << user_opts.uid << std::endl;
        return 0;
    }
    if (*ratelimit_set) {
        std::cout << "[ratelimit set] scope=" << quota_scope
                  << " max-read-ops="    << ratelimit_opts.max_read_ops
                  << " max-write-ops="   << ratelimit_opts.max_write_ops
                  << " max-read-bytes="  << ratelimit_opts.max_read_bytes
                  << " max-write-bytes=" << ratelimit_opts.max_write_bytes
                  << std::endl;
        return 0;
    }
    if (*ratelimit_get) {
        std::cout << "[ratelimit get] scope=" << quota_scope << std::endl;
        return 0;
    }
    if (*ratelimit_enable) {
        std::cout << "[ratelimit enable] scope=" << quota_scope << std::endl;
        return 0;
    }
    if (*ratelimit_disable) {
        std::cout << "[ratelimit disable] scope=" << quota_scope << std::endl;
        return 0;
    }

    // No subcommand given — show help
    std::cout << app.help() << std::endl;
    return 0;
}
