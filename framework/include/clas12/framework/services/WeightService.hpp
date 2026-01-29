#pragma once

#include <cmath>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <clas12/Particle.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/IService.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/services/LoggingService.hpp>

#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>

namespace clas12::framework {

/**
 * @brief Type of weight for categorization
 */
enum class WeightType {
    Luminosity,   ///< Run luminosity normalization
    Acceptance,   ///< Geometric acceptance
    Efficiency,   ///< Reconstruction/selection efficiency
    Trigger,      ///< Trigger efficiency
    PID,          ///< Particle ID efficiency
    Radiative,    ///< Radiative corrections
    Custom        ///< User-defined
};

/**
 * @brief Get string representation of weight type
 */
inline auto weight_type_to_string(WeightType type) -> std::string {
    switch (type) {
        case WeightType::Luminosity: return "luminosity";
        case WeightType::Acceptance: return "acceptance";
        case WeightType::Efficiency: return "efficiency";
        case WeightType::Trigger: return "trigger";
        case WeightType::PID: return "pid";
        case WeightType::Radiative: return "radiative";
        case WeightType::Custom: return "custom";
    }
    return "unknown";
}

/**
 * @brief Container for event weight with uncertainty
 */
struct EventWeight {
    double value = 1.0;           ///< Weight value
    double uncertainty = 0.0;     ///< Absolute uncertainty
    WeightType type = WeightType::Custom;
    std::string name;             ///< Weight identifier

    /// Multiply weights (for combining)
    auto operator*(const EventWeight& other) const -> EventWeight {
        EventWeight result;
        result.value = value * other.value;
        // Propagate uncertainty (relative errors add in quadrature)
        double rel_err1 = (value != 0.0) ? uncertainty / value : 0.0;
        double rel_err2 = (other.value != 0.0) ? other.uncertainty / other.value : 0.0;
        result.uncertainty = result.value * std::sqrt(rel_err1 * rel_err1 + rel_err2 * rel_err2);
        result.type = WeightType::Custom;
        result.name = name + "*" + other.name;
        return result;
    }

    /// Relative uncertainty
    [[nodiscard]] auto relative_uncertainty() const -> double {
        return (value != 0.0) ? uncertainty / value : 0.0;
    }
};

/**
 * @brief 1D efficiency map lookup
 */
class EfficiencyMap1D {
   public:
    EfficiencyMap1D() = default;

    /**
     * @brief Load from ROOT histogram
     */
    auto load(TH1* hist) -> bool {
        if (!hist) return false;

        m_nbins = hist->GetNbinsX();
        m_xmin = hist->GetXaxis()->GetXmin();
        m_xmax = hist->GetXaxis()->GetXmax();
        m_bin_width = (m_xmax - m_xmin) / m_nbins;

        m_values.resize(m_nbins);
        m_errors.resize(m_nbins);

        for (int i = 0; i < m_nbins; ++i) {
            m_values[i] = hist->GetBinContent(i + 1);
            m_errors[i] = hist->GetBinError(i + 1);
        }

        return true;
    }

    /**
     * @brief Lookup efficiency at x
     */
    [[nodiscard]] auto lookup(double x) const -> std::pair<double, double> {
        if (m_values.empty()) return {1.0, 0.0};

        int bin = static_cast<int>((x - m_xmin) / m_bin_width);
        bin = std::clamp(bin, 0, m_nbins - 1);

        return {m_values[bin], m_errors[bin]};
    }

   private:
    std::vector<double> m_values;
    std::vector<double> m_errors;
    int m_nbins = 0;
    double m_xmin = 0.0, m_xmax = 1.0, m_bin_width = 1.0;
};

/**
 * @brief 2D efficiency map lookup
 */
class EfficiencyMap2D {
   public:
    EfficiencyMap2D() = default;

