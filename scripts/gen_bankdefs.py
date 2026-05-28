#!/usr/bin/env python3
"""Generate C++ wrapper headers from coatjava bank definition JSON files.

Reads every .json file in data/bankdefs/hipo4/ and emits one C++ header per
bank into clas12/include/clas12/bankdefs/. Each generated header declares a
class that wraps a hipo::bank_view with type-safe, pre-resolved column
handles so per-field access never hits a string lookup at runtime.

Source of truth:
    https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4

Usage:
    python3 scripts/gen_bankdefs.py

The coatjava JSON files are checked in under data/bankdefs/hipo4/. To refresh
them, run scripts/fetch_bankdefs.sh (or re-download from the URL above).

Design:
- One generated class per bank (e.g. REC_Particle for REC::Particle).
- bind(const hipo::bank_view&) resolves every column name to a column_handle
  exactly once; subsequent per-row access goes through the handle.
- A nested Row view offers a nicer field-accessor syntax:
      auto p = particle.row(i);
      int pid = p.pid();
      float px = p.px();
- Integer columns return `int` regardless of storage type (B/S/I) so callers
  don't have to care that 'pindex' is an int16_t on wire. Float columns
  return float or double as declared.
- Pindex-style columns (name matches pindex/index/hbindex/windex/windex_*)
  are additionally exposed via a column_handle so PindexLinker can read
  the entire column as a typed span in one shot — no per-row string lookup.
- Whole-column span accessors: bank.pid_col() -> std::span<const int32_t>.
"""

import json
import pathlib
import re
import sys


# =============================================================================
# HIPO type mapping
# =============================================================================

# HIPO type code -> (C++ storage type, wider public return type, span element)
#
# public return type: for integer columns we widen to `int` so callers see a
# consistent signed 32-bit type. For floats we keep float/double distinct.
TYPE_MAP = {
    "B": {"storage": "int8_t",  "public": "int",     "getter": "getByte",   "kind": "int"},
    "S": {"storage": "int16_t", "public": "int",     "getter": "getShort",  "kind": "int"},
    "I": {"storage": "int32_t", "public": "int",     "getter": "getInt",    "kind": "int"},
    "L": {"storage": "int64_t", "public": "int64_t", "getter": "getLong",   "kind": "int"},
    "F": {"storage": "float",   "public": "float",   "getter": "getFloat",  "kind": "float"},
    "D": {"storage": "double",  "public": "double",  "getter": "getDouble", "kind": "float"},
}

# C++ reserved / problematic names that can appear in HIPO columns. Append
# a trailing underscore to the getter/member if we ever hit one.
CPP_KEYWORDS = {
    "class", "struct", "union", "enum", "namespace", "typename", "template",
    "auto", "int", "float", "double", "short", "long", "char", "bool", "void",
    "register", "static", "const", "volatile", "extern", "inline", "return",
    "new", "delete", "this", "operator", "friend", "private", "public",
    "protected", "virtual", "override", "final", "for", "while", "do", "if",
    "else", "switch", "case", "default", "break", "continue", "try", "catch",
    "throw", "goto", "typedef", "using", "nullptr", "true", "false",
}

# Columns that look like "row index into another bank". Matched case-sensitively
# against the column name. This list is conservative — any column matching is
# exposed with a dedicated column_handle and reported as a link candidate.
LINK_COLUMN_PATTERNS = [
    re.compile(r"^pindex$"),         # row into REC::Particle
    re.compile(r"^index$"),          # row into the detector hit bank
    re.compile(r"^hbindex$"),        # row into another track bank
    re.compile(r"^[a-z]index$"),     # uindex, windex, etc.
    re.compile(r"^mcindex$"),        # row into MC::Particle
    re.compile(r"^.index$"),         # rindex
]

# Banks that should be included by the umbrella "analysis_banks.hpp" header.
# Everything else is opt-in to keep compile time down for analyses that only
# use the high-level banks.
UMBRELLA_BANKS = {
    # Physics event header / particle
    "REC::Event", "REC::Particle",
    "RECHB::Event", "RECHB::Particle",
    "RECAI::Event", "RECAI::Particle",
    "RECHBAI::Event", "RECHBAI::Particle",
    # Detector responses linked to particles
    "REC::Calorimeter", "REC::CaloExtras",
    "REC::Cherenkov",
    "REC::Scintillator", "REC::ScintExtras",
    "REC::ForwardTagger",
    "REC::Track", "REC::Traj",
    # MC
    "MC::Particle", "MC::Lund", "MC::GenMatch", "MC::RecMatch",
    # Run config / trigger
    "RUN::config",
    # Decays
    "DECAYS::Particle",
}


