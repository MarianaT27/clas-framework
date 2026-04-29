#pragma once

#include <string>
#include <fmt/format.h>

enum class Target {
    Undefined,
    LD2,
    CxC,
    CuSn,
    Cu,
    Sn,
    Custom,  // For user-defined targets not in the standard list
};

inline auto to_string(Target target) -> std::string {
    switch (target) {
        case Target::Undefined:
            return "Undefined";
        case Target::LD2:
            return "LD2";
        case Target::CxC:
            return "CxC";
        case Target::CuSn:
            return "CuSn";
        case Target::Cu:
            return "Cu";
        case Target::Sn:
            return "Sn";
        case Target::Custom:
            return "Custom";
    }
    return "Unknown";
}

inline auto string_to_target(const std::string& str) -> Target {
    if (str == "LD2") return Target::LD2;
    if (str == "CxC") return Target::CxC;
    if (str == "CuSn") return Target::CuSn;
    if (str == "Cu") return Target::Cu;
    if (str == "Sn") return Target::Sn;
    if (str == "Custom") return Target::Custom;
    return Target::Undefined;
}

enum class Polarity {
    Inbending,
    Outbending,
};

inline auto to_string(Polarity polarity) -> std::string {
    switch (polarity) {
        case Polarity::Inbending:
            return "IB";
        case Polarity::Outbending:
            return "OB";
    }
    return "Unknown";
}

inline auto string_to_polarity(const std::string& str) -> Polarity {
    if (str == "In" || str == "IB" || str == "Inbending" || str == "inbending") return Polarity::Inbending;
    if (str == "Out" || str == "OB" || str == "Outbending" || str == "outbending") return Polarity::Outbending;
    return Polarity::Inbending;  // Default value
}

enum class RunGroup {
    RG_A,
    RG_B,
    RG_C,
    RG_D,
};

inline auto to_string(RunGroup run_group) -> std::string {
    switch (run_group) {
        case RunGroup::RG_A:
            return "RG-A";
        case RunGroup::RG_B:
            return "RG-B";
        case RunGroup::RG_C:
            return "RG-C";
        case RunGroup::RG_D:
            return "RG-D";
    }
    return "Unknown";
}

inline auto string_to_rungroup(const std::string& str) -> RunGroup {
    if (str == "RG-A" || str == "RGA" || str == "A") return RunGroup::RG_A;
    if (str == "RG-B" || str == "RGB" || str == "B") return RunGroup::RG_B;
    if (str == "RG-C" || str == "RGC" || str == "C") return RunGroup::RG_C;
    if (str == "RG-D" || str == "RGD" || str == "D") return RunGroup::RG_D;
    return RunGroup::RG_D;  // Default to RG-D
}


// =============================================================================
// TOML configuration hooks
// =============================================================================
//
// `framework::declare_config(...)` looks up a free `parse_config_value(sv, T&)`
// for any enum field via ADL. Defining these in the same namespace as the
// enums (here, the global namespace) lets them be picked up automatically.
// `bool` return: `true` on successful parse (assigns out), `false` to emit a
// type-mismatch error from the framework.

#include <string_view>

inline auto parse_config_value(std::string_view s, Target& out) -> bool {
    auto t = string_to_target(std::string(s));
    out = t;
    return t != Target::Undefined;
}

inline auto parse_config_value(std::string_view s, Polarity& out) -> bool {
    out = string_to_polarity(std::string(s));
    return true;  // string_to_polarity defaults to Inbending if unknown
}

inline auto parse_config_value(std::string_view s, RunGroup& out) -> bool {
    out = string_to_rungroup(std::string(s));
    return true;
}

// fmt formatters for clas12 enums
template <>
struct fmt::formatter<Target> : fmt::formatter<std::string> {
    auto format(Target t, fmt::format_context& ctx) const {
        return fmt::formatter<std::string>::format(to_string(t), ctx);
    }
};

template <>
struct fmt::formatter<Polarity> : fmt::formatter<std::string> {
    auto format(Polarity p, fmt::format_context& ctx) const {
        return fmt::formatter<std::string>::format(to_string(p), ctx);
    }
};

template <>
struct fmt::formatter<RunGroup> : fmt::formatter<std::string> {
    auto format(RunGroup rg, fmt::format_context& ctx) const {
        return fmt::formatter<std::string>::format(to_string(rg), ctx);
    }
};