#pragma once

#include <cmath>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include <clas12/Constants.hpp>
#include <clas12/Particle.hpp>
#include <clas12/Banks.hpp>
#include <clas12/ReadBank.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/IService.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/services/LoggingService.hpp>

namespace clas12::framework {

/**
 * @brief PID hypothesis (particle species)
 */
enum class PIDHypothesis {
    Electron,
    Positron,
    Muon,
    Pion,
    Kaon,
    Proton,
    Deuteron,
    Unknown
};

/**
 * @brief Convert PIDHypothesis to string
 */
inline auto pid_hypothesis_to_string(PIDHypothesis h) -> std::string {
    switch (h) {
        case PIDHypothesis::Electron: return "electron";
        case PIDHypothesis::Positron: return "positron";
        case PIDHypothesis::Muon: return "muon";
        case PIDHypothesis::Pion: return "pion";
        case PIDHypothesis::Kaon: return "kaon";
        case PIDHypothesis::Proton: return "proton";
        case PIDHypothesis::Deuteron: return "deuteron";
        case PIDHypothesis::Unknown: return "unknown";
    }
    return "unknown";
}

/**
 * @brief Get particle mass for hypothesis [GeV/c^2]
 */
inline auto hypothesis_mass(PIDHypothesis h) -> double {
    switch (h) {
        case PIDHypothesis::Electron:
        case PIDHypothesis::Positron: return clas12::constants::mass::electron;
        case PIDHypothesis::Muon: return 0.10566;  // Muon mass
        case PIDHypothesis::Pion: return clas12::constants::mass::pion_plus;
        case PIDHypothesis::Kaon: return clas12::constants::mass::kaon_plus;
        case PIDHypothesis::Proton: return clas12::constants::mass::proton;
        case PIDHypothesis::Deuteron: return clas12::constants::mass::deuterium;
        case PIDHypothesis::Unknown: return 0.0;
    }
    return 0.0;
}

/**
 * @brief Detector contributions to PID
 */
enum class PIDDetector {
    FTOF,    ///< Forward Time-of-Flight
    CTOF,    ///< Central Time-of-Flight
    HTCC,    ///< High Threshold Cherenkov Counter
    LTCC,    ///< Low Threshold Cherenkov Counter
    RICH,    ///< Ring Imaging Cherenkov (future)
    ECAL,    ///< Electromagnetic Calorimeter (sampling fraction)
    DC       ///< Drift Chamber (dE/dx, future)
};

/**
 * @brief PID result for a particle
 */
struct PIDResult {
    PIDHypothesis best_hypothesis = PIDHypothesis::Unknown;
    double confidence = 0.0;

    /// Log-likelihoods for each hypothesis
    std::map<PIDHypothesis, double> log_likelihoods;

    /// Delta log-likelihood relative to pion hypothesis
    std::map<PIDHypothesis, double> delta_ll;

    /// Per-detector contributions
    std::map<PIDDetector, std::map<PIDHypothesis, double>> detector_contributions;

    // ========================================================================
    // Accessor Methods
    // ========================================================================

    /**
     * @brief Get delta log-likelihood between two hypotheses
     *
     * @param h1 First hypothesis
     * @param h2 Second hypothesis (reference)
     * @return DLL = log(L(h1)) - log(L(h2))
     */
    [[nodiscard]] auto get_dll(PIDHypothesis h1, PIDHypothesis h2) const -> double {
        auto it1 = log_likelihoods.find(h1);
        auto it2 = log_likelihoods.find(h2);
        if (it1 == log_likelihoods.end() || it2 == log_likelihoods.end()) {
            return 0.0;
        }
        return it1->second - it2->second;
    }

    /**
     * @brief Get probability for a hypothesis (from normalized likelihoods)
     */
    [[nodiscard]] auto probability(PIDHypothesis h) const -> double {
        auto it = log_likelihoods.find(h);
        if (it == log_likelihoods.end()) return 0.0;

        // Convert log-likelihoods to probabilities
        double max_ll = -1e10;
        for (const auto& [hyp, ll] : log_likelihoods) {
            max_ll = std::max(max_ll, ll);
        }

        double sum = 0.0;
        double this_exp = 0.0;
        for (const auto& [hyp, ll] : log_likelihoods) {
            double exp_val = std::exp(ll - max_ll);
            sum += exp_val;
            if (hyp == h) this_exp = exp_val;
        }

        return (sum > 0.0) ? this_exp / sum : 0.0;
    }

