#pragma once

#include <string>

namespace Core {
enum class Target {
    Undefined,
    LD2,
    CxC,
    CuSn,
    Cu,
    Sn,
};

inline auto to_string(Target target) -> std::string {
    switch (target) {
        case Target::Undefined: return "Undefined";
        case Target::LD2: return "LD2";
        case Target::CxC: return "CxC";
        case Target::CuSn: return "CuSn";
        case Target::Cu: return "Cu";
        case Target::Sn: return "Sn";
    }
    return "Unknown";
}

inline auto string_to_target(const std::string& str) -> Target {
    if (str == "LD2") return Target::LD2;
    if (str == "CxC") return Target::CxC;
    if (str == "CuSn") return Target::CuSn;
    if (str == "Cu") return Target::Cu;
    if (str == "Sn") return Target::Sn;
    return Target::Undefined;
}

enum class Polarity {
    Inbending,
    Outbending,
};

inline auto to_string(Polarity polarity) -> std::string {
    switch (polarity) {
        case Polarity::Inbending: return "IB";
        case Polarity::Outbending: return "OB";
    }
    return "Unknown";
}

inline auto string_to_polarity(const std::string& str) -> Polarity {
    if (str == "In") return Polarity::Inbending;
    if (str == "Out") return Polarity::Outbending;
    return Polarity::Inbending; // Default value
}

}  // namespace Core