# =============================================================================
# Physics-view configuration
# =============================================================================
#
# Declarative per-bank rules for the denormalized physics-view readers that
# algorithms consume (e.g. `clas12::CalorimeterBank` + `read_Calorimeter_bank`).
# The generator expands each entry into:
#   * a flat POD struct named `view_name` with one field per bank column,
#   * an inline `read_<view_name>(Event&, int pindex)` function implementing
#     the row-selection strategy.
#
# Three strategies cover every physics view currently in clas12/Banks.hpp:
#
#   first_matching : first linked row where `filter_col == filter_val` wins.
#                    Output: flat struct mirroring the bank.
#
#   best_by_priority : among rows passing the filter, pick the one whose
#                      `priority_col` value comes first in the priority list.
#                      Bank-order breaks ties. Optional `extras` bank is
#                      merged 1:1 via row_index.
#
#   dispatched : every row passing the filter is bucketed by `dispatch_col`
#                into a prefix group. Fields from each bucket are written
#                with that group's prefix. Optional `extras` bank merged
#                per-group via row_index.
#
# Filter / priority / dispatch values are emitted verbatim into the generated
# C++ as identifiers, so they must name constants from
# clas12/include/clas12/Detectors.hpp (or any header the generated umbrella
# transitively includes).
PHYSICS_VIEWS = {
    "REC::Calorimeter": {
        "view_name": "CalorimeterBank",
        "strategy":  "dispatched",
        "filter":    ("detector", "ECAL"),
        "dispatch":  ("layer", [
            ("PCAL",  "pcal_"),
            ("ECIN",  "inner_"),
            ("ECOUT", "outer_"),
        ]),
        "extras":    "REC::CaloExtras",
    },
    "REC::Traj": {
        "view_name": "DCTrajBank",
        "read_name": "read_Traj_bank",  # legacy symbol predates the DC prefix
        "strategy":  "dispatched",
        "filter":    ("detector", "DC"),
        "dispatch":  ("layer", [
            ("DC1", "region1_"),
            ("DC3", "region2_"),
            ("DC6", "region3_"),
        ]),
    },
    "REC::Cherenkov": {
        "view_name": "CherenkovBank",
        "strategy":  "first_matching",
        "filter":    ("detector", "HTCC"),
    },
    "REC::Track": {
        "view_name": "TrackBank",
        "strategy":  "first_matching",
        "filter":    ("detector", "DC"),
    },
    "REC::Scintillator": {
        "view_name": "ScintillatorBank",
        "strategy":  "best_by_priority",
        "filter":    ("detector", "FTOF"),
        "priority":  ("layer", ["FTOF1B", "FTOF1A", "FTOF2"]),
        "extras":    "REC::ScintExtras",
    },
    "RICH::Particle": {
        "view_name": "RICHParticleBank",
        "strategy":  "first_row",
    },
}


# =============================================================================
# Name helpers
# =============================================================================

def sanitize_member(name: str) -> str:
    """Map a column name to a valid C++ identifier."""
    out = re.sub(r"[^A-Za-z0-9_]", "_", name)
    if out and out[0].isdigit():
        out = "_" + out
    if out in CPP_KEYWORDS:
        out = out + "_"
    return out


def bank_to_class_name(bank_name: str) -> str:
    """REC::Particle -> REC_Particle, ALERT::ai:projections -> ALERT_ai_projections.

    Any character that isn't a valid C++ identifier character is replaced
    with '_'. Collapses consecutive underscores so ``ALERT::ai`` becomes
    ``ALERT_ai`` instead of ``ALERT__ai``.
    """
    out = re.sub(r"[^A-Za-z0-9_]", "_", bank_name)
    out = re.sub(r"_+", "_", out)
    return out.strip("_")


def bank_to_filename(bank_name: str) -> str:
    """REC::Particle -> REC_Particle.hpp."""
    return bank_to_class_name(bank_name) + ".hpp"


def is_link_column(col_name: str) -> bool:
    return any(p.match(col_name) for p in LINK_COLUMN_PATTERNS)


