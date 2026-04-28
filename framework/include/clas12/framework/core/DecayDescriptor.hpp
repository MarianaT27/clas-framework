#pragma once

#include <algorithm>
#include <map>
#include <memory>
#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include <clas12/PIDs.hpp>

namespace framework {

/**
 * @brief Particle information in a decay descriptor
 */
struct ParticleSpec {
    std::string name;     ///< Particle name (e.g., "p+", "pi-", "Lambda0")
    int pdg = 0;          ///< PDG code
    int charge = 0;       ///< Charge (in units of e)
    double mass = 0.0;    ///< PDG mass [GeV/c^2]
    bool is_final = true; ///< True if final state particle

    [[nodiscard]] auto is_valid() const -> bool { return pdg != 0; }
};

/**
 * @brief Node in a decay tree
 */
struct DecayNode {
    ParticleSpec particle;               ///< Particle at this node
    std::vector<DecayNode> daughters;    ///< Daughter particles (empty if final state)

    /// Check if this is a final state particle
    [[nodiscard]] auto is_final_state() const -> bool { return daughters.empty(); }

    /// Get number of daughters
    [[nodiscard]] auto num_daughters() const -> size_t { return daughters.size(); }
};

/**
 * @brief Parser and representation for decay descriptors
 *
 * DecayDescriptor parses LHCb-style decay strings and provides
 * information about decay topologies. This enables flexible
 * specification of decay chains without hardcoding.
 *
 * Supported syntax:
 * - Simple decays: "pi0 -> gamma gamma"
 * - Charged particles: "Lambda0 -> p+ pi-"
 * - Nested decays: "D0 -> [K- pi+] pi0" (future)
 * - Conjugate: "Lambda0 -> p+ pi-  cc" (future)
 *
 * Particle names (case-insensitive):
 * - Leptons: e-, e+, mu-, mu+, gamma
 * - Mesons: pi+, pi-, pi0, K+, K-, K0, eta, rho0, omega, phi
 * - Baryons: p+, p, n, Lambda0, Sigma+, Sigma0, Sigma-
 *
 * Example usage:
 * @code
 * auto desc = DecayDescriptor::parse("Lambda0 -> p+ pi-");
 * if (desc.is_valid()) {
 *     std::cout << "Mother: " << desc.mother().particle.name << std::endl;
 *     for (const auto& daughter : desc.daughters()) {
 *         std::cout << "  -> " << daughter.particle.name << std::endl;
 *     }
 * }
 * @endcode
 */
class DecayDescriptor {
   public:
    DecayDescriptor() = default;

    /**
     * @brief Parse a decay descriptor string
     *
     * @param descriptor Decay string (e.g., "Lambda0 -> p+ pi-")
     * @return DecayDescriptor object (check is_valid() for success)
     */
    static auto parse(const std::string& descriptor) -> DecayDescriptor {
        DecayDescriptor result;
        result.m_original = descriptor;

        // Parse the descriptor
        result.m_valid = result.parse_descriptor(descriptor);

        return result;
    }

    /**
     * @brief Check if descriptor was parsed successfully
     */
    [[nodiscard]] auto is_valid() const -> bool { return m_valid; }

    /**
     * @brief Get error message if parsing failed
     */
    [[nodiscard]] auto error_message() const -> const std::string& { return m_error; }

    /**
     * @brief Get original descriptor string
     */
    [[nodiscard]] auto original() const -> const std::string& { return m_original; }

    /**
     * @brief Get mother particle node
     */
    [[nodiscard]] auto mother() const -> const DecayNode& { return m_root; }

    /**
     * @brief Get daughter nodes
     */
    [[nodiscard]] auto daughters() const -> const std::vector<DecayNode>& {
        return m_root.daughters;
    }

    /**
     * @brief Get number of daughters
     */
    [[nodiscard]] auto num_daughters() const -> size_t {
        return m_root.daughters.size();
    }

    /**
     * @brief Get decay depth (1 for simple 2-body, 2+ for nested)
     */
    [[nodiscard]] auto depth() const -> int {
        return calculate_depth(m_root);
    }

    /**
     * @brief Get all final state PDG codes
     */
    [[nodiscard]] auto final_state_pids() const -> std::vector<int> {
        std::vector<int> pids;
        collect_final_state_pids(m_root, pids);
        return pids;
    }