    /**
     * @brief Load from ROOT histogram
     */
    auto load(TH2* hist) -> bool {
        if (!hist) return false;

        m_nbinsx = hist->GetNbinsX();
        m_nbinsy = hist->GetNbinsY();
        m_xmin = hist->GetXaxis()->GetXmin();
        m_xmax = hist->GetXaxis()->GetXmax();
        m_ymin = hist->GetYaxis()->GetXmin();
        m_ymax = hist->GetYaxis()->GetXmax();
        m_bin_width_x = (m_xmax - m_xmin) / m_nbinsx;
        m_bin_width_y = (m_ymax - m_ymin) / m_nbinsy;

        m_values.resize(m_nbinsx * m_nbinsy);
        m_errors.resize(m_nbinsx * m_nbinsy);

        for (int i = 0; i < m_nbinsx; ++i) {
            for (int j = 0; j < m_nbinsy; ++j) {
                int idx = i * m_nbinsy + j;
                m_values[idx] = hist->GetBinContent(i + 1, j + 1);
                m_errors[idx] = hist->GetBinError(i + 1, j + 1);
            }
        }

        return true;
    }

    /**
     * @brief Lookup efficiency at (x, y)
     */
    [[nodiscard]] auto lookup(double x, double y) const -> std::pair<double, double> {
        if (m_values.empty()) return {1.0, 0.0};

        int binx = static_cast<int>((x - m_xmin) / m_bin_width_x);
        int biny = static_cast<int>((y - m_ymin) / m_bin_width_y);
        binx = std::clamp(binx, 0, m_nbinsx - 1);
        biny = std::clamp(biny, 0, m_nbinsy - 1);

        int idx = binx * m_nbinsy + biny;
        return {m_values[idx], m_errors[idx]};
    }

   private:
    std::vector<double> m_values;
    std::vector<double> m_errors;
    int m_nbinsx = 0, m_nbinsy = 0;
    double m_xmin = 0.0, m_xmax = 1.0, m_ymin = 0.0, m_ymax = 1.0;
    double m_bin_width_x = 1.0, m_bin_width_y = 1.0;
};

/**
 * @brief 3D efficiency map lookup
 */
class EfficiencyMap3D {
   public:
    EfficiencyMap3D() = default;

    /**
     * @brief Load from ROOT histogram
     */
    auto load(TH3* hist) -> bool {
        if (!hist) return false;

        m_nbinsx = hist->GetNbinsX();
        m_nbinsy = hist->GetNbinsY();
        m_nbinsz = hist->GetNbinsZ();
        m_xmin = hist->GetXaxis()->GetXmin();
        m_xmax = hist->GetXaxis()->GetXmax();
        m_ymin = hist->GetYaxis()->GetXmin();
        m_ymax = hist->GetYaxis()->GetXmax();
        m_zmin = hist->GetZaxis()->GetXmin();
        m_zmax = hist->GetZaxis()->GetXmax();
        m_bin_width_x = (m_xmax - m_xmin) / m_nbinsx;
        m_bin_width_y = (m_ymax - m_ymin) / m_nbinsy;
        m_bin_width_z = (m_zmax - m_zmin) / m_nbinsz;

        m_values.resize(m_nbinsx * m_nbinsy * m_nbinsz);
        m_errors.resize(m_nbinsx * m_nbinsy * m_nbinsz);

        for (int i = 0; i < m_nbinsx; ++i) {
            for (int j = 0; j < m_nbinsy; ++j) {
                for (int k = 0; k < m_nbinsz; ++k) {
                    int idx = (i * m_nbinsy + j) * m_nbinsz + k;
                    m_values[idx] = hist->GetBinContent(i + 1, j + 1, k + 1);
                    m_errors[idx] = hist->GetBinError(i + 1, j + 1, k + 1);
                }
            }
        }

        return true;
    }

    /**
     * @brief Lookup efficiency at (x, y, z)
     */
    [[nodiscard]] auto lookup(double x, double y, double z) const -> std::pair<double, double> {
        if (m_values.empty()) return {1.0, 0.0};

        int binx = static_cast<int>((x - m_xmin) / m_bin_width_x);
        int biny = static_cast<int>((y - m_ymin) / m_bin_width_y);
        int binz = static_cast<int>((z - m_zmin) / m_bin_width_z);
        binx = std::clamp(binx, 0, m_nbinsx - 1);
        biny = std::clamp(biny, 0, m_nbinsy - 1);
        binz = std::clamp(binz, 0, m_nbinsz - 1);

        int idx = (binx * m_nbinsy + biny) * m_nbinsz + binz;
        return {m_values[idx], m_errors[idx]};
    }