# =============================================================================
# Code generation
# =============================================================================

HEADER_TEMPLATE = """\
// =============================================================================
// Auto-generated from {source} - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {{

"""

FOOTER = "}  // namespace bankdefs\n"


def emit_column_info(entries):
    """Build per-column metadata lists used by the template."""
    cols = []
    for e in entries:
        raw = e["name"]
        tcode = e["type"]
        if tcode not in TYPE_MAP:
            raise ValueError(f"unknown type code {tcode!r} for column {raw!r}")
        t = TYPE_MAP[tcode]
        cols.append({
            "raw_name": raw,
            "cxx_name": sanitize_member(raw),
            "storage": t["storage"],
            "public":  t["public"],
            "getter":  t["getter"],
            "kind":    t["kind"],
            "info":    e.get("info", "").replace('"', r'\"'),
            "is_link": is_link_column(raw),
        })
    return cols


def gen_header(bank_def: dict, source_file: str) -> str:
    """Emit the full C++ header text for one bank."""
    name = bank_def["name"]
    group = bank_def["group"]
    item = bank_def["item"]
    info = bank_def.get("info", "").replace('"', r'\"')
    cols = emit_column_info(bank_def["entries"])
    cls = bank_to_class_name(name)

    lines = []
    lines.append(HEADER_TEMPLATE.format(source=source_file))
    lines.append(f"/// HIPO bank wrapper for `{name}` (group {group}, item {item}).")
    if info:
        lines.append(f"/// {info}")
    lines.append("///")
    lines.append("/// Bind the wrapper to a `hipo::bank` once, then access columns")
    lines.append("/// via cached item-index getters (no per-call name lookup).")
    lines.append("///")
    lines.append("/// Example:")
    lines.append("/// @code")
    lines.append(f"/// {cls} bank;")
    lines.append(f"/// if (auto sc = bank.bind(event.get_bank(\"{name}\")); !sc) return FAILURE();")
    lines.append("/// for (int i = 0; i < bank.rows(); ++i) {")
    lines.append("///     auto row = bank.row(i);")
    if cols:
        example_col = cols[0]["cxx_name"]
        lines.append(f"///     auto {example_col} = row.{example_col}();")
    lines.append("/// }")
    lines.append("/// @endcode")
    lines.append(f"class {cls} {{")
    lines.append(" public:")
    lines.append(f"    static constexpr std::string_view bank_name = \"{name}\";")
    lines.append(f"    static constexpr int bank_group = {group};")
    lines.append(f"    static constexpr int bank_item  = {item};")
    lines.append("")
    lines.append("    class Row;")
    lines.append("")
    lines.append(f"    {cls}() = default;")
    lines.append("")
    lines.append("    /// Bind to a live hipo::bank. Per-thread cache keyed by the schema's")
    lines.append("    /// (group, item): the first bind() in a thread resolves every column")
    lines.append("    /// name to its item index via schema.getEntryOrder(); subsequent")
    lines.append("    /// bind()s hit the cache. Keying by (group,item) rather than the schema")
    lines.append("    /// POINTER is critical: the framework constructs a fresh hipo::bank")
    lines.append("    /// (copying its schema to a new address) every event, so a pointer key")
    lines.append("    /// would miss every event and re-resolve all columns by string. (group,")
    lines.append("    /// item) is stable across those copies, so resolution is paid once per")
    lines.append("    /// thread per bank type. Returns true on success.")
    lines.append("    bool bind(const hipo::bank& view) noexcept {")
    lines.append("        m_view = &view;")
    lines.append("        // const_cast is OK: hipo::bank::getSchema() lacks a const overload")
    lines.append("        // but only reads schemaEntriesMap; we never mutate the bank here.")
    lines.append("        auto& schema = const_cast<hipo::bank&>(view).getSchema();")
    lines.append("        struct ItemCache {")
    lines.append("            int gi_group = -1;")
    lines.append("            int gi_item  = -2;")
    for c in cols:
        lines.append(f"            int i_{c['cxx_name']} = -1;")
    lines.append("        };")
    lines.append("        static thread_local ItemCache cache;")
    lines.append("        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {")
    lines.append("            cache.gi_group = schema.getGroup();")
    lines.append("            cache.gi_item  = schema.getItem();")
    for c in cols:
        lines.append(f"            cache.i_{c['cxx_name']} = schema.exists(\"{c['raw_name']}\") ? schema.getEntryOrder(\"{c['raw_name']}\") : -1;")
    lines.append("        }")
    for c in cols:
        lines.append(f"        i_{c['cxx_name']} = cache.i_{c['cxx_name']};")
    lines.append("        return true;")
    lines.append("    }")
    lines.append("")
    lines.append("    /// Current row count of the bound bank (0 if unbound).")
    lines.append("    [[nodiscard]] int rows() const noexcept { return m_view ? m_view->getRows() : 0; }")
    lines.append("")
    lines.append("    /// True if bind() has been called.")
    lines.append("    [[nodiscard]] bool valid() const noexcept { return m_view != nullptr; }")
    lines.append("")
    lines.append("    /// Access the underlying hipo::bank (for advanced use).")
    lines.append("    [[nodiscard]] const hipo::bank& view() const noexcept { return *m_view; }")
    lines.append("")

    # Scalar getters
    lines.append("    // -------------------------------------------------------------------")
    lines.append("    // Per-row scalar getters (item-index based, no string lookup)")
    lines.append("    // -------------------------------------------------------------------")
    for c in cols:
        lines.append(f"    /// {c['raw_name']}: {c['info']}")
        lines.append(f"    [[nodiscard]] {c['public']} {c['cxx_name']}(int row) const noexcept {{")
        lines.append(f"        return static_cast<{c['public']}>(m_view->{c['getter']}(i_{c['cxx_name']}, row));")
        lines.append("    }")
    lines.append("")

    # Row helper
    lines.append("    /// Construct a lightweight Row view for the given index.")
    lines.append("    [[nodiscard]] Row row(int index) const noexcept;")
    lines.append("")

    # Link column item indices (public for PindexLinker-style helpers)
    link_cols = [c for c in cols if c["is_link"]]
    if link_cols:
        lines.append("    // -------------------------------------------------------------------")
        lines.append("    // Link-column item indices (used by PindexLinker to scan the")
        lines.append("    // pindex/index column without re-resolving by name per call)")
        lines.append("    // -------------------------------------------------------------------")
        for c in link_cols:
            lines.append(f"    [[nodiscard]] int {c['cxx_name']}_handle() const noexcept {{ return i_{c['cxx_name']}; }}")
        lines.append("")

    lines.append(" private:")
    lines.append("    const hipo::bank* m_view = nullptr;")
    for c in cols:
        lines.append(f"    int i_{c['cxx_name']} = -1;")
    lines.append("")
    lines.append("    friend class Row;")
    lines.append("};")
    lines.append("")

    # Row view class
    lines.append(f"/// Lightweight row view for {name}. Stores a pointer + row index only.")
    lines.append(f"class {cls}::Row {{")
    lines.append(" public:")
    lines.append(f"    Row(const {cls}* bank, int row) noexcept : m_bank(bank), m_row(row) {{}}")
    lines.append("    [[nodiscard]] int row_index() const noexcept { return m_row; }")
    for c in cols:
        lines.append(f"    [[nodiscard]] {c['public']} {c['cxx_name']}() const noexcept {{ return m_bank->{c['cxx_name']}(m_row); }}")
    lines.append(" private:")
    lines.append(f"    const {cls}* m_bank;")
    lines.append("    int m_row;")
    lines.append("};")
    lines.append("")

    # Out-of-line row() definition
    lines.append(f"inline {cls}::Row {cls}::row(int index) const noexcept {{")
    lines.append("    return Row{this, index};")
    lines.append("}")
    lines.append("")

    lines.append(FOOTER)
    return "\n".join(lines)