    /**
     * @brief Get all final state particle specs
     */
    [[nodiscard]] auto final_state_particles() const -> std::vector<ParticleSpec> {
        std::vector<ParticleSpec> particles;
        collect_final_state_particles(m_root, particles);
        return particles;
    }

    /**
     * @brief Get mother PDG code
     */
    [[nodiscard]] auto mother_pdg() const -> int {
        return m_root.particle.pdg;
    }

    /**
     * @brief Get mother mass [GeV/c^2]
     */
    [[nodiscard]] auto mother_mass() const -> double {
        return m_root.particle.mass;
    }

    /**
     * @brief Check if this is a two-body decay
     */
    [[nodiscard]] auto is_two_body() const -> bool {
        return m_root.daughters.size() == 2;
    }

    /**
     * @brief Check if this is a three-body decay
     */
    [[nodiscard]] auto is_three_body() const -> bool {
        return m_root.daughters.size() == 3;
    }

   private:
    DecayNode m_root;
    bool m_valid = false;
    std::string m_error;
    std::string m_original;

    /**
     * @brief Parse the descriptor string
     */
    auto parse_descriptor(const std::string& descriptor) -> bool {
        // Find the arrow
        size_t arrow_pos = descriptor.find("->");
        if (arrow_pos == std::string::npos) {
            m_error = "Missing '->' in decay descriptor";
            return false;
        }

        // Extract mother and daughters
        std::string mother_str = trim(descriptor.substr(0, arrow_pos));
        std::string daughters_str = trim(descriptor.substr(arrow_pos + 2));

        if (mother_str.empty()) {
            m_error = "Empty mother particle";
            return false;
        }

        if (daughters_str.empty()) {
            m_error = "No daughter particles specified";
            return false;
        }

        // Parse mother
        auto mother_spec = particle_from_name(mother_str);
        if (!mother_spec.is_valid()) {
            m_error = "Unknown mother particle: " + mother_str;
            return false;
        }
        mother_spec.is_final = false;
        m_root.particle = mother_spec;

        // Parse daughters (space-separated)
        std::istringstream iss(daughters_str);
        std::string daughter_name;
        while (iss >> daughter_name) {
            // Skip "cc" (charge conjugate marker for future use)
            if (daughter_name == "cc") continue;

            auto daughter_spec = particle_from_name(daughter_name);
            if (!daughter_spec.is_valid()) {
                m_error = "Unknown daughter particle: " + daughter_name;
                return false;
            }

            DecayNode daughter_node;
            daughter_node.particle = daughter_spec;
            m_root.daughters.push_back(daughter_node);
        }

        if (m_root.daughters.empty()) {
            m_error = "No valid daughter particles";
            return false;
        }

        return true;
    }

    /**
     * @brief Convert particle name to ParticleSpec
     */
    static auto particle_from_name(const std::string& name) -> ParticleSpec {
        // Normalize name (lowercase for comparison)
        std::string lower_name = name;
        std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);

