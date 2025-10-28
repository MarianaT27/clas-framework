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

}  // namespace Core