# =============================================================================
# Generated readers (physics-view structs + generic pindex row readers)
# =============================================================================

def has_pindex(bank_def: dict) -> bool:
    return any(e["name"] == "pindex" for e in bank_def["entries"])


def has_index(bank_def: dict) -> bool:
    return any(e["name"] == "index" for e in bank_def["entries"])


def default_init(public_type: str) -> str:
    """POD field default: NaN for floats, zero for integers."""
    if public_type in ("float", "double"):
        return f"std::numeric_limits<{public_type}>::quiet_NaN()"
    return "0"


def gen_row_struct_and_reader(bank_def: dict) -> list[str]:
    """Emit a flat row struct + one or two vector readers for a linked bank.

    Every bank exposing a ``pindex`` column gets a
    ``read_<Bank>_rows(Event&, int pindex)`` helper keyed via the
    framework's pindex linker. Banks that also expose an ``index`` column
    additionally get ``read_<Bank>_rows_by_index(Event&, int key)`` keyed
    via the framework's index linker - useful for sibling-row joins where
    one bank stores row indices pointing at another.
    """
    name = bank_def["name"]
    cls  = bank_to_class_name(name)
    cols = emit_column_info(bank_def["entries"])

    struct_name = f"{cls}Row"
    has_pin     = has_pindex(bank_def)
    has_idx     = has_index(bank_def)

    lines: list[str] = []
    lines.append(f"// ---- {name} ".ljust(78, "-"))
    lines.append(f"/// Flat POD row mirroring every column of `{name}`.")
    lines.append(f"struct {struct_name} {{")
    for c in cols:
        lines.append(f"    {c['public']} {c['cxx_name']} = {default_init(c['public'])};")
    lines.append("    int row_index = -1;")
    lines.append("};")
    lines.append("")

    def emit_reader(fn_suffix: str, key_name: str, rows_call: str, doc: str):
        fn_name = f"read_{cls}_rows{fn_suffix}"
        lines.append(f"/// {doc}")
        lines.append(f"inline auto {fn_name}(framework::Event& event, int {key_name})")
        lines.append(f"    -> std::vector<{struct_name}> {{")
        lines.append(f"    std::vector<{struct_name}> result;")
        lines.append(f"    for (auto r : event.{rows_call}<bankdefs::{cls}>({key_name})) {{")
        lines.append(f"        {struct_name} s;")
        for c in cols:
            lines.append(f"        s.{c['cxx_name']} = r.{c['cxx_name']}();")
        lines.append("        s.row_index = r.row_index();")
        lines.append("        result.push_back(s);")
        lines.append("    }")
        lines.append("    return result;")
        lines.append("}")
        lines.append("")

    if has_pin:
        emit_reader("",
                    "pindex",
                    "rows_for_pindex",
                    f"Read every `{name}` row whose pindex column equals `pindex`.")
    if has_idx:
        emit_reader("_by_index",
                    "key",
                    "rows_for_index",
                    f"Read every `{name}` row whose index column equals `key`.")

    return lines


