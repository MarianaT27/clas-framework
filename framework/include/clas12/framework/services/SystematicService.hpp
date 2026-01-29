#pragma once

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <clas12/Particle.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/IService.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/SystematicVariation.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>

namespace clas12::framework {

/**
 * @brief Weight-based systematic variation
 *
 * Instead of modifying particle momenta, applies event weights.
 * Useful for efficiency corrections, trigger effects, etc.
 */
struct WeightVariation {
    std::string name;                             ///< Variation name
    std::string description;                      ///< Human-readable description
    std::function<double(Event&)> weight_func;    ///< Function to compute weight
    std::function<double(Event&)> uncertainty_func; ///< Optional uncertainty

    /// Construct weight variation
    WeightVariation(std::string n, std::string desc,
                    std::function<double(Event&)> wf,
                    std::function<double(Event&)> uf = nullptr)
        : name(std::move(n))
        , description(std::move(desc))
        , weight_func(std::move(wf))
        , uncertainty_func(std::move(uf)) {}
};

/**
 * @brief Service wrapper for systematic uncertainty management
 *
 * SystematicService provides a centralized interface for managing
 * systematic variations in physics analyses. It supports:
 *
 * - Momentum/energy scale variations
 * - Resolution smearing
 * - Weight-based variations (efficiency, trigger, etc.)
 * - Automatic histogram cloning for all variations
 *
 * Example usage:
 * @code
 * // In algorithm on_initialize():
 * m_sys_svc = &require_service<SystematicService>("SystematicService");
 *
 * // Register variations
 * m_sys_svc->add_energy_scale(0.01);  // 1% energy scale
 * m_sys_svc->add_momentum_resolution(0.005);  // 0.5% resolution
 *
 * // Clone histograms for all variations
 * m_sys_svc->clone_for_variations(m_hist_svc, "/Electron/momentum");
 *
 * // In algorithm process():
 * for (const auto& var_name : m_sys_svc->all_variations()) {
 *     m_sys_svc->set_variation(var_name);
 *
 *     // Get varied particle
 *     auto electron_varied = m_sys_svc->vary(electron);
 *
 *     // Get weight
 *     double weight = m_sys_svc->weight(event);
 *
 *     // Fill histogram for this variation
 *     std::string hist_path = "/Electron/momentum_" + var_name;
 *     m_hist_svc->get_hist1D(hist_path)->Fill(electron_varied.p(), weight);
 * }
 * @endcode
 */
class SystematicService : public Service {
   public:
    /// Static service name for compile-time access
    static constexpr std::string_view service_name() { return "SystematicService"; }

    explicit SystematicService(std::string name = "SystematicService")
        : Service(std::move(name)) {}

    // ========================================================================
    // Service Lifecycle
    // ========================================================================

    auto on_initialize() -> StatusCode override {
        // Parse enabled variations from comma-separated string
        parse_enabled_variations(m_enabled_variations_str.get());

        Log::info(this->name(), "SystematicService initialized");
        Log::info(this->name(), "  Variations registered: {}", m_sys_manager.variations().size());

        if (!m_enabled_variations.empty()) {
            Log::info(this->name(), "  Enabled variations: {}",
                      fmt::join(m_enabled_variations, ", "));
        }

        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        Log::info(this->name(), "SystematicService finalized");
        return StatusCode::SUCCESS();
    }

    // ========================================================================
    // Configuration Properties
    // ========================================================================

    /// Energy scale uncertainty (per sigma)
    Property<double> m_energy_scale_sigma{"energy_scale", 0.01,
                                          "Energy scale uncertainty per sigma"};

    /// Momentum resolution uncertainty (per sigma)
    Property<double> m_momentum_res_sigma{"momentum_resolution", 0.005,
                                          "Momentum resolution uncertainty per sigma"};

    /// Comma-separated list of enabled variations
    Property<std::string> m_enabled_variations_str{"enabled_variations", "",
                                                   "Enabled variations (comma-separated)"};

    // ========================================================================
    // Variation Registration
    // ========================================================================

    /**
     * @brief Add energy scale systematic
     *
     * Creates "energy_scale_up" and "energy_scale_down" variations.
     *
     * @param sigma Magnitude of variation (default from property)
     */
    auto add_energy_scale(double sigma = 0.0) -> void {
        if (sigma <= 0.0) sigma = m_energy_scale_sigma.get();

        m_sys_manager.add_variation("energy_scale_up", VariationType::EnergyScale, +1.0);
        m_sys_manager.add_variation("energy_scale_down", VariationType::EnergyScale, -1.0);

        m_variation_names.insert("energy_scale_up");
        m_variation_names.insert("energy_scale_down");
    }

    /**
     * @brief Add momentum resolution systematic
     *
     * Creates "momentum_res_up" and "momentum_res_down" variations.
     *
     * @param sigma Magnitude of variation (default from property)
     */
    auto add_momentum_resolution(double sigma = 0.0) -> void {
        if (sigma <= 0.0) sigma = m_momentum_res_sigma.get();

        m_sys_manager.add_variation("momentum_res_up", VariationType::MomentumScale, +1.0);
        m_sys_manager.add_variation("momentum_res_down", VariationType::MomentumScale, -1.0);

        m_variation_names.insert("momentum_res_up");
        m_variation_names.insert("momentum_res_down");
    }

    /**
     * @brief Add a weight-based systematic variation
     *
     * @param name Variation name
     * @param func Weight function
     * @param description Human-readable description
     */
    auto add_weight_variation(const std::string& name,
                              std::function<double(Event&)> func,
                              const std::string& description = "") -> void {
        m_weight_variations[name] = WeightVariation(name, description, std::move(func));
        m_variation_names.insert(name);
    }

