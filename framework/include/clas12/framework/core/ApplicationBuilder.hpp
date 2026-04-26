#pragma once

/**
 * @file ApplicationBuilder.hpp
 * @brief Fluent builder for clas12::framework::Application
 *
 * C++ Core Guidelines:
 * - C.50: Use a factory function if you need "virtual behavior" during initialization
 * - F.21: To return multiple "out" values, prefer returning a struct
 * - I.23: Keep the number of function arguments low
 */

#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include <toml++/toml.hpp>

#include <clas12/framework/core/Application.hpp>
#include <clas12/framework/core/Algorithm.hpp>
// Concrete service headers used to be included here so
// add_standard_services_to() could instantiate them inline. That function
// now lives in ApplicationBuilder.cpp; the eleven service headers only
// need to be parsed once during framework_lib compilation instead of once
// per analysis translation unit.

namespace clas12::framework {

/**
 * @brief Fluent builder for constructing Application instances
 *
 * Provides a clean, readable way to configure and build applications:
 *
 * @code
 * auto app = ApplicationBuilder("My Analysis")
 *     .with_input_files(files)
 *     .with_output("output.root")
 *     .with_threads(4)
 *     .with_standard_services()
 *     .add_algorithm<MyAlgorithm>("MyAlg", arg1, arg2)
 *     .build();
 * @endcode
 */
class ApplicationBuilder {
public:
    explicit ApplicationBuilder(std::string name)
        : m_name(std::move(name)) {}

    // ========================================================================
    // Input/Output Configuration
    // ========================================================================

    /// Set input HIPO files
    auto with_input_files(std::vector<std::string> files) -> ApplicationBuilder& {
        m_input_files = std::move(files);
        return *this;
    }

    /// Set single input file
    auto with_input_file(std::string file) -> ApplicationBuilder& {
        m_input_files = {std::move(file)};
        return *this;
    }

    /// Set output ROOT file (used by histogram and ntuple services)
    auto with_output(std::string filename) -> ApplicationBuilder& {
        m_output_file = std::move(filename);
        return *this;
    }

    // ========================================================================
    // Processing Configuration
    // ========================================================================

    /// Set percentage of events to process
    auto with_percentage(double pct) -> ApplicationBuilder& {
        m_percentage = pct;
        return *this;
    }

    /// Set number of processing threads
    auto with_threads(int n) -> ApplicationBuilder& {
        m_num_threads = n;
        return *this;
    }

    /// Attach the parsed TOML config table. The Application will look
    /// up `[<algorithm.name()>]` for every registered algorithm and
    /// apply matching `Property` values via
    /// `PropertyMixin::configure_from_toml`.
    auto with_config(std::shared_ptr<const toml::table> config) -> ApplicationBuilder& {
        m_config = std::move(config);
        return *this;
    }

    // ========================================================================
    // Service Configuration
    // ========================================================================

    /// Add standard services (logging, histogram, ntuple, conditions, etc.)
    auto with_standard_services() -> ApplicationBuilder& {
        m_add_standard_services = true;
        return *this;
    }

    /// Add a custom service
    template <typename ServiceT, typename... Args>
    auto add_service(Args&&... args) -> ApplicationBuilder& {
        m_service_factories.push_back([... args = std::forward<Args>(args)](ServiceManager& mgr) mutable {
            mgr.add_service(std::make_unique<ServiceT>(std::forward<Args>(args)...));
        });
        return *this;
    }

    /// Configure a service after creation
    template <typename ServiceT>
    auto configure_service(std::function<void(ServiceT&)> configurator) -> ApplicationBuilder& {
        m_service_configurators.push_back([configurator = std::move(configurator)](ServiceManager& mgr) {
            if (auto* svc = mgr.get_service<ServiceT>(ServiceT{}.name())) {
                configurator(*svc);
            }
        });
        return *this;
    }

    // ========================================================================
    // Algorithm Configuration
    // ========================================================================

    /// Add an algorithm by type with constructor arguments
    template <typename AlgT, typename... Args>
    auto add_algorithm(Args&&... args) -> ApplicationBuilder& {
        // Use a tuple to capture the arguments, then apply them later
        auto captured_args = std::make_tuple(std::forward<Args>(args)...);
        m_algorithm_factories.push_back([captured_args = std::move(captured_args)]() mutable {
            return std::apply([](auto&&... a) {
                return std::make_unique<AlgT>(std::forward<decltype(a)>(a)...);
            }, std::move(captured_args));
        });
        return *this;
    }

    /// Add a pre-constructed algorithm
    auto add_algorithm(std::unique_ptr<Algorithm> alg) -> ApplicationBuilder& {
        auto ptr = alg.release();
        m_algorithm_factories.push_back([ptr]() {
            return std::unique_ptr<Algorithm>(ptr);
        });
        return *this;
    }

    // ========================================================================
    // Metadata Configuration
    // ========================================================================

    /// Set analysis version
    auto with_version(std::string version) -> ApplicationBuilder& {
        m_version = std::move(version);
        return *this;
    }

    /// Set beam energy
    auto with_beam_energy(double energy) -> ApplicationBuilder& {
        m_beam_energy = energy;
        return *this;
    }

    /// Set target name
    auto with_target(std::string target) -> ApplicationBuilder& {
        m_target = std::move(target);
        return *this;
    }

    // ========================================================================
    // Build
    // ========================================================================

    /// Build and return the configured Application
    [[nodiscard]] auto build() -> Application;

private:
    std::string m_name;
    std::vector<std::string> m_input_files;
    std::string m_output_file = "output.root";
    double m_percentage = 100.0;
    int m_num_threads = 1;
    bool m_add_standard_services = false;
    std::shared_ptr<const toml::table> m_config;

    // Metadata
    std::string m_version = "1.0";
    double m_beam_energy = 10.53;
    std::string m_target;

    // Deferred service/algorithm creation
    std::vector<std::function<void(ServiceManager&)>> m_service_factories;
    std::vector<std::function<void(ServiceManager&)>> m_service_configurators;
    std::vector<std::function<std::unique_ptr<Algorithm>()>> m_algorithm_factories;

    auto add_standard_services_to(Application& app) -> void;
};

/**
 * @brief Helper function to create an ApplicationBuilder
 *
 * @code
 * auto app = make_application("My Analysis")
 *     .with_input_files(files)
 *     .build();
 * @endcode
 */
[[nodiscard]] inline auto make_application(std::string name) -> ApplicationBuilder {
    return ApplicationBuilder(std::move(name));
}

}  // namespace clas12::framework