def _view_struct_fields(cols, prefix: str = "") -> list[str]:
    """Emit `<type> <prefix><col> = <default>;` lines for every column."""
    out = []
    for c in cols:
        out.append(f"    {c['public']} {prefix}{c['cxx_name']} = {default_init(c['public'])};")
    return out


def _assign_columns(target_var: str, row_var: str, cols, prefix: str = "",
                    indent: int = 8) -> list[str]:
    """Emit `<target>.<prefix><col> = <row>.<col>();` lines for every column."""
    pad = " " * indent
    out = []
    for c in cols:
        out.append(f"{pad}{target_var}.{prefix}{c['cxx_name']} = {row_var}.{c['cxx_name']}();")
    return out


def gen_physics_view(bank_def: dict, cfg: dict, banks_by_name: dict) -> list[str]:
    """Emit the flat physics-view struct + reader for one PHYSICS_VIEWS entry."""
    bank_name  = bank_def["name"]
    view_name  = cfg["view_name"]
    strategy   = cfg["strategy"]
    cls        = bank_to_class_name(bank_name)
    cols       = emit_column_info(bank_def["entries"])

    extras_bank = cfg.get("extras")
    extras_cols = []
    extras_cls  = None
    if extras_bank:
        if extras_bank not in banks_by_name:
            raise ValueError(f"PHYSICS_VIEWS[{bank_name!r}] references missing extras bank {extras_bank!r}")
        extras_def = banks_by_name[extras_bank]
        extras_cols = emit_column_info(extras_def["entries"])
        extras_cls  = bank_to_class_name(extras_bank)

    # Validate referenced columns against the bank schema.
    col_names = {c["cxx_name"] for c in cols}
    for cfg_key in ("filter", "priority", "dispatch"):
        if cfg_key in cfg:
            col = cfg[cfg_key][0]
            if col not in col_names:
                raise ValueError(
                    f"PHYSICS_VIEWS[{bank_name!r}] {cfg_key} references column {col!r} "
                    f"not in {bank_name}")

    lines: list[str] = []
    lines.append(f"// ==== {bank_name} -> {view_name} ".ljust(78, "="))

    # -- struct ---------------------------------------------------------------
    lines.append(f"/// Physics view of `{bank_name}` (generated from PHYSICS_VIEWS).")
    lines.append(f"struct {view_name} {{")
    if strategy == "dispatched":
        for _, prefix in cfg["dispatch"][1]:
            lines.extend(_view_struct_fields(cols, prefix))
            if extras_cols:
                lines.extend(_view_struct_fields(extras_cols, prefix))
    else:  # first_matching, best_by_priority, first_row
        lines.extend(_view_struct_fields(cols))
        if extras_cols:
            lines.extend(_view_struct_fields(extras_cols))
    lines.append("};")
    lines.append("")

    # -- reader ---------------------------------------------------------------
    # The reader name matches the legacy hand-written symbol so call sites
    # (read_Calorimeter_bank, read_Scintillator_bank, ...) don't need to move.
    fn_name = cfg.get("read_name")
    if not fn_name:
        legacy_suffix = view_name.removesuffix("Bank")
        fn_name = f"read_{legacy_suffix}_bank"
    lines.append(f"/// Strategy: {strategy}. See PHYSICS_VIEWS in scripts/gen_bankdefs.py.")
    lines.append(f"inline auto {fn_name}(framework::Event& event, int pindex)")
    lines.append(f"    -> {view_name} {{")
    lines.append(f"    {view_name} result;")

    # All strategies use a single forward scan over rows matching `pindex`
    # (event.for_rows_with_pindex / find_row_with_pindex) instead of building a
    # PindexLinker — for per-particle access (one pindex/event) the linker's
    # 3-pass build is pure overhead. Selection semantics are preserved exactly:
    # `continue` -> `return` (void lambda) and the first_* break -> `return true`.
    if strategy == "first_matching":
        f_col, f_val = cfg["filter"]
        lines.append(f"    event.find_row_with_pindex<bankdefs::{cls}>(pindex, [&](auto r) -> bool {{")
        lines.append(f"        if (r.{f_col}() != {f_val}) return false;")
        lines.extend(_assign_columns("result", "r", cols))
        lines.append("        return true;")
        lines.append("    });")

    elif strategy == "first_row":
        lines.append(f"    event.find_row_with_pindex<bankdefs::{cls}>(pindex, [&](auto r) -> bool {{")
        lines.extend(_assign_columns("result", "r", cols))
        lines.append("        return true;")
        lines.append("    });")

    elif strategy == "best_by_priority":
        f_col, f_val       = cfg["filter"]
        p_col, p_values    = cfg["priority"]
        lines.append(f"    std::optional<bankdefs::{cls}::Row> best;")
        lines.append("    int best_rank = -1;")
        lines.append(f"    event.for_rows_with_pindex<bankdefs::{cls}>(pindex, [&](auto r) {{")
        lines.append(f"        if (r.{f_col}() != {f_val}) return;")
        lines.append("        int rank;")
        lines.append(f"        switch (r.{p_col}()) {{")
        for i, v in enumerate(p_values):
            lines.append(f"            case {v}: rank = {i}; break;")
        lines.append("            default: return;")
        lines.append("        }")
        lines.append("        if (best_rank == -1 || rank < best_rank) {")
        lines.append("            best = r;")
        lines.append("            best_rank = rank;")
        lines.append("        }")
        lines.append("    });")
        lines.append("    if (!best.has_value()) return result;")
        lines.extend(_assign_columns("result", "(*best)", cols, indent=4))
        if extras_bank:
            lines.append(f"    auto& extras = event.bank<bankdefs::{extras_cls}>();")
            lines.append("    const int best_row = best->row_index();")
            lines.append("    if (extras.valid() && best_row < extras.rows()) {")
            lines.append("        auto er = extras.row(best_row);")
            lines.extend(_assign_columns("result", "er", extras_cols, indent=8))
            lines.append("    }")

    elif strategy == "dispatched":
        f_col, f_val     = cfg["filter"]
        d_col, d_table   = cfg["dispatch"]
        if extras_bank:
            lines.append(f"    auto& extras = event.bank<bankdefs::{extras_cls}>();")
            lines.append("    const bool has_extras = extras.valid();")
            lines.append("    const int extras_rows = extras.rows();")
        lines.append(f"    event.for_rows_with_pindex<bankdefs::{cls}>(pindex, [&](auto r) {{")
        lines.append(f"        if (r.{f_col}() != {f_val}) return;")
        lines.append(f"        switch (r.{d_col}()) {{")
        for val, prefix in d_table:
            lines.append(f"            case {val}: {{")
            for c in cols:
                lines.append(f"                result.{prefix}{c['cxx_name']} = r.{c['cxx_name']}();")
            if extras_bank:
                lines.append("                const int row_idx = r.row_index();")
                lines.append("                if (has_extras && row_idx < extras_rows) {")
                lines.append("                    auto er = extras.row(row_idx);")
                for c in extras_cols:
                    lines.append(f"                    result.{prefix}{c['cxx_name']} = er.{c['cxx_name']}();")
                lines.append("                }")
            lines.append("                break;")
            lines.append("            }")
        lines.append("            default: break;")
        lines.append("        }")
        lines.append("    });")

    else:
        raise ValueError(f"unknown strategy {strategy!r} in PHYSICS_VIEWS[{bank_name!r}]")

    lines.append("    return result;")
    lines.append("}")
    lines.append("")
    return lines