    /**
     * @brief Add efficiency variation
     *
     * @param name Variation name
     * @param func Function returning efficiency weight
     */
    auto add_efficiency_variation(const std::string& name,
                                  std::function<double(Event&)> func) -> void {
        add_weight_variation(name, std::move(func), "Efficiency variation: " + name);
    }

    /**
     * @brief Register a custom particle variation function
     *
     * @param name Variation name
     * @param func Function that modifies a particle
     */
    auto register_custom_variation(const std::string& name,
                                   std::function<clas12::Particle(const clas12::Particle&)> func)
        -> void {
        m_sys_manager.register_custom_variation(name, std::move(func));
        m_variation_names.insert(name);
    }

    // ========================================================================
    // Variation Management
    // ========================================================================

    /**
     * @brief Set current variation for processing
     *
     * @param name Variation name (empty string or "nominal" for nominal)
     */
    auto set_variation(const std::string& name) -> void {
        m_current_variation = (name == "nominal") ? "" : name;
        m_sys_manager.set_current_variation(m_current_variation);
    }

    /**
     * @brief Get current variation name
     */
    [[nodiscard]] auto current_variation() const -> const std::string& {
        return m_current_variation;
    }

    /**
     * @brief Check if currently running nominal
     */
    [[nodiscard]] auto is_nominal() const -> bool {
        return m_current_variation.empty();
    }

    /**
     * @brief Get all registered variation names
     */
    [[nodiscard]] auto all_variations() const -> std::vector<std::string> {
        std::vector<std::string> result;
        result.push_back("nominal");  // Always include nominal

        for (const auto& name : m_variation_names) {
            result.push_back(name);
        }

        return result;
    }

    /**
     * @brief Get enabled variation names (subset of all)
     */
    [[nodiscard]] auto enabled_variations() const -> std::vector<std::string> {
        if (m_enabled_variations.empty()) {
            return all_variations();  // All enabled if none specified
        }

        std::vector<std::string> result;
        result.push_back("nominal");

        for (const auto& name : m_enabled_variations) {
            if (m_variation_names.count(name)) {
                result.push_back(name);
            }
        }

        return result;
    }

    // ========================================================================
    // Variation Application
    // ========================================================================

    /**
     * @brief Apply current variation to a particle
     *
     * @param particle Original particle
     * @return Modified particle (or original if nominal)
     */
    [[nodiscard]] auto vary(const clas12::Particle& particle) const -> clas12::Particle {
        if (is_nominal()) {
            return particle;
        }

        // Find matching variation
        for (const auto& var : m_sys_manager.variations()) {
            if (var.name == m_current_variation) {
                return m_sys_manager.vary(particle, var);
            }
        }

        // Try custom variation
        return m_sys_manager.apply_custom(particle, m_current_variation);
    }

    /**
     * @brief Get event weight for current variation
     *
     * @param event Current event
     * @return Weight (1.0 for nominal or if no weight variation)
     */
    [[nodiscard]] auto weight(Event& event) const -> double {
        if (is_nominal()) {
            return 1.0;
        }

        auto it = m_weight_variations.find(m_current_variation);
        if (it != m_weight_variations.end() && it->second.weight_func) {
            return it->second.weight_func(event);
        }

        return 1.0;
    }

    // ========================================================================
    // Histogram Cloning
    // ========================================================================

    /**
     * @brief Clone a histogram for all variations
     *
     * Creates copies of the base histogram with suffixes for each variation.
     * E.g., "/Electron/momentum" -> "/Electron/momentum_nominal",
     *       "/Electron/momentum_energy_scale_up", etc.
     *
     * @param hist_svc Histogram service
     * @param base_path Base histogram path
     */
    auto clone_for_variations(HistogramService* hist_svc,
                              const std::string& base_path) -> void {
        if (!hist_svc) return;

        for (const auto& var_name : enabled_variations()) {
            std::string suffix = "_" + (var_name.empty() ? "nominal" : var_name);
            hist_svc->clone_hist1D_with_suffix(base_path, suffix);
        }
    }

    /**
     * @brief Clone a 2D histogram for all variations
     */
    auto clone_2D_for_variations(HistogramService* hist_svc,
                                  const std::string& base_path) -> void {
        if (!hist_svc) return;

        for (const auto& var_name : enabled_variations()) {
            std::string suffix = "_" + (var_name.empty() ? "nominal" : var_name);
            hist_svc->clone_hist2D_with_suffix(base_path, suffix);
        }
    }

    /**
     * @brief Get histogram path for current variation
     */
    [[nodiscard]] auto get_histogram_path(const std::string& base_path) const -> std::string {
        std::string suffix = is_nominal() ? "_nominal" : "_" + m_current_variation;
        return base_path + suffix;
    }

    // ========================================================================
    // Underlying Manager Access
    // ========================================================================

    /**
     * @brief Get the underlying SystematicManager
     */
    [[nodiscard]] auto manager() -> SystematicManager& {
        return m_sys_manager;
    }

    [[nodiscard]] auto manager() const -> const SystematicManager& {
        return m_sys_manager;
    }

   private:
    SystematicManager m_sys_manager;
    std::map<std::string, WeightVariation> m_weight_variations;
    std::set<std::string> m_variation_names;
    std::set<std::string> m_enabled_variations;
    std::string m_current_variation;

    /**
     * @brief Parse comma-separated enabled variations
     */
    auto parse_enabled_variations(const std::string& str) -> void {
        if (str.empty()) return;

        std::istringstream ss(str);
        std::string var;
        while (std::getline(ss, var, ',')) {
            auto start = var.find_first_not_of(" \t");
            auto end = var.find_last_not_of(" \t");
            if (start != std::string::npos) {
                m_enabled_variations.insert(var.substr(start, end - start + 1));
            }
        }
    }
};

}  // namespace clas12::framework
