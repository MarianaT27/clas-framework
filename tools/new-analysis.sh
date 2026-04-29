#!/usr/bin/env bash
# Scaffold a new CLAS12 analysis.
#
# Creates:
#   analyses/<name>/<name>.cpp        — driver main
#   analyses/<name>/<Name>Alg.hpp     — algorithm template (with --with-alg)
#   config/<name>.toml                — stub TOML config
#
# And appends the executable() entry to analyses/meson.build.
#
# Usage:
#   tools/new-analysis.sh <name>             # driver-only (compose existing algs)
#   tools/new-analysis.sh <name> --with-alg  # driver + a custom algorithm template
#
# Run from the repo root.

set -euo pipefail

if [[ $# -lt 1 ]]; then
    echo "usage: $0 <name> [--with-alg]" >&2
    echo "  Creates a new analysis under analyses/<name>/" >&2
    exit 64
fi

name="$1"
with_alg=0
if [[ "${2:-}" == "--with-alg" ]]; then
    with_alg=1
fi

# Validate name (lowercase letters, digits, underscores; no leading digit).
if [[ ! "$name" =~ ^[a-z][a-z0-9_]*$ ]]; then
    echo "error: analysis name must be lowercase_with_underscores, got '$name'" >&2
    exit 64
fi

# CamelCase form for class names (foo_bar -> FooBar).
camel_name="$(echo "$name" | awk -F_ '{ for(i=1;i<=NF;++i) printf "%s%s", toupper(substr($i,1,1)), substr($i,2); print "" }')"

# Sanity-check: must run from repo root.
if [[ ! -f meson.build ]] || [[ ! -d analyses ]] || [[ ! -d framework ]]; then
    echo "error: run this script from the repo root (analyses/, framework/, meson.build expected)" >&2
    exit 65
fi

dir="analyses/$name"
if [[ -e "$dir" ]]; then
    echo "error: '$dir' already exists" >&2
    exit 73
fi

mkdir -p "$dir"

# ---------- driver .cpp -------------------------------------------------------
driver_cpp="$dir/$name.cpp"
if [[ "$with_alg" -eq 1 ]]; then
    cat > "$driver_cpp" <<EOF
/**
 * @file $name.cpp
 * @brief $camel_name analysis driver.
 *
 * Run with:
 *   ./$name --config config/$name.toml
 */

#include <clas12/framework/core/AnalysisRunner.hpp>
#include "${camel_name}Alg.hpp"

auto main(int argc, char* argv[]) -> int {
    return framework::AnalysisRunner::run(
        argc, argv, "$camel_name Analysis", "$name",
        [](auto& builder, const auto& /*config*/) {
            framework::add_alg<analyses::${camel_name}Alg>(builder, "${camel_name}");
        });
}
EOF
else
    cat > "$driver_cpp" <<EOF
/**
 * @file $name.cpp
 * @brief $camel_name analysis driver.
 *
 * Composes existing algorithms. Add or remove framework::add_alg<...>() lines
 * as needed; each algorithm's TOML section name matches the string passed.
 *
 * Run with:
 *   ./$name --config config/$name.toml
 */

#include <clas12/framework/core/AnalysisRunner.hpp>
// #include <clas12/algorithms/selection/ElectronSelectionAlg.hpp>
// #include <clas12/algorithms/monitoring/EventMonitor.hpp>

auto main(int argc, char* argv[]) -> int {
    return framework::AnalysisRunner::run(
        argc, argv, "$camel_name Analysis", "$name",
        [](auto& builder, const auto& /*config*/) {
            (void)builder;
            // framework::add_alg<algorithms::EventMonitor>(builder, "EventMonitor");
            // framework::add_alg<algorithms::ElectronSelectionAlg>(builder);
        });
}
EOF
fi

# ---------- algorithm header (optional) --------------------------------------
if [[ "$with_alg" -eq 1 ]]; then
    alg_hpp="$dir/${camel_name}Alg.hpp"
    cat > "$alg_hpp" <<EOF
#pragma once

/**
 * @file ${camel_name}Alg.hpp
 * @brief $camel_name algorithm.
 */

#include <clas12/framework/prelude.hpp>

namespace analyses {

using framework::Log;

/// Configuration for ${camel_name}Alg.
struct ${camel_name}Config {
    bool enable_cut = true;
    double cut_value = 1.0;

    static constexpr auto config_fields() {
        using T = ${camel_name}Config;
        return std::make_tuple(
            framework::field("enable_cut", &T::enable_cut),
            framework::field("cut_value",  &T::cut_value));
    }
};

class ${camel_name}Alg : public framework::Algorithm {
public:
    using Config = ${camel_name}Config;
    using StatusCode = framework::StatusCode;

    explicit ${camel_name}Alg(std::string name = "${camel_name}", const Config& config = Config{})
        : Algorithm(std::move(name)), m_config(config), m_cutflow("${camel_name}") {
        required_services(m_hist);
        // declare_banks("REC::Particle");
        declare_config(m_config);
        declare_histograms(h_example);
    }

    [[nodiscard]] auto config() const -> const Config& { return m_config; }

    auto on_initialize() -> StatusCode override {
        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Passes Cut");
        return StatusCode::SUCCESS();
    }

    auto process(framework::Event& event) -> StatusCode override {
        m_cutflow.pass("All Events");
        (void)event;

        if (m_config.enable_cut) {
            // ... per-event logic; \`return StatusCode::FILTERED();\` to skip.
            m_cutflow.pass("Passes Cut");
            h_example.fill(m_config.cut_value);
        }

        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        m_cutflow.print();
        return StatusCode::SUCCESS();
    }

private:
    Config             m_config;
    framework::Cutflow m_cutflow;

    framework::ServiceRef<framework::HistogramService> m_hist;

    framework::Hist1<> h_example{"/${camel_name}/example",
        "Example;value;Counts", 0.0, 10.0, 100};
};

}  // namespace analyses
EOF
fi

# ---------- TOML config -------------------------------------------------------
mkdir -p config
config_toml="config/$name.toml"
if [[ -e "$config_toml" ]]; then
    echo "warning: $config_toml already exists, leaving untouched" >&2
else
    if [[ "$with_alg" -eq 1 ]]; then
        cat > "$config_toml" <<EOF
# Configuration for the \`$name\` binary.
#
# Run with:
#   ./$name --config $config_toml

[run]
input       = ["data/your_input.hipo"]
output      = "$name.root"
num_threads = 4
percentage  = 100.0

[analysis]
target      = "LD2"
polarity    = "Outbending"
beam_energy = 10.53

# Per-algorithm section name = algorithm's name() string.
[${camel_name}]
enable_cut = true
cut_value  = 1.0
EOF
    else
        cat > "$config_toml" <<EOF
# Configuration for the \`$name\` binary.
#
# Run with:
#   ./$name --config $config_toml

[run]
input       = ["data/your_input.hipo"]
output      = "$name.root"
num_threads = 4
percentage  = 100.0

[analysis]
target      = "LD2"
polarity    = "Outbending"
beam_energy = 10.53
EOF
    fi
fi

# ---------- analyses/meson.build entry ----------------------------------------
meson_entry="
# $camel_name analysis
executable(
  '$name',
  files('$name/$name.cpp'),
  dependencies: analysis_deps,
)"
if grep -q "executable( *'$name'" analyses/meson.build 2>/dev/null; then
    echo "warning: analyses/meson.build already has an entry for '$name', skipping append" >&2
else
    printf '%s\n' "$meson_entry" >> analyses/meson.build
fi

cat <<EOF

  Created $dir/
    - $name.cpp
$([[ $with_alg -eq 1 ]] && echo "    - ${camel_name}Alg.hpp")
  Created $config_toml
  Appended executable() entry to analyses/meson.build

  Build:
      meson compile -C build
  Run:
      ./build/analyses/$name --config $config_toml
EOF