def gen_generated_readers_header(all_bank_defs: list[dict], banks_by_name: dict) -> str:
    """Assemble the full text of clas12/bankdefs/generated_readers.hpp."""
    linkable_banks = [b for b in all_bank_defs if has_pindex(b) or has_index(b)]

    lines: list[str] = []
    lines.append("// =============================================================================")
    lines.append("// Auto-generated by scripts/gen_bankdefs.py - do not edit by hand.")
    lines.append("//")
    lines.append("// Contents:")
    lines.append("//   1. `bankdefs::<Bank>Row` + `read_<Bank>_rows(Event&, int pindex)`")
    lines.append("//      for every bank that has a `pindex` column, and additionally")
    lines.append("//      `read_<Bank>_rows_by_index(Event&, int key)` for every bank that has")
    lines.append("//      an `index` column (for sibling-row joins like FMT::Tracks.index ->")
    lines.append("//      REC::Track row).")
    lines.append("//   2. `clas12::<View>Bank` + `read_<View>_bank(Event&, int pindex)` for each")
    lines.append("//      entry in PHYSICS_VIEWS - denormalized physics-view structs driven by")
    lines.append("//      the declarative strategy config (first_matching / best_by_priority /")
    lines.append("//      dispatched / first_row).")
    lines.append("// =============================================================================")
    lines.append("")
    lines.append("#pragma once")
    lines.append("")
    lines.append("#include <limits>")
    lines.append("#include <optional>")
    lines.append("#include <vector>")
    lines.append("")
    lines.append("#include <clas12/Detectors.hpp>")
    lines.append("#include <clas12/framework/core/Event.hpp>")
    lines.append("")
    # Include every linkable wrapper header (covers both pindex and index
    # banks; sets make sure there are no duplicate #includes).
    for b in linkable_banks:
        lines.append(f"#include <clas12/bankdefs/{bank_to_filename(b['name'])}>")
    # Include every sibling extras bank referenced by PHYSICS_VIEWS that isn't
    # itself in the linkable set (e.g. REC::CaloExtras has no pindex column).
    linkable_names = {b["name"] for b in linkable_banks}
    extras_banks = {cfg["extras"] for cfg in PHYSICS_VIEWS.values() if "extras" in cfg}
    for extras in sorted(extras_banks):
        if extras not in linkable_names:
            if extras not in banks_by_name:
                raise ValueError(f"PHYSICS_VIEWS references unknown extras bank {extras!r}")
            lines.append(f"#include <clas12/bankdefs/{bank_to_filename(extras)}>")
    lines.append("")

    # ---- generic row structs + readers (bankdefs) -------------------
    lines.append("namespace bankdefs {")
    lines.append("")
    for b in linkable_banks:
        lines.extend(gen_row_struct_and_reader(b))
    lines.append("}  // namespace bankdefs")
    lines.append("")

    # ---- physics views (global scope to match the legacy Banks.hpp shim) ----
    lines.append("")
    for bank_name, cfg in PHYSICS_VIEWS.items():
        if bank_name not in banks_by_name:
            raise ValueError(f"PHYSICS_VIEWS references unknown bank {bank_name!r}")
        lines.extend(gen_physics_view(banks_by_name[bank_name], cfg, banks_by_name))
    lines.append("")
    return "\n".join(lines)