   private:
    std::vector<double> m_values;
    std::vector<double> m_errors;
    int m_nbinsx = 0, m_nbinsy = 0, m_nbinsz = 0;
    double m_xmin = 0.0, m_xmax = 1.0, m_ymin = 0.0, m_ymax = 1.0, m_zmin = 0.0, m_zmax = 1.0;
    double m_bin_width_x = 1.0, m_bin_width_y = 1.0, m_bin_width_z = 1.0;
};

/**
 * @brief Service for event and particle weight management
 *
 * WeightService provides a centralized interface for managing event weights
 * and efficiency corrections in physics analyses. It supports:
 *
 * - Event-level weights (luminosity, trigger, etc.)
 * - Particle-level efficiency corrections
 * - Loading efficiency maps from ROOT histograms
 * - 1D, 2D, and 3D efficiency lookups
 * - Uncertainty propagation
 *
 * Example usage:
 * @code
 * // In algorithm on_initialize():
 * m_weight_svc = &require_service<WeightService>("WeightService");
 *
 * // Load efficiency map
 * m_weight_svc->load_efficiency_map_2d("electron_eff", "efficiency.root", "h_eff_p_theta");
 *
 * // Register event-level weight
 * m_weight_svc->register_event_weight("luminosity", WeightType::Luminosity,
 *     [this](const Event& e) { return m_grl_svc->luminosity_weight(e.run_number()); });
 *
 * // In algorithm process():
 * // Get particle efficiency
 * auto [eff, err] = m_weight_svc->lookup_efficiency_2d("electron_eff", electron.p(), electron.theta());
 *
 * // Get total event weight
 * EventWeight total = m_weight_svc->get_total_weight(event);
 * h_mass->Fill(mass, total.value);
 * @endcode
 */
class WeightService : public Service {
   public:
    /// Static service name for compile-time access
    static constexpr std::string_view service_name() { return "WeightService"; }

    explicit WeightService(std::string name = "WeightService")
        : Service(std::move(name)) {}

    // ========================================================================
    // Service Lifecycle
    // ========================================================================

    auto on_initialize() -> StatusCode override {
        Log::info(this->name(), "WeightService initialized");
        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        Log::info(this->name(), "WeightService finalized");
        Log::info(this->name(), "  Event weight calculators: {}", m_event_weights.size());
        Log::info(this->name(), "  Particle weight calculators: {}", m_particle_weights.size());
        Log::info(this->name(), "  Efficiency maps loaded: {}",
                  m_eff_maps_1d.size() + m_eff_maps_2d.size() + m_eff_maps_3d.size());
        return StatusCode::SUCCESS();
    }

    // ========================================================================
    // Event Weight Registration
    // ========================================================================

    /**
     * @brief Register an event-level weight calculator
     *
     * @param name Weight identifier
     * @param type Weight category
     * @param calculator Function to compute weight
     * @param uncertainty Optional function to compute uncertainty
     */
    auto register_event_weight(const std::string& name, WeightType type,
                               std::function<double(Event&)> calculator,
                               std::function<double(Event&)> uncertainty = nullptr) -> void {
        m_event_weights[name] = {type, std::move(calculator), std::move(uncertainty)};
        Log::debug(this->name(), "Registered event weight: {} ({})",
                   name, weight_type_to_string(type));
    }

    /**
     * @brief Register a particle-level weight calculator
     *
     * @param name Weight identifier
     * @param type Weight category
     * @param calculator Function to compute weight from particle
     * @param uncertainty Optional function to compute uncertainty
     */
    auto register_particle_weight(const std::string& name, WeightType type,
                                  std::function<double(const clas12::Particle&)> calculator,
                                  std::function<double(const clas12::Particle&)> uncertainty = nullptr) -> void {
        m_particle_weights[name] = {type, std::move(calculator), std::move(uncertainty)};
        Log::debug(this->name(), "Registered particle weight: {} ({})",
                   name, weight_type_to_string(type));
    }

    // ========================================================================
    // Efficiency Map Loading
    // ========================================================================

