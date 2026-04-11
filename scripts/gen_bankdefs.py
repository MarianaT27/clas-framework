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
import textwrap


# =============================================================================
# HIPO type mapping
# =============================================================================

# HIPO type code -> (C++ storage type, wider public return type, span element)
#
# public return type: for integer columns we widen to `int` so callers see a
# consistent signed 32-bit type. For floats we keep float/double distinct.
TYPE_MAP = {
    "B": {"storage": "int8_t",  "public": "int",     "span": "int8_t",  "kind": "int"},
    "S": {"storage": "int16_t", "public": "int",     "span": "int16_t", "kind": "int"},
    "I": {"storage": "int32_t", "public": "int",     "span": "int32_t", "kind": "int"},
    "L": {"storage": "int64_t", "public": "int64_t", "span": "int64_t", "kind": "int"},
    "F": {"storage": "float",   "public": "float",   "span": "float",   "kind": "float"},
    "D": {"storage": "double",  "public": "double",  "span": "double",  "kind": "float"},
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
#include <span>
#include <string_view>

#include <bank_view.h>
#include <schema.h>

namespace clas12::bankdefs {{

"""

FOOTER = "}  // namespace clas12::bankdefs\n"


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
            "span":    t["span"],
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
    lines.append("/// Bind the wrapper to a `hipo::bank_view` once, then access columns")
    lines.append("/// via zero-cost handle-based getters.")
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
    lines.append("    /// Bind to a live bank_view. Resolves every column handle exactly once.")
    lines.append("    /// Returns false if any required column is missing in the schema.")
    lines.append("    bool bind(const hipo::bank_view& view) noexcept {")
    lines.append("        m_view = view;")
    lines.append("        if (!view.valid()) return false;")
    lines.append("        const auto& schema = view.get_schema();")
    for c in cols:
        lines.append(f"        if (auto h = schema.get_handle(\"{c['raw_name']}\"); h) h_{c['cxx_name']} = *h;")
        lines.append(f"        else h_{c['cxx_name']} = {{}};")
    lines.append("        return true;")
    lines.append("    }")
    lines.append("")
    lines.append("    /// Current row count of the bound bank view.")
    lines.append("    [[nodiscard]] int rows() const noexcept { return m_view.rows(); }")
    lines.append("")
    lines.append("    /// True if bind() has been called with a valid view.")
    lines.append("    [[nodiscard]] bool valid() const noexcept { return m_view.valid(); }")
    lines.append("")
    lines.append("    /// Access the underlying bank_view (for advanced use).")
    lines.append("    [[nodiscard]] const hipo::bank_view& view() const noexcept { return m_view; }")
    lines.append("")

    # Scalar getters
    lines.append("    // -------------------------------------------------------------------")
    lines.append("    // Per-row scalar getters (handle-based, zero string lookup)")
    lines.append("    // -------------------------------------------------------------------")
    for c in cols:
        lines.append(f"    /// {c['raw_name']}: {c['info']}")
        lines.append(f"    [[nodiscard]] {c['public']} {c['cxx_name']}(int row) const noexcept {{")
        lines.append(f"        return static_cast<{c['public']}>(m_view.get<{c['storage']}>(h_{c['cxx_name']}, row));")
        lines.append("    }")
    lines.append("")

    # Column span getters
    lines.append("    // -------------------------------------------------------------------")
    lines.append("    // Whole-column span accessors (SIMD-friendly, zero-copy)")
    lines.append("    // -------------------------------------------------------------------")
    for c in cols:
        lines.append(f"    /// {c['raw_name']} column as a contiguous span.")
        lines.append(f"    [[nodiscard]] std::span<const {c['span']}> {c['cxx_name']}_col() const noexcept {{")
        lines.append(f"        return m_view.column<{c['span']}>(h_{c['cxx_name']});")
        lines.append("    }")
    lines.append("")

    # Row helper
    lines.append("    /// Construct a lightweight Row view for the given index.")
    lines.append("    [[nodiscard]] Row row(int index) const noexcept;")
    lines.append("")

    # Link column handles (public for PindexLinker-style helpers)
    link_cols = [c for c in cols if c["is_link"]]
    if link_cols:
        lines.append("    // -------------------------------------------------------------------")
        lines.append("    // Link-column handles (used by PindexLinker-style helpers to")
        lines.append("    // read the whole index column as a typed span in one shot)")
        lines.append("    // -------------------------------------------------------------------")
        for c in link_cols:
            lines.append(f"    [[nodiscard]] hipo::column_handle {c['cxx_name']}_handle() const noexcept {{ return h_{c['cxx_name']}; }}")
        lines.append("")

    lines.append(" private:")
    lines.append("    hipo::bank_view m_view{};")
    for c in cols:
        lines.append(f"    hipo::column_handle h_{c['cxx_name']}{{}};")
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

            all_class_infos.append({
                "bank_name": name,
                "class_name": cls,
                "filename": filename,
                "n_cols": n_cols,
                "n_links": n_links,
            })

    (out_dir / "analysis_banks.hpp").write_text(gen_umbrella(all_class_infos))
    (out_dir / "all_banks.hpp").write_text(gen_index(all_class_infos))

    print(f"Generated {len(all_class_infos)} bank wrappers ({total_cols} columns, "
          f"{total_links} link columns)")
    print(f"  -> {out_dir}")
    print(f"  umbrella: analysis_banks.hpp covers {sum(1 for i in all_class_infos if i['bank_name'] in UMBRELLA_BANKS)} banks")
    return 0


if __name__ == "__main__":
    sys.exit(main())