    /**
     * @brief Check if particle is consistent with electron
     */
    [[nodiscard]] auto is_electron(double dll_cut = 0.0) const -> bool {
        return get_dll(PIDHypothesis::Electron, PIDHypothesis::Pion) > dll_cut;
    }

    /**
     * @brief Check if particle is consistent with pion
     */
    [[nodiscard]] auto is_pion(double dll_cut = 0.0) const -> bool {
        // Pion is default, so check it's not something else
        return get_dll(PIDHypothesis::Pion, PIDHypothesis::Kaon) > -dll_cut &&
               get_dll(PIDHypothesis::Pion, PIDHypothesis::Proton) > -dll_cut;
    }

    /**
     * @brief Check if particle is consistent with kaon
     */
    [[nodiscard]] auto is_kaon(double dll_cut = 0.0) const -> bool {
        return get_dll(PIDHypothesis::Kaon, PIDHypothesis::Pion) > dll_cut;
    }

    /**
     * @brief Check if particle is consistent with proton
     */
    [[nodiscard]] auto is_proton(double dll_cut = 0.0) const -> bool {
        return get_dll(PIDHypothesis::Proton, PIDHypothesis::Pion) > dll_cut;
    }
};

/**
 * @brief Service for enhanced particle identification
 *
 * PIDService provides log-likelihood based particle identification
 * by combining information from multiple CLAS12 detectors:
 * - Time-of-Flight (FTOF/CTOF)
 * - Cherenkov counters (HTCC/LTCC)
 * - Electromagnetic calorimeter
 *
 * The service computes delta log-likelihoods (DLL) which measure
 * how much more likely one hypothesis is compared to another.
 *
 * Example usage:
 * @code
 * // In algorithm on_initialize():
 * m_pid_svc = &require_service<PIDService>("PIDService");
 *
 * // In algorithm process():
 * for (const auto& track : charged_tracks) {
 *     auto pid_result = m_pid_svc->compute_pid(track, event);
 *
 *     // Use DLL for kaon selection
 *     if (pid_result.get_dll(PIDHypothesis::Kaon, PIDHypothesis::Pion) > 2.0) {
 *         // Likely a kaon
 *         double eff_weight = m_pid_svc->efficiency_weight(
 *             track, PIDHypothesis::Kaon, 2.0);
 *         h_kaon_p->fill(track.p(), eff_weight);
 *     }
 * }
 * @endcode
 */
class PIDService : public Service {
   public:
    /// Static service name for compile-time access
    static constexpr std::string_view service_name() { return "PIDService"; }

    explicit PIDService(std::string name = "PIDService")
        : Service(std::move(name)) {}

    // ========================================================================
    // Service Lifecycle
    // ========================================================================

    auto on_initialize() -> StatusCode override {
        Log::info(this->name(), "PIDService initialized");
        Log::info(this->name(), "  Use TOF: {}", m_use_tof.get());
        Log::info(this->name(), "  Use Cherenkov: {}", m_use_cherenkov.get());
        Log::info(this->name(), "  Use Calorimeter: {}", m_use_calorimeter.get());

        // Initialize default PDF parameters
        initialize_default_pdfs();

        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        Log::info(this->name(), "PIDService finalized");
        return StatusCode::SUCCESS();
    }

    // ========================================================================
    // Configuration Properties
    // ========================================================================

    /// Use TOF information in PID
    Property<bool> m_use_tof{"use_tof", true, "Include TOF in PID"};

    /// Use Cherenkov information in PID
    Property<bool> m_use_cherenkov{"use_cherenkov", true, "Include Cherenkov in PID"};

    /// Use calorimeter information in PID
    Property<bool> m_use_calorimeter{"use_calorimeter", true, "Include calorimeter in PID"};

    /// TOF resolution [ns]
    Property<double> m_tof_resolution{"tof_resolution", 0.080, "TOF time resolution [ns]"};