    /**
     * @brief Load 1D efficiency map from ROOT file
     *
     * @param name Map identifier
     * @param root_file Path to ROOT file
     * @param hist_name Name of histogram in file
     * @return StatusCode indicating success
     */
    auto load_efficiency_map_1d(const std::string& name,
                                 const std::string& root_file,
                                 const std::string& hist_name) -> StatusCode {
        std::unique_ptr<TFile> file(TFile::Open(root_file.c_str(), "READ"));
        if (!file || file->IsZombie()) {
            Log::error(this->name(), "Failed to open file: {}", root_file);
            return StatusCode::FAILURE("Failed to open efficiency file");
        }

        TH1* hist = dynamic_cast<TH1*>(file->Get(hist_name.c_str()));
        if (!hist) {
            Log::error(this->name(), "Histogram {} not found in {}", hist_name, root_file);
            return StatusCode::FAILURE("Histogram not found");
        }

        auto map = std::make_unique<EfficiencyMap1D>();
        if (!map->load(hist)) {
            return StatusCode::FAILURE("Failed to load histogram data");
        }

        m_eff_maps_1d[name] = std::move(map);
        Log::info(this->name(), "Loaded 1D efficiency map: {} from {}:{}",
                  name, root_file, hist_name);

        return StatusCode::SUCCESS();
    }

    /**
     * @brief Load 2D efficiency map from ROOT file
     */
    auto load_efficiency_map_2d(const std::string& name,
                                 const std::string& root_file,
                                 const std::string& hist_name) -> StatusCode {
        std::unique_ptr<TFile> file(TFile::Open(root_file.c_str(), "READ"));
        if (!file || file->IsZombie()) {
            Log::error(this->name(), "Failed to open file: {}", root_file);
            return StatusCode::FAILURE("Failed to open efficiency file");
        }

        TH2* hist = dynamic_cast<TH2*>(file->Get(hist_name.c_str()));
        if (!hist) {
            Log::error(this->name(), "Histogram {} not found in {}", hist_name, root_file);
            return StatusCode::FAILURE("Histogram not found");
        }

        auto map = std::make_unique<EfficiencyMap2D>();
        if (!map->load(hist)) {
            return StatusCode::FAILURE("Failed to load histogram data");
        }

        m_eff_maps_2d[name] = std::move(map);
        Log::info(this->name(), "Loaded 2D efficiency map: {} from {}:{}",
                  name, root_file, hist_name);

        return StatusCode::SUCCESS();
    }

    /**
     * @brief Load 3D efficiency map from ROOT file
     */
    auto load_efficiency_map_3d(const std::string& name,
                                 const std::string& root_file,
                                 const std::string& hist_name) -> StatusCode {
        std::unique_ptr<TFile> file(TFile::Open(root_file.c_str(), "READ"));
        if (!file || file->IsZombie()) {
            Log::error(this->name(), "Failed to open file: {}", root_file);
            return StatusCode::FAILURE("Failed to open efficiency file");
        }

        TH3* hist = dynamic_cast<TH3*>(file->Get(hist_name.c_str()));
        if (!hist) {
            Log::error(this->name(), "Histogram {} not found in {}", hist_name, root_file);
            return StatusCode::FAILURE("Histogram not found");
        }

        auto map = std::make_unique<EfficiencyMap3D>();
        if (!map->load(hist)) {
            return StatusCode::FAILURE("Failed to load histogram data");
        }

        m_eff_maps_3d[name] = std::move(map);
        Log::info(this->name(), "Loaded 3D efficiency map: {} from {}:{}",
                  name, root_file, hist_name);

        return StatusCode::SUCCESS();
    }

    // ========================================================================
    // Weight Calculation
    // ========================================================================

    /**
     * @brief Get a specific event weight
     *
     * @param name Weight identifier
     * @param event Current event
     * @return EventWeight with value and uncertainty
     */
    [[nodiscard]] auto get_weight(const std::string& name, Event& event) -> EventWeight {
        EventWeight result;
        result.name = name;

        auto it = m_event_weights.find(name);
        if (it == m_event_weights.end()) {
            Log::warning(this->name(), "Event weight '{}' not found", name);
            return result;  // Returns default (1.0)
        }

        const auto& [type, calc, uncert] = it->second;
        result.type = type;
        result.value = calc(event);
        if (uncert) {
            result.uncertainty = uncert(event);
        }

        return result;
    }

