#pragma once

/**
 * @file ApplicationBuilder.hpp
 * @brief Fluent builder for Framework::Application
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

#include "Framework/Application.hpp"
#include "Framework/IAlgorithm.hpp"
#include "Framework/Services/ConditionsService.hpp"
#include "Framework/Services/HipoFileService.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/LoggingService.hpp"
#include "Framework/Services/MetadataService.hpp"
#include "Framework/Services/NTupleService.hpp"
#include "Framework/Services/OutputFileService.hpp"
#include "Framework/Services/ParticleSelectorService.hpp"
#include "Framework/Services/ProfilingService.hpp"
#include "Framework/Services/RootFileService.hpp"
#include "Framework/Services/SamplingFractionService.hpp"

namespace Framework {

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
    auto add_algorithm(std::unique_ptr<IAlgorithm> alg) -> ApplicationBuilder& {
        auto ptr = alg.release();
        m_algorithm_factories.push_back([ptr]() {
            return std::unique_ptr<IAlgorithm>(ptr);
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
    [[nodiscard]] auto build() -> Application {
        Application app(m_name);

        // Add standard services if requested
        if (m_add_standard_services) {
            add_standard_services_to(app);
        }

        // Add custom services
        for (auto& factory : m_service_factories) {
            factory(app.service_manager());
        }

        // Configure services
        for (auto& configurator : m_service_configurators) {
            configurator(app.service_manager());
        }

        // Add algorithms
        for (auto& factory : m_algorithm_factories) {
            app.add_algorithm(factory());
        }

        // Configure application
        app.set_input_files(m_input_files);
        app.set_percentage(m_percentage);
        app.set_num_threads(m_num_threads);

        return app;
    }

private:
    std::string m_name;
    std::vector<std::string> m_input_files;
    std::string m_output_file = "output.root";
    double m_percentage = 100.0;
    int m_num_threads = 1;
    bool m_add_standard_services = false;

    // Metadata
    std::string m_version = "1.0";
    double m_beam_energy = 10.53;
    std::string m_target;

    // Deferred service/algorithm creation
    std::vector<std::function<void(ServiceManager&)>> m_service_factories;
    std::vector<std::function<void(ServiceManager&)>> m_service_configurators;
    std::vector<std::function<std::unique_ptr<IAlgorithm>()>> m_algorithm_factories;

    auto add_standard_services_to(Application& app) -> void {
        auto& mgr = app.service_manager();

        // Core services
        auto log_svc = std::make_unique<LoggingService>();
        auto output_svc = std::make_unique<OutputFileService>();
        auto hist_svc = std::make_unique<HistogramService>();
        auto ntuple_svc = std::make_unique<NTupleService>();
        auto conditions_svc = std::make_unique<ConditionsService>();
        auto particle_svc = std::make_unique<ParticleSelectorService>();
        auto sf_svc = std::make_unique<SamplingFractionService>();

        // File services
        auto hipo_svc = std::make_unique<HipoFileService>();
        auto root_svc = std::make_unique<RootFileService>();
        auto metadata_svc = std::make_unique<MetadataService>();
        auto profiling_svc = std::make_unique<ProfilingService>();

        // Configure OutputFileService with output file
        output_svc->set_output_file(m_output_file);
        
        // Get raw pointer to OutputFileService before moving it
        OutputFileService* output_svc_ptr = output_svc.get();
        
        // Wire HistogramService, NTupleService, and MetadataService to use OutputFileService
        hist_svc->set_output_file_service(output_svc_ptr);
        ntuple_svc->set_output_file_service(output_svc_ptr);
        metadata_svc->set_output_file_service(output_svc_ptr);

        // Configure other file services
        root_svc->set_default_output(m_output_file);
        metadata_svc->set_output_file(m_output_file);  // Fallback if OutputFileService not available
        hipo_svc->set_input_files(m_input_files);

        // Configure metadata
        metadata_svc->set_version(m_version);
        metadata_svc->set_beam_energy(m_beam_energy);
        if (!m_target.empty()) {
            metadata_svc->set_target(m_target);
        }

        // Add services in dependency order
        // OutputFileService MUST be added and initialized FIRST
        mgr.add_service(std::move(log_svc));
        mgr.add_service(std::move(output_svc));  // Must be before hist_svc and ntuple_svc
        mgr.add_service(std::move(hipo_svc));
        mgr.add_service(std::move(root_svc));
        mgr.add_service(std::move(conditions_svc));
        mgr.add_service(std::move(hist_svc));
        mgr.add_service(std::move(ntuple_svc));
        mgr.add_service(std::move(particle_svc));
        mgr.add_service(std::move(sf_svc));
        mgr.add_service(std::move(metadata_svc));
        mgr.add_service(std::move(profiling_svc));
    }
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

}  // namespace Framework