    /// HTCC photoelectron threshold for electron ID
    Property<double> m_htcc_nphe_cut{"htcc_nphe_cut", 2.0, "HTCC nphe threshold for electron"};

    /// Calibration file path
    Property<std::string> m_calibration_file{"calibration_file", "",
                                              "Path to PID calibration file"};

    // ========================================================================
    // Log-Likelihood PID
    // ========================================================================

    /**
     * @brief Compute full PID result for a particle
     *
     * @param particle Reconstructed particle
     * @param event Event (for detector bank access)
     * @return PIDResult with likelihoods for all hypotheses
     */
    auto compute_pid(const clas12::Particle& particle, Event& event) -> PIDResult {
        PIDResult result;

        // List of hypotheses to test
        std::vector<PIDHypothesis> hypotheses = {
            PIDHypothesis::Electron,
            PIDHypothesis::Pion,
            PIDHypothesis::Kaon,
            PIDHypothesis::Proton
        };

        // Add positron for positive tracks
        if (particle.charge() > 0) {
            hypotheses[0] = PIDHypothesis::Positron;
        }

        // Initialize log-likelihoods
        for (auto h : hypotheses) {
            result.log_likelihoods[h] = 0.0;
        }

        // Add TOF contribution
        if (m_use_tof.get()) {
            auto tof_lls = compute_tof_log_likelihood(particle, event);
            for (const auto& [h, ll] : tof_lls) {
                result.log_likelihoods[h] += ll;
                result.detector_contributions[PIDDetector::FTOF][h] = ll;
            }
        }

        // Add Cherenkov contribution
        if (m_use_cherenkov.get()) {
            auto cher_lls = compute_cherenkov_log_likelihood(particle, event);
            for (const auto& [h, ll] : cher_lls) {
                result.log_likelihoods[h] += ll;
                result.detector_contributions[PIDDetector::HTCC][h] = ll;
            }
        }

        // Add calorimeter contribution (mainly for e/pi separation)
        if (m_use_calorimeter.get()) {
            auto calo_lls = compute_calorimeter_log_likelihood(particle, event);
            for (const auto& [h, ll] : calo_lls) {
                result.log_likelihoods[h] += ll;
                result.detector_contributions[PIDDetector::ECAL][h] = ll;
            }
        }

        // Compute delta log-likelihoods relative to pion
        for (const auto& [h, ll] : result.log_likelihoods) {
            result.delta_ll[h] = ll - result.log_likelihoods[PIDHypothesis::Pion];
        }

        // Find best hypothesis
        double max_ll = -1e10;
        for (const auto& [h, ll] : result.log_likelihoods) {
            if (ll > max_ll) {
                max_ll = ll;
                result.best_hypothesis = h;
            }
        }

        // Compute confidence (probability of best hypothesis)
        result.confidence = result.probability(result.best_hypothesis);

        return result;
    }

    /**
     * @brief Get delta log-likelihood between two hypotheses
     */
    [[nodiscard]] auto delta_log_likelihood(const clas12::Particle& particle,
                                            Event& event,
                                            PIDHypothesis h1,
                                            PIDHypothesis h2) -> double {
        auto result = compute_pid(particle, event);
        return result.get_dll(h1, h2);
    }

    // ========================================================================
    // Single Detector Contributions
    // ========================================================================

    /**
     * @brief Get TOF-based log-likelihood
     */
    [[nodiscard]] auto tof_log_likelihood(const clas12::Particle& particle,
                                          Event& event,
                                          PIDHypothesis hypothesis) -> double {
        auto lls = compute_tof_log_likelihood(particle, event);
        auto it = lls.find(hypothesis);
        return (it != lls.end()) ? it->second : 0.0;
    }

    /**
     * @brief Get Cherenkov-based log-likelihood
     */
    [[nodiscard]] auto cherenkov_log_likelihood(const clas12::Particle& particle,
                                                Event& event,
                                                PIDHypothesis hypothesis) -> double {
        auto lls = compute_cherenkov_log_likelihood(particle, event);
        auto it = lls.find(hypothesis);
        return (it != lls.end()) ? it->second : 0.0;
    }

