#pragma once

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

// TMVA headers (conditionally included)
#ifdef USE_TMVA
#include <TMVA/Reader.h>
#endif

namespace clas12::framework {

/**
 * @brief Variable extractor for MVA evaluation
 *
 * Defines how to extract a feature value from a particle.
 */
struct MVAVariable {
    std::string name;                                    ///< Variable name (must match training)
    std::string expression;                              ///< Human-readable expression
    std::function<float(const clas12::Particle&)> extractor;  ///< Function to compute value
    mutable float value = 0.0f;                          ///< Current value (for TMVA binding)

    MVAVariable(std::string n, std::string expr,
                std::function<float(const clas12::Particle&)> ext)
        : name(std::move(n))
        , expression(std::move(expr))
        , extractor(std::move(ext)) {}
};

/**
 * @brief MVA model configuration
 */
struct MVAModelConfig {
    std::string name;               ///< Model identifier
    std::string weights_file;       ///< Path to weights XML file
    std::string method_name;        ///< TMVA method name (e.g., "BDT")
    float default_threshold = 0.0f; ///< Default cut threshold
    std::vector<MVAVariable> variables;  ///< Variables used by this model
};

/**
 * @brief Individual MVA model reader
 *
 * Wraps a TMVA::Reader or custom implementation for a single model.
 */
class MVAReader {
   public:
    MVAReader() = default;

    /**
     * @brief Load TMVA weights file
     *
     * @param weights_file Path to XML weights file
     * @param method_name TMVA method name
     * @return True if successful
     */
    auto load_tmva(const std::string& weights_file,
                   const std::string& method_name) -> bool {
#ifdef USE_TMVA
        if (m_variables.empty()) {
            Log::error("MVAReader", "No variables defined before loading TMVA");
            return false;
        }

        m_reader = std::make_unique<TMVA::Reader>("!Color:!Silent");

        // Add variables to reader
        for (auto& var : m_variables) {
            m_reader->AddVariable(var.name, &var.value);
        }

        // Book the method
        m_reader->BookMVA(method_name, weights_file);
        m_method_name = method_name;
        m_is_loaded = true;

        Log::info("MVAReader", "Loaded TMVA model: {} from {}", method_name, weights_file);
        return true;
#else
        Log::warning("MVAReader", "TMVA support not compiled - using fallback");
        m_method_name = method_name;
        m_weights_file = weights_file;
        m_is_loaded = false;
        return false;
#endif
    }

    /**
     * @brief Add a variable to the reader
     *
     * Variables must be added in the same order as training.
     *
     * @param name Variable name (must match training)
     * @param expression Human-readable description
     * @param extractor Function to compute variable from particle
     */
    auto add_variable(const std::string& name,
                      const std::string& expression,
                      std::function<float(const clas12::Particle&)> extractor) -> void {
        m_variables.emplace_back(name, expression, std::move(extractor));
    }

    /**
     * @brief Add standard CLAS12 electron ID variables
     *
     * Adds the common variables used for electron identification.
     */
    auto add_standard_electron_variables() -> void {
        add_variable("p", "momentum", [](const clas12::Particle& p) {
            return static_cast<float>(p.p());
        });
        add_variable("theta", "polar angle", [](const clas12::Particle& p) {
            return static_cast<float>(p.theta());
        });
        add_variable("phi", "azimuthal angle", [](const clas12::Particle& p) {
            return static_cast<float>(p.phi());
        });
        add_variable("vz", "vertex z", [](const clas12::Particle& p) {
            return static_cast<float>(p.vz());
        });
    }

    /**
     * @brief Evaluate MVA for a particle
     *
     * @param particle Particle to evaluate
     * @return MVA output value
     */
    [[nodiscard]] auto evaluate(const clas12::Particle& particle) -> float {
        // Update variable values
        for (auto& var : m_variables) {
            var.value = var.extractor(particle);
        }

#ifdef USE_TMVA
        if (m_is_loaded && m_reader) {
            return m_reader->EvaluateMVA(m_method_name);
        }
#endif
        // Fallback: return 0 if TMVA not available
        return 0.0f;
    }

    /**
     * @brief Evaluate MVA with explicit variable values
     *
     * @param values Variable values in order
     * @return MVA output value
     */
    [[nodiscard]] auto evaluate(const std::vector<float>& values) -> float {
        if (values.size() != m_variables.size()) {
            Log::warning("MVAReader", "Variable count mismatch: {} vs {}",
                         values.size(), m_variables.size());
            return 0.0f;
        }

        for (size_t i = 0; i < values.size(); ++i) {
            m_variables[i].value = values[i];
        }

#ifdef USE_TMVA
        if (m_is_loaded && m_reader) {
            return m_reader->EvaluateMVA(m_method_name);
        }
#endif
        return 0.0f;
    }

