#pragma once

#include <optional>
#include <string>

namespace CLAS {

enum class PIDs {
    ELECTRON = 11,
    POSITRON = -11,
    PROTON = 2212,
    NEUTRON = 2112,
    PIPLUS = 211,
    PIMINUS = -211,
    KPLUS = 321,
    KMINUS = -321,
    PHOTON = 22
};

/**
 * @brief Overload == operator to compare PIDs enum with int
 */
inline auto operator==(PIDs pid, int value) -> bool {
    return static_cast<int>(pid) == value;
}

/**
 * @brief Overload == operator to compare int with PIDs enum
 */
inline auto operator==(int value, PIDs pid) -> bool {
    return value == static_cast<int>(pid);
}

/**
 * @brief Convert integer PID to enum (if valid)
 *
 * @param pid Integer particle ID
 * @return PIDs enum value if valid, otherwise std::nullopt
 */
inline auto to_pid_enum(int pid) -> std::optional<PIDs> {
    switch (pid) {
        case 11: return PIDs::ELECTRON;
        case -11: return PIDs::POSITRON;
        case 2212: return PIDs::PROTON;
        case 2112: return PIDs::NEUTRON;
        case 211: return PIDs::PIPLUS;
        case -211: return PIDs::PIMINUS;
        case 321: return PIDs::KPLUS;
        case -321: return PIDs::KMINUS;
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
        case PIDs::ELECTRON: return "Electron";
        case PIDs::POSITRON: return "Positron";
        case PIDs::PROTON: return "Proton";
        case PIDs::NEUTRON: return "Neutron";
        case PIDs::PIPLUS: return "PionPlus";
        case PIDs::PIMINUS: return "PionMinus";
        case PIDs::KPLUS: return "KaonPlus";
        case PIDs::KMINUS: return "KaonMinus";
        default: return "Unknown";
    }
}

}  // namespace CLAS