    /**
     * @brief Get calorimeter-based log-likelihood
     */
    [[nodiscard]] auto calorimeter_log_likelihood(const clas12::Particle& particle,
                                                  Event& event,
                                                  PIDHypothesis hypothesis) -> double {
        auto lls = compute_calorimeter_log_likelihood(particle, event);
        auto it = lls.find(hypothesis);
        return (it != lls.end()) ? it->second : 0.0;
    }

    // ========================================================================
    // Simple PID Decisions
    // ========================================================================

    /**
     * @brief Check if particle is consistent with electron
     */
    [[nodiscard]] auto is_electron(const clas12::Particle& particle,
                                   Event& event,
                                   double dll_cut = 2.0) -> bool {
        auto result = compute_pid(particle, event);
        return result.is_electron(dll_cut);
    }

    /**
     * @brief Check if particle is consistent with pion
     */
    [[nodiscard]] auto is_pion(const clas12::Particle& particle,
                               Event& event,
                               double dll_cut = 0.0) -> bool {
        auto result = compute_pid(particle, event);
        return result.is_pion(dll_cut);
    }

    /**
     * @brief Check if particle is consistent with kaon
     */
    [[nodiscard]] auto is_kaon(const clas12::Particle& particle,
                               Event& event,
                               double dll_cut = 0.0) -> bool {
        auto result = compute_pid(particle, event);
        return result.is_kaon(dll_cut);
    }

    /**
     * @brief Check if particle is consistent with proton
     */
    [[nodiscard]] auto is_proton(const clas12::Particle& particle,
                                 Event& event,
                                 double dll_cut = 0.0) -> bool {
        auto result = compute_pid(particle, event);
        return result.is_proton(dll_cut);
    }

    // ========================================================================
    // Efficiency Corrections
    // ========================================================================

    /**
     * @brief Get PID efficiency for a particle
     *
     * Returns the efficiency for correctly identifying a particle
     * as the given hypothesis with the specified DLL cut.
     *
     * @param particle Particle
     * @param hypothesis True hypothesis
     * @param dll_cut Applied DLL cut
     * @return Efficiency (0-1)
     */
    [[nodiscard]] auto efficiency(const clas12::Particle& particle,
                                  PIDHypothesis hypothesis,
                                  double dll_cut) -> double {
        // Look up in efficiency map if loaded
        auto it = m_efficiency_maps.find(hypothesis);
        if (it != m_efficiency_maps.end()) {
            return lookup_efficiency(it->second, particle.p(), particle.theta());
        }

        // Default parameterization (placeholder)
        // In practice, these should be loaded from calibration files
        double p = particle.p();
        double theta = particle.theta();

        // Simple efficiency model (decreases at low momentum)
        double eff = 0.95;
        if (p < 0.5) {
            eff *= p / 0.5;
        }

        // Reduce efficiency at extreme angles
        if (theta < 10.0 || theta > 35.0) {
            eff *= 0.9;
        }

        return eff;
    }

    /**
     * @brief Get PID efficiency weight (1/efficiency)
     */
    [[nodiscard]] auto efficiency_weight(const clas12::Particle& particle,
                                         PIDHypothesis hypothesis,
                                         double dll_cut) -> double {
        double eff = efficiency(particle, hypothesis, dll_cut);
        return (eff > 0.01) ? 1.0 / eff : 1.0;
    }

    /**
     * @brief Load efficiency maps from file
     */
    auto load_efficiency_maps(const std::string& filename) -> StatusCode {
        // TODO: Implement loading from ROOT file
        Log::info(this->name(), "Loading efficiency maps from {}", filename);
        return StatusCode::SUCCESS();
    }

   private:
    // PDF parameters for each detector and hypothesis
    struct GaussianPDF {
        double mean = 0.0;
        double sigma = 1.0;
    };

    std::map<PIDDetector, std::map<PIDHypothesis, std::function<GaussianPDF(double)>>> m_pdf_params;

    // Efficiency maps (p, theta binned)
    struct EfficiencyMap {
        std::vector<double> p_bins;
        std::vector<double> theta_bins;
        std::vector<std::vector<double>> efficiency;
    };
    std::map<PIDHypothesis, EfficiencyMap> m_efficiency_maps;