        // Particle database
        static const std::map<std::string, ParticleSpec> particle_db = {
            // Leptons
            {"e-", {"e-", 11, -1, 0.000511}},
            {"e+", {"e+", -11, +1, 0.000511}},
            {"electron", {"e-", 11, -1, 0.000511}},
            {"positron", {"e+", -11, +1, 0.000511}},
            {"mu-", {"mu-", 13, -1, 0.10566}},
            {"mu+", {"mu+", -13, +1, 0.10566}},
            {"gamma", {"gamma", 22, 0, 0.0}},
            {"photon", {"gamma", 22, 0, 0.0}},

            // Pions
            {"pi+", {"pi+", 211, +1, 0.13957}},
            {"pi-", {"pi-", -211, -1, 0.13957}},
            {"pi0", {"pi0", 111, 0, 0.13498}},
            {"pion+", {"pi+", 211, +1, 0.13957}},
            {"pion-", {"pi-", -211, -1, 0.13957}},
            {"pion0", {"pi0", 111, 0, 0.13498}},

            // Kaons
            {"k+", {"K+", 321, +1, 0.49368}},
            {"k-", {"K-", -321, -1, 0.49368}},
            {"k0", {"K0", 311, 0, 0.49761}},
            {"k0s", {"K0S", 310, 0, 0.49761}},
            {"k0l", {"K0L", 130, 0, 0.49761}},
            {"kaon+", {"K+", 321, +1, 0.49368}},
            {"kaon-", {"K-", -321, -1, 0.49368}},

            // Eta
            {"eta", {"eta", 221, 0, 0.54786}},
            {"eta'", {"eta'", 331, 0, 0.95778}},
            {"etaprime", {"eta'", 331, 0, 0.95778}},

            // Vector mesons
            {"rho0", {"rho0", 113, 0, 0.77526}},
            {"rho+", {"rho+", 213, +1, 0.77526}},
            {"rho-", {"rho-", -213, -1, 0.77526}},
            {"omega", {"omega", 223, 0, 0.78265}},
            {"phi", {"phi", 333, 0, 1.01946}},

            // Proton/Neutron
            {"p+", {"p+", 2212, +1, 0.93827}},
            {"p", {"p+", 2212, +1, 0.93827}},
            {"proton", {"p+", 2212, +1, 0.93827}},
            {"pbar", {"pbar", -2212, -1, 0.93827}},
            {"antiproton", {"pbar", -2212, -1, 0.93827}},
            {"n", {"n", 2112, 0, 0.93957}},
            {"neutron", {"n", 2112, 0, 0.93957}},

            // Strange baryons
            {"lambda0", {"Lambda0", 3122, 0, 1.11568}},
            {"lambda", {"Lambda0", 3122, 0, 1.11568}},
            {"lambdabar", {"Lambdabar", -3122, 0, 1.11568}},
            {"sigma+", {"Sigma+", 3222, +1, 1.18937}},
            {"sigma0", {"Sigma0", 3212, 0, 1.19264}},
            {"sigma-", {"Sigma-", 3112, -1, 1.19745}},
            {"xi0", {"Xi0", 3322, 0, 1.31486}},
            {"xi-", {"Xi-", 3312, -1, 1.32171}},

            // Charm mesons
            {"d0", {"D0", 421, 0, 1.86484}},
            {"d+", {"D+", 411, +1, 1.86966}},
            {"d-", {"D-", -411, -1, 1.86966}},
            {"ds+", {"Ds+", 431, +1, 1.96835}},
            {"ds-", {"Ds-", -431, -1, 1.96835}},

            // Charmonium
            {"j/psi", {"J/psi", 443, 0, 3.09690}},
            {"jpsi", {"J/psi", 443, 0, 3.09690}},
            {"psi(2s)", {"psi(2S)", 100443, 0, 3.68610}},

            // Deuteron
            {"d", {"d", 1000010020, +1, 1.87561}},
            {"deuteron", {"d", 1000010020, +1, 1.87561}},
        };

        auto it = particle_db.find(lower_name);
        if (it != particle_db.end()) {
            return it->second;
        }

        // Return invalid spec
        return ParticleSpec{name, 0, 0, 0.0};
    }

    /**
     * @brief Calculate depth of decay tree
     */
    static auto calculate_depth(const DecayNode& node) -> int {
        if (node.is_final_state()) return 0;

        int max_depth = 0;
        for (const auto& daughter : node.daughters) {
            max_depth = std::max(max_depth, calculate_depth(daughter));
        }
        return max_depth + 1;
    }

    /**
     * @brief Collect final state PDG codes
     */
    static auto collect_final_state_pids(const DecayNode& node, std::vector<int>& pids) -> void {
        if (node.is_final_state()) {
            pids.push_back(node.particle.pdg);
        } else {
            for (const auto& daughter : node.daughters) {
                collect_final_state_pids(daughter, pids);
            }
        }
    }

    /**
     * @brief Collect final state particle specs
     */
    static auto collect_final_state_particles(const DecayNode& node,
                                              std::vector<ParticleSpec>& particles) -> void {
        if (node.is_final_state()) {
            particles.push_back(node.particle);
        } else {
            for (const auto& daughter : node.daughters) {
                collect_final_state_particles(daughter, particles);
            }
        }
    }

    /**
     * @brief Trim whitespace from string
     */
    static auto trim(const std::string& s) -> std::string {
        auto start = s.find_first_not_of(" \t\n\r");
        if (start == std::string::npos) return "";
        auto end = s.find_last_not_of(" \t\n\r");
        return s.substr(start, end - start + 1);
    }
};

}  // namespace framework