def gen_umbrella(all_class_infos: list[dict]) -> str:
    """Emit an umbrella header that pulls in the common analysis banks."""
    lines = []
    lines.append("// =============================================================================")
    lines.append("// Auto-generated umbrella header - do not edit by hand.")
    lines.append("// Pulls in the bank wrappers that most CLAS12 analyses touch.")
    lines.append("// Detector-level raw hit banks are available as individual headers under")
    lines.append("// clas12/bankdefs/ - include them explicitly if you need them.")
    lines.append("// =============================================================================")
    lines.append("#pragma once")
    lines.append("")
    for info in all_class_infos:
        if info["bank_name"] in UMBRELLA_BANKS:
            lines.append(f"#include <clas12/bankdefs/{info['filename']}>")
    lines.append("")
    return "\n".join(lines)


def gen_index(all_class_infos: list[dict]) -> str:
    """Emit a header that declares every generated wrapper - one include line
    per bank.  Only for tooling that wants 'everything'."""
    lines = []
    lines.append("// =============================================================================")
    lines.append("// Auto-generated: include every generated bank wrapper.")
    lines.append("// This is heavy - only use it if you genuinely need every bank type.")
    lines.append("// =============================================================================")
    lines.append("#pragma once")
    lines.append("")
    for info in all_class_infos:
        lines.append(f"#include <clas12/bankdefs/{info['filename']}>")
    lines.append("")
    return "\n".join(lines)