    /**
     * @brief Initialize default PDF parameters
     */
    auto initialize_default_pdfs() -> void {
        // TOF beta PDFs - parameterized by momentum
        // Mean is expected beta, sigma is resolution
        double sigma_tof = m_tof_resolution.get();

        for (auto h : {PIDHypothesis::Electron, PIDHypothesis::Positron,
                       PIDHypothesis::Pion, PIDHypothesis::Kaon, PIDHypothesis::Proton}) {
            m_pdf_params[PIDDetector::FTOF][h] = [h, sigma_tof](double p) -> GaussianPDF {
                double mass = hypothesis_mass(h);
                double beta_expected = p / std::sqrt(p * p + mass * mass);
                // Time resolution translates to beta resolution
                double sigma_beta = sigma_tof * beta_expected * beta_expected / p;
                return {beta_expected, sigma_beta};
            };
        }
    }

    /**
     * @brief Compute TOF log-likelihood for all hypotheses
     */
    auto compute_tof_log_likelihood(const clas12::Particle& particle, Event& event)
        -> std::map<PIDHypothesis, double> {
        std::map<PIDHypothesis, double> result;

        // Get measured beta from particle
        double beta_meas = particle.beta();
        double p = particle.p();

        if (beta_meas <= 0 || beta_meas > 1.0 || p <= 0) {
            // No valid TOF measurement
            return result;
        }

        // Compute log-likelihood for each hypothesis
        for (auto h : {PIDHypothesis::Electron, PIDHypothesis::Positron,
                       PIDHypothesis::Pion, PIDHypothesis::Kaon, PIDHypothesis::Proton}) {

            double mass = hypothesis_mass(h);
            double beta_expected = p / std::sqrt(p * p + mass * mass);

            // TOF resolution model
            double sigma = m_tof_resolution.get() * beta_expected * beta_expected / p;
            sigma = std::max(sigma, 0.01);  // Minimum resolution

            // Gaussian log-likelihood
            double pull = (beta_meas - beta_expected) / sigma;
            result[h] = -0.5 * pull * pull;
        }

        return result;
    }

    /**
     * @brief Compute Cherenkov log-likelihood
     */
    auto compute_cherenkov_log_likelihood(const clas12::Particle& particle, Event& event)
        -> std::map<PIDHypothesis, double> {
        std::map<PIDHypothesis, double> result;

        // Try to read Cherenkov bank
        if (!event.has_bank("REC::Cherenkov")) {
            return result;
        }

        auto& cher_bank = event.get_bank("REC::Cherenkov");

        // Find matching HTCC hit
        double nphe = 0.0;
        for (int i = 0; i < cher_bank.getRows(); ++i) {
            int pindex = cher_bank.getInt("pindex", i);
            int detector = cher_bank.getInt("detector", i);

            if (pindex == particle.index() && detector == 15) {  // HTCC
                nphe = cher_bank.getFloat("nphe", i);
                break;
            }
        }

        // HTCC thresholds (approximate)
        // Electrons should produce lots of light
        // Pions below threshold until ~4.9 GeV
        // Kaons below threshold until ~17 GeV
        // Protons below threshold until ~33 GeV

        double p = particle.p();
        double htcc_threshold_cut = m_htcc_nphe_cut.get();

        // Electron/positron: expect light above threshold
        if (nphe > htcc_threshold_cut) {
            result[PIDHypothesis::Electron] = std::log(0.95);  // High probability
            result[PIDHypothesis::Positron] = std::log(0.95);
        } else {
            result[PIDHypothesis::Electron] = std::log(0.05);
            result[PIDHypothesis::Positron] = std::log(0.05);
        }

        // Pions: expect light only above ~4.9 GeV
        double pion_threshold = 4.9;
        if (p < pion_threshold) {
            // Below threshold: should have no light
            if (nphe < htcc_threshold_cut) {
                result[PIDHypothesis::Pion] = std::log(0.90);
            } else {
                result[PIDHypothesis::Pion] = std::log(0.10);
            }
        } else {
            // Above threshold: should have light
            if (nphe > htcc_threshold_cut) {
                result[PIDHypothesis::Pion] = std::log(0.90);
            } else {
                result[PIDHypothesis::Pion] = std::log(0.50);
            }
        }

        // Kaons and protons: below HTCC threshold in CLAS12 momentum range
        if (nphe < htcc_threshold_cut) {
            result[PIDHypothesis::Kaon] = std::log(0.90);
            result[PIDHypothesis::Proton] = std::log(0.90);
        } else {
            // Seeing light is inconsistent with K/p at typical momenta
            result[PIDHypothesis::Kaon] = std::log(0.20);
            result[PIDHypothesis::Proton] = std::log(0.10);
        }

        return result;
    }

