#pragma once

#include <optional>
#include <string>

namespace Core {

enum class PIDs {
    Electron = 11,
    Positron = -11,
    Proton = 2212,
    Neutron = 2112,
    PionPlus = 211,
    PionMinus = -211,
    KaonPlus = 321,
    KaonMinus = -321,
    Photon = 22
};

/**
 * @brief Convert integer PID to enum (if valid)
 *
 * @param pid Integer particle ID
 * @return PIDs enum value if valid, otherwise std::nullopt
 */
inline auto to_pid_enum(int pid) -> std::optional<PIDs> {
    switch (pid) {
        case 11: return PIDs::Electron;
        case -11: return PIDs::Positron;
        case 2212: return PIDs::Proton;
        case 2112: return PIDs::Neutron;
        case 211: return PIDs::PionPlus;
        case -211: return PIDs::PionMinus;
        case 321: return PIDs::KaonPlus;
        case -321: return PIDs::KaonMinus;
        default: return std::nullopt;
    }
}

/**
 * @brief Check if an integer PID matches an enum PID
 */
inline auto matches(int pid, PIDs type) -> bool {
    return pid == static_cast<int>(type);
}

/**
 * @brief Get human-readable name for a PID
 */
inline auto pid_name(PIDs pid) -> std::string {
    switch (pid) {
        case PIDs::Electron: return "Electron";
        case PIDs::Positron: return "Positron";
        case PIDs::Proton: return "Proton";
        case PIDs::Neutron: return "Neutron";
        case PIDs::PionPlus: return "PionPlus";
        case PIDs::PionMinus: return "PionMinus";
        case PIDs::KaonPlus: return "KaonPlus";
        case PIDs::KaonMinus: return "KaonMinus";
        default: return "Unknown";
    }
}

}  // namespace Core