    /**
     * @brief Check if model is loaded
     */
    [[nodiscard]] auto is_loaded() const -> bool { return m_is_loaded; }

    /**
     * @brief Get number of variables
     */
    [[nodiscard]] auto num_variables() const -> size_t { return m_variables.size(); }

    /**
     * @brief Get variable names
     */
    [[nodiscard]] auto variable_names() const -> std::vector<std::string> {
        std::vector<std::string> names;
        names.reserve(m_variables.size());
        for (const auto& var : m_variables) {
            names.push_back(var.name);
        }
        return names;
    }

   private:
#ifdef USE_TMVA
    std::unique_ptr<TMVA::Reader> m_reader;
#endif
    std::vector<MVAVariable> m_variables;
    std::string m_method_name;
    std::string m_weights_file;
    bool m_is_loaded = false;
};

/**
 * @brief Service for MVA/BDT-based particle identification and selection
 *
 * MVAService provides a centralized interface for loading and evaluating
 * machine learning models (BDT, MLP, etc.) for particle identification
 * and signal selection. It supports:
 *
 * - Loading TMVA weight files
 * - Custom variable definitions
 * - Multiple models for different purposes
 * - Threshold-based selection
 *
 * Example usage:
 * @code
 * // In algorithm on_initialize():
 * m_mva_svc = &require_service<MVAService>("MVAService");
 *
 * // Register model with variables
 * auto& reader = m_mva_svc->create_reader("electron_id");
 * reader.add_variable("p", "momentum", [](const Particle& p) { return p.p(); });
 * reader.add_variable("theta", "theta", [](const Particle& p) { return p.theta(); });
 * m_mva_svc->load_model("electron_id", "weights/electron_bdt.xml", "BDT");
 *
 * // In algorithm process():
 * for (const auto& electron : electrons) {
 *     float bdt_score = m_mva_svc->evaluate("electron_id", electron);
 *     if (m_mva_svc->passes_cut("electron_id", electron, 0.85)) {
 *         // Selected electron
 *     }
 * }
 * @endcode
 */
class MVAService : public Service {
   public:
    /// Static service name for compile-time access
    static constexpr std::string_view service_name() { return "MVAService"; }

    explicit MVAService(std::string name = "MVAService")
        : Service(std::move(name)) {}

    // ========================================================================
    // Service Lifecycle
    // ========================================================================

    auto on_initialize() -> StatusCode override {
        Log::info(this->name(), "MVAService initialized");

        // Load any pre-configured models from properties
        if (!m_electron_id_weights.get().empty()) {
            auto& reader = create_reader("electron_id");
            reader.add_standard_electron_variables();
            load_model("electron_id", m_electron_id_weights.get(), "BDT");
            set_threshold("electron_id", m_electron_id_threshold.get());
        }

        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        Log::info(this->name(), "MVAService finalized");
        Log::info(this->name(), "  Models loaded: {}", m_readers.size());
        return StatusCode::SUCCESS();
    }

    // ========================================================================
    // Configuration Properties
    // ========================================================================

    /// Electron ID weights file
    Property<std::string> m_electron_id_weights{"electron_id_weights", "",
                                                 "Path to electron ID BDT weights"};

    /// Electron ID threshold
    Property<float> m_electron_id_threshold{"electron_id_threshold", 0.0f,
                                            "Electron ID BDT cut threshold"};

    // ========================================================================
    // Model Registration
    // ========================================================================

    /**
     * @brief Create a new MVA reader
     *
     * Creates a reader that can have variables added before loading weights.
     *
     * @param name Model identifier
     * @return Reference to the created reader
     */
    auto create_reader(const std::string& name) -> MVAReader& {
        m_readers[name] = std::make_unique<MVAReader>();
        return *m_readers[name];
    }

    /**
     * @brief Load a model from weights file
     *
     * The reader must already exist with variables defined.
     *
     * @param name Model identifier
     * @param weights_file Path to weights XML file
     * @param method_name TMVA method name
     * @return True if successful
     */
    auto load_model(const std::string& name,
                    const std::string& weights_file,
                    const std::string& method_name) -> bool {
        auto it = m_readers.find(name);
        if (it == m_readers.end()) {
            Log::error(this->name(), "Reader '{}' not found. Create it first.", name);
            return false;
        }

        bool success = it->second->load_tmva(weights_file, method_name);
        if (success) {
            Log::info(this->name(), "Loaded model '{}' from {}", name, weights_file);
        }
        return success;
    }