    /**
     * @brief Compute calorimeter log-likelihood
     */
    auto compute_calorimeter_log_likelihood(const clas12::Particle& particle, Event& event)
        -> std::map<PIDHypothesis, double> {
        std::map<PIDHypothesis, double> result;

        // Try to read calorimeter bank
        if (!event.has_bank("REC::Calorimeter")) {
            return result;
        }

        auto& calo_bank = event.get_bank("REC::Calorimeter");

        // Sum energy in PCAL, ECin, ECout
        double energy_pcal = 0.0, energy_ecin = 0.0, energy_ecout = 0.0;

        for (int i = 0; i < calo_bank.getRows(); ++i) {
            int pindex = calo_bank.getInt("pindex", i);
            if (pindex != particle.index()) continue;

            int layer = calo_bank.getInt("layer", i);
            double energy = calo_bank.getFloat("energy", i);

            if (layer == 1) energy_pcal = energy;       // PCAL
            else if (layer == 4) energy_ecin = energy;  // ECin
            else if (layer == 7) energy_ecout = energy; // ECout
        }

        double total_energy = energy_pcal + energy_ecin + energy_ecout;
        double p = particle.p();

        if (total_energy <= 0 || p <= 0) {
            return result;
        }

        // Sampling fraction = E_calo / p
        double sf = total_energy / p;

        // Electrons have SF ~ 0.25-0.30 (full shower)
        // Hadrons have SF << 0.25 (MIP or hadronic shower)

        // Expected SF for electrons (with some p-dependence)
        double sf_electron_mean = 0.27;
        double sf_electron_sigma = 0.03;

        // Expected SF for hadrons (MIP)
        double sf_hadron_mean = 0.08;
        double sf_hadron_sigma = 0.04;

        // Gaussian log-likelihoods
        double pull_e = (sf - sf_electron_mean) / sf_electron_sigma;
        double pull_h = (sf - sf_hadron_mean) / sf_hadron_sigma;

        result[PIDHypothesis::Electron] = -0.5 * pull_e * pull_e;
        result[PIDHypothesis::Positron] = -0.5 * pull_e * pull_e;
        result[PIDHypothesis::Pion] = -0.5 * pull_h * pull_h;
        result[PIDHypothesis::Kaon] = -0.5 * pull_h * pull_h;
        result[PIDHypothesis::Proton] = -0.5 * pull_h * pull_h;

        return result;
    }

    /**
     * @brief Look up efficiency from map
     */
    auto lookup_efficiency(const EfficiencyMap& map, double p, double theta) const -> double {
        if (map.p_bins.empty() || map.theta_bins.empty()) {
            return 1.0;
        }

        // Find bin indices
        auto p_it = std::lower_bound(map.p_bins.begin(), map.p_bins.end(), p);
        auto th_it = std::lower_bound(map.theta_bins.begin(), map.theta_bins.end(), theta);

        size_t p_idx = (p_it == map.p_bins.end()) ?
            map.p_bins.size() - 1 : std::distance(map.p_bins.begin(), p_it);
        size_t th_idx = (th_it == map.theta_bins.end()) ?
            map.theta_bins.size() - 1 : std::distance(map.theta_bins.begin(), th_it);

        if (p_idx > 0) --p_idx;
        if (th_idx > 0) --th_idx;

        if (p_idx < map.efficiency.size() && th_idx < map.efficiency[p_idx].size()) {
            return map.efficiency[p_idx][th_idx];
        }

        return 1.0;
    }
};

}  // namespace clas12::framework
