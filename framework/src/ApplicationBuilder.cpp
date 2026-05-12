// Out-of-line definitions for ApplicationBuilder.
//
// Previously ApplicationBuilder.hpp pulled 11 concrete service headers so
// add_standard_services_to() could use make_unique. Because the header is
// included by every analysis and example entry point, every one of those
// translation units paid the parse cost. This .cpp contains the factory
// body; the header now only forward-declares the concrete types and has
// no direct dependency on their implementations.

#include <clas12/framework/core/ApplicationBuilder.hpp>

#include <memory>
#include <utility>

#include <clas12/framework/core/Application.hpp>
#include <clas12/framework/core/ServiceManager.hpp>
#include <clas12/framework/services/ConditionsService.hpp>
#include <clas12/framework/services/HipoFileService.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/MetadataService.hpp>
#include <clas12/framework/services/NTupleService.hpp>
#include <clas12/framework/services/OutputFileService.hpp>
#include <clas12/framework/services/SamplingFractionService.hpp>

namespace framework {

auto ApplicationBuilder::build() -> Application {
    Application app(m_name);

    if (m_add_standard_services) {
        add_standard_services_to(app);
    }

    for (auto& factory : m_service_factories) {
        factory(app.service_manager());
    }

    for (auto& configurator : m_service_configurators) {
        configurator(app.service_manager());
    }

    for (auto& factory : m_algorithm_factories) {
        app.add_algorithm(factory());
    }

    app.set_input_files(m_input_files);
    app.set_percentage(m_percentage);
    app.set_num_threads(m_num_threads);
    app.set_config(m_config);  // null if user never called with_config()
    app.set_io_options(m_io_options);
    app.set_record_range(m_record_first, m_record_last);

    return app;
}

auto ApplicationBuilder::add_standard_services_to(Application& app) -> void {
    auto& mgr = app.service_manager();

    // Core services
    auto log_svc = std::make_unique<LoggingService>();
    auto output_svc = std::make_unique<OutputFileService>();
    auto hist_svc = std::make_unique<HistogramService>();
    auto ntuple_svc = std::make_unique<NTupleService>();
    auto conditions_svc = std::make_unique<ConditionsService>();
    auto sf_svc = std::make_unique<SamplingFractionService>();

    // File services
    auto hipo_svc = std::make_unique<HipoFileService>();
    auto metadata_svc = std::make_unique<MetadataService>();

    // Configure OutputFileService with output file. HistogramService,
    // NTupleService and MetadataService discover it via locate_service()
    // in their initialize() methods using the declared dependencies below.
    output_svc->set_output_file(m_output_file);

    // Configure other file services
    metadata_svc->set_output_file(m_output_file);
    hipo_svc->set_input_files(m_input_files);

    // Configure metadata
    metadata_svc->set_version(m_version);
    metadata_svc->set_beam_energy(m_beam_energy);
    if (!m_target.empty()) {
        metadata_svc->set_target(m_target);
    }

    // Register services with explicit dependencies. Topological sort
    // (Kahn's algorithm in ServiceManager) handles the ordering.
    // NTupleService is intentionally placed after HistogramService and
    // OutputFileService so it initializes last and finalizes first
    // (finalize_all reverses init order). This replaced a hardcoded
    // special case in ServiceManager::topological_sort.
    mgr.add_service(std::move(log_svc));
    mgr.add_service(std::move(output_svc));
    mgr.add_service(std::move(hipo_svc));
    mgr.add_service(std::move(conditions_svc));
    mgr.add_service(std::move(hist_svc),     {"OutputFileService"});
    mgr.add_service(std::move(sf_svc));
    mgr.add_service(std::move(metadata_svc), {"OutputFileService"});
    mgr.add_service(std::move(ntuple_svc),   {"OutputFileService", "HistogramService"});
}

}  // namespace framework