    /**
     * @brief Register a complete model configuration
     *
     * Creates reader, adds variables, and loads weights in one call.
     *
     * @param config Model configuration
     * @return True if successful
     */
    auto register_model(const MVAModelConfig& config) -> bool {
        auto& reader = create_reader(config.name);

        for (const auto& var : config.variables) {
            reader.add_variable(var.name, var.expression, var.extractor);
        }

        bool success = reader.load_tmva(config.weights_file, config.method_name);
        if (success) {
            set_threshold(config.name, config.default_threshold);
        }
        return success;
    }

    /**
     * @brief Set threshold for a model
     *
     * @param name Model identifier
     * @param threshold Cut value
     */
    auto set_threshold(const std::string& name, float threshold) -> void {
        m_thresholds[name] = threshold;
    }

    /**
     * @brief Get threshold for a model
     */
    [[nodiscard]] auto get_threshold(const std::string& name) const -> float {
        auto it = m_thresholds.find(name);
        return (it != m_thresholds.end()) ? it->second : 0.0f;
    }

    // ========================================================================
    // Evaluation
    // ========================================================================

    /**
     * @brief Evaluate MVA for a particle
     *
     * @param model_name Model identifier
     * @param particle Particle to evaluate
     * @return MVA output value, or 0 if model not found
     */
    [[nodiscard]] auto evaluate(const std::string& model_name,
                                 const clas12::Particle& particle) -> float {
        auto it = m_readers.find(model_name);
        if (it == m_readers.end()) {
            Log::warning(this->name(), "Model '{}' not found", model_name);
            return 0.0f;
        }
        return it->second->evaluate(particle);
    }

    /**
     * @brief Check if particle passes MVA cut
     *
     * @param model_name Model identifier
     * @param particle Particle to evaluate
     * @param threshold Optional explicit threshold (uses stored if not provided)
     * @return True if MVA > threshold
     */
    [[nodiscard]] auto passes_cut(const std::string& model_name,
                                   const clas12::Particle& particle,
                                   std::optional<float> threshold = std::nullopt) -> bool {
        float score = evaluate(model_name, particle);
        float cut = threshold.value_or(get_threshold(model_name));
        return score > cut;
    }

    /**
     * @brief Evaluate MVA with explicit variable values
     *
     * @param model_name Model identifier
     * @param values Variable values in training order
     * @return MVA output value
     */
    [[nodiscard]] auto evaluate(const std::string& model_name,
                                 const std::vector<float>& values) -> float {
        auto it = m_readers.find(model_name);
        if (it == m_readers.end()) {
            Log::warning(this->name(), "Model '{}' not found", model_name);
            return 0.0f;
        }
        return it->second->evaluate(values);
    }

    // ========================================================================
    // Model Information
    // ========================================================================

    /**
     * @brief Check if a model is registered
     */
    [[nodiscard]] auto has_model(const std::string& name) const -> bool {
        return m_readers.count(name) > 0;
    }

    /**
     * @brief Check if a model is loaded (weights available)
     */
    [[nodiscard]] auto is_model_loaded(const std::string& name) const -> bool {
        auto it = m_readers.find(name);
        return (it != m_readers.end()) && it->second->is_loaded();
    }

    /**
     * @brief Get list of registered model names
     */
    [[nodiscard]] auto model_names() const -> std::vector<std::string> {
        std::vector<std::string> names;
        names.reserve(m_readers.size());
        for (const auto& [name, reader] : m_readers) {
            names.push_back(name);
        }
        return names;
    }

    /**
     * @brief Get variable names for a model
     */
    [[nodiscard]] auto variable_names(const std::string& model_name) const
        -> std::vector<std::string> {
        auto it = m_readers.find(model_name);
        if (it == m_readers.end()) {
            return {};
        }
        return it->second->variable_names();
    }

    /**
     * @brief Get access to a reader (for advanced usage)
     */
    [[nodiscard]] auto get_reader(const std::string& name) -> MVAReader* {
        auto it = m_readers.find(name);
        return (it != m_readers.end()) ? it->second.get() : nullptr;
    }

    [[nodiscard]] auto get_reader(const std::string& name) const -> const MVAReader* {
        auto it = m_readers.find(name);
        return (it != m_readers.end()) ? it->second.get() : nullptr;
    }

   private:
    std::map<std::string, std::unique_ptr<MVAReader>> m_readers;
    std::map<std::string, float> m_thresholds;
};

}  // namespace clas12::framework