    /**
     * @brief Get total event weight (product of all registered weights)
     *
     * @param event Current event
     * @return Combined EventWeight
     */
    [[nodiscard]] auto get_total_weight(Event& event) -> EventWeight {
        EventWeight total;
        total.name = "total";
        total.value = 1.0;
        total.uncertainty = 0.0;

        for (auto& [name, weight_info] : m_event_weights) {
            const auto& [type, calc, uncert] = weight_info;
            double w = calc(event);
            double u = uncert ? uncert(event) : 0.0;

            // Combine uncertainties (relative errors in quadrature)
            double rel_err_prev = (total.value != 0.0) ? total.uncertainty / total.value : 0.0;
            double rel_err_new = (w != 0.0) ? u / w : 0.0;

            total.value *= w;
            total.uncertainty = total.value * std::sqrt(rel_err_prev * rel_err_prev +
                                                        rel_err_new * rel_err_new);
        }

        return total;
    }

    /**
     * @brief Get total event weight for specific weight types
     *
     * @param event Current event
     * @param types Weight types to include
     * @return Combined EventWeight
     */
    [[nodiscard]] auto get_weight_by_types(Event& event,
                                            const std::vector<WeightType>& types) -> EventWeight {
        EventWeight total;
        total.name = "combined";
        total.value = 1.0;
        total.uncertainty = 0.0;

        for (auto& [name, weight_info] : m_event_weights) {
            const auto& [type, calc, uncert] = weight_info;

            // Check if this type should be included
            bool include = false;
            for (WeightType t : types) {
                if (type == t) {
                    include = true;
                    break;
                }
            }

            if (!include) continue;

            double w = calc(event);
            double u = uncert ? uncert(event) : 0.0;

            double rel_err_prev = (total.value != 0.0) ? total.uncertainty / total.value : 0.0;
            double rel_err_new = (w != 0.0) ? u / w : 0.0;

            total.value *= w;
            total.uncertainty = total.value * std::sqrt(rel_err_prev * rel_err_prev +
                                                        rel_err_new * rel_err_new);
        }

        return total;
    }

    /**
     * @brief Get particle efficiency weight
     *
     * @param name Weight identifier
     * @param particle Particle to evaluate
     * @return Weight value (1.0 if not found)
     */
    [[nodiscard]] auto get_particle_weight(const std::string& name,
                                            const clas12::Particle& particle) -> double {
        auto it = m_particle_weights.find(name);
        if (it == m_particle_weights.end()) {
            return 1.0;
        }
        return std::get<1>(it->second)(particle);
    }

    /**
     * @brief Get particle efficiency with uncertainty
     */
    [[nodiscard]] auto get_particle_weight_with_error(const std::string& name,
                                                       const clas12::Particle& particle)
        -> std::pair<double, double> {
        auto it = m_particle_weights.find(name);
        if (it == m_particle_weights.end()) {
            return {1.0, 0.0};
        }

        const auto& [type, calc, uncert] = it->second;
        double w = calc(particle);
        double u = uncert ? uncert(particle) : 0.0;
        return {w, u};
    }

    // ========================================================================
    // Efficiency Lookup
    // ========================================================================

    /**
     * @brief Lookup 1D efficiency
     *
     * @param map_name Map identifier
     * @param x Variable value
     * @return Pair of (efficiency, uncertainty)
     */
    [[nodiscard]] auto lookup_efficiency_1d(const std::string& map_name, double x) const
        -> std::pair<double, double> {
        auto it = m_eff_maps_1d.find(map_name);
        if (it == m_eff_maps_1d.end()) {
            return {1.0, 0.0};
        }
        return it->second->lookup(x);
    }

    /**
     * @brief Lookup 2D efficiency
     *
     * @param map_name Map identifier
     * @param x First variable value
     * @param y Second variable value
     * @return Pair of (efficiency, uncertainty)
     */
    [[nodiscard]] auto lookup_efficiency_2d(const std::string& map_name, double x, double y) const
        -> std::pair<double, double> {
        auto it = m_eff_maps_2d.find(map_name);
        if (it == m_eff_maps_2d.end()) {
            return {1.0, 0.0};
        }
        return it->second->lookup(x, y);
    }

    /**
     * @brief Lookup 3D efficiency
     *
     * @param map_name Map identifier
     * @param x First variable value
     * @param y Second variable value
     * @param z Third variable value
     * @return Pair of (efficiency, uncertainty)
     */
    [[nodiscard]] auto lookup_efficiency_3d(const std::string& map_name,
                                             double x, double y, double z) const
        -> std::pair<double, double> {
        auto it = m_eff_maps_3d.find(map_name);
        if (it == m_eff_maps_3d.end()) {
            return {1.0, 0.0};
        }
        return it->second->lookup(x, y, z);
    }