# =============================================================================
# Main
# =============================================================================

def main() -> int:
    repo_root = pathlib.Path(__file__).resolve().parent.parent
    defs_dir = repo_root / "data" / "bankdefs" / "hipo4"
    out_dir = repo_root / "clas12" / "include" / "clas12" / "bankdefs"

    if not defs_dir.exists():
        print(f"error: {defs_dir} does not exist", file=sys.stderr)
        return 1

    out_dir.mkdir(parents=True, exist_ok=True)

    json_files = sorted(defs_dir.glob("*.json"))
    if not json_files:
        print(f"error: no JSON files in {defs_dir}", file=sys.stderr)
        return 1

    all_class_infos: list[dict] = []
    all_bank_defs: list[dict] = []
    banks_by_name: dict[str, dict] = {}
    seen_names: set[str] = set()
    total_cols = 0
    total_links = 0

    for jf in json_files:
        with open(jf) as f:
            defs = json.load(f)

        for bank_def in defs:
            name = bank_def["name"]
            if name in seen_names:
                print(f"warning: duplicate bank {name!r} in {jf.name}", file=sys.stderr)
                continue
            seen_names.add(name)

            cls = bank_to_class_name(name)
            filename = bank_to_filename(name)
            text = gen_header(bank_def, jf.name)
            (out_dir / filename).write_text(text)

            n_cols = len(bank_def["entries"])
            n_links = sum(1 for e in bank_def["entries"] if is_link_column(e["name"]))
            total_cols += n_cols
            total_links += n_links

            all_bank_defs.append(bank_def)
            banks_by_name[name] = bank_def
            all_class_infos.append({
                "bank_name": name,
                "class_name": cls,
                "filename": filename,
                "n_cols": n_cols,
                "n_links": n_links,
            })

    (out_dir / "analysis_banks.hpp").write_text(gen_umbrella(all_class_infos))
    (out_dir / "all_banks.hpp").write_text(gen_index(all_class_infos))

    readers_text = gen_generated_readers_header(all_bank_defs, banks_by_name)
    (out_dir / "generated_readers.hpp").write_text(readers_text)
    n_pindex = sum(1 for b in all_bank_defs if has_pindex(b))
    n_index  = sum(1 for b in all_bank_defs if has_index(b))
    n_linkable = sum(1 for b in all_bank_defs if has_pindex(b) or has_index(b))

    print(f"Generated {len(all_class_infos)} bank wrappers ({total_cols} columns, "
          f"{total_links} link columns)")
    print(f"  -> {out_dir}")
    print(f"  umbrella: analysis_banks.hpp covers {sum(1 for i in all_class_infos if i['bank_name'] in UMBRELLA_BANKS)} banks")
    print(f"  generated_readers.hpp: {n_linkable} linkable banks "
          f"({n_pindex} pindex, {n_index} index), {len(PHYSICS_VIEWS)} physics views")
    return 0


if __name__ == "__main__":
    sys.exit(main())