    // ========================================================================
    // Convenience Methods
    // ========================================================================

    /**
     * @brief Register efficiency from 1D map as particle weight
     *
     * @param weight_name Weight identifier
     * @param map_name Map identifier
     * @param var_func Function to extract variable from particle
     */
    auto register_efficiency_from_map_1d(
        const std::string& weight_name,
        const std::string& map_name,
        std::function<double(const clas12::Particle&)> var_func) -> void {

        register_particle_weight(weight_name, WeightType::Efficiency,
            [this, map_name, var_func](const clas12::Particle& p) -> double {
                auto [eff, err] = lookup_efficiency_1d(map_name, var_func(p));
                return eff;
            },
            [this, map_name, var_func](const clas12::Particle& p) -> double {
                auto [eff, err] = lookup_efficiency_1d(map_name, var_func(p));
                return err;
            });
    }

    /**
     * @brief Register efficiency from 2D map as particle weight
     *
     * @param weight_name Weight identifier
     * @param map_name Map identifier
     * @param x_func Function to extract x variable from particle
     * @param y_func Function to extract y variable from particle
     */
    auto register_efficiency_from_map_2d(
        const std::string& weight_name,
        const std::string& map_name,
        std::function<double(const clas12::Particle&)> x_func,
        std::function<double(const clas12::Particle&)> y_func) -> void {

        register_particle_weight(weight_name, WeightType::Efficiency,
            [this, map_name, x_func, y_func](const clas12::Particle& p) -> double {
                auto [eff, err] = lookup_efficiency_2d(map_name, x_func(p), y_func(p));
                return eff;
            },
            [this, map_name, x_func, y_func](const clas12::Particle& p) -> double {
                auto [eff, err] = lookup_efficiency_2d(map_name, x_func(p), y_func(p));
                return err;
            });
    }

    /**
     * @brief Check if an efficiency map exists
     */
    [[nodiscard]] auto has_efficiency_map(const std::string& name) const -> bool {
        return m_eff_maps_1d.count(name) > 0 ||
               m_eff_maps_2d.count(name) > 0 ||
               m_eff_maps_3d.count(name) > 0;
    }

    /**
     * @brief Check if an event weight is registered
     */
    [[nodiscard]] auto has_event_weight(const std::string& name) const -> bool {
        return m_event_weights.count(name) > 0;
    }

    /**
     * @brief Check if a particle weight is registered
     */
    [[nodiscard]] auto has_particle_weight(const std::string& name) const -> bool {
        return m_particle_weights.count(name) > 0;
    }

    /**
     * @brief Get list of registered event weight names
     */
    [[nodiscard]] auto event_weight_names() const -> std::vector<std::string> {
        std::vector<std::string> names;
        names.reserve(m_event_weights.size());
        for (const auto& [name, info] : m_event_weights) {
            names.push_back(name);
        }
        return names;
    }

    /**
     * @brief Get list of registered particle weight names
     */
    [[nodiscard]] auto particle_weight_names() const -> std::vector<std::string> {
        std::vector<std::string> names;
        names.reserve(m_particle_weights.size());
        for (const auto& [name, info] : m_particle_weights) {
            names.push_back(name);
        }
        return names;
    }

   private:
    // Event-level weight calculators
    // Tuple: (type, calculator, uncertainty_calculator)
    using EventWeightInfo = std::tuple<WeightType,
                                       std::function<double(Event&)>,
                                       std::function<double(Event&)>>;
    std::map<std::string, EventWeightInfo> m_event_weights;

    // Particle-level weight calculators
    using ParticleWeightInfo = std::tuple<WeightType,
                                          std::function<double(const clas12::Particle&)>,
                                          std::function<double(const clas12::Particle&)>>;
    std::map<std::string, ParticleWeightInfo> m_particle_weights;

    // Efficiency maps
    std::map<std::string, std::unique_ptr<EfficiencyMap1D>> m_eff_maps_1d;
    std::map<std::string, std::unique_ptr<EfficiencyMap2D>> m_eff_maps_2d;
    std::map<std::string, std::unique_ptr<EfficiencyMap3D>> m_eff_maps_3d;
};

}  // namespace clas12::framework
