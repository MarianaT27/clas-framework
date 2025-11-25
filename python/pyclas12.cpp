#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "Framework/Application.hpp"
#include "Framework/IAlgorithm.hpp"
#include "Framework/ServiceManager.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/ConditionsService.hpp"
#include "Core/Target.hpp"

namespace py = pybind11;
using namespace Framework;

/**
 * @brief Python bindings for CLAS12 analysis framework
 *
 * This module provides Python access to the C++ framework, enabling
 * configuration-driven analysis similar to CMSSW's Python configs.
 */
PYBIND11_MODULE(pyclas12, m) {
    m.doc() = "Python bindings for CLAS12 analysis framework";

    // StatusCode
    py::class_<StatusCode>(m, "StatusCode")
        .def("is_success", &StatusCode::is_success)
        .def("is_failure", &StatusCode::is_failure);

    // Target enum
    py::enum_<Core::Target>(m, "Target")
        .value("LD2", Core::Target::LD2)
        .value("CxC", Core::Target::CxC)
        .value("Cu", Core::Target::Cu)
        .value("Sn", Core::Target::Sn)
        .value("CuSn", Core::Target::CuSn)
        .export_values();

    // Polarity enum
    py::enum_<Core::Polarity>(m, "Polarity")
        .value("Inbending", Core::Polarity::Inbending)
        .value("Outbending", Core::Polarity::Outbending)
        .export_values();

    // HistogramService
    py::class_<HistogramService, Service, std::shared_ptr<HistogramService>>(m, "HistogramService")
        .def(py::init<std::string>(), py::arg("name") = "HistogramService")
        .def("set_output_file", &HistogramService::set_output_file)
        .def("book_hist1D", &HistogramService::book_hist1D,
             py::arg("path"), py::arg("title"),
             py::arg("nbins"), py::arg("xmin"), py::arg("xmax"),
             py::return_value_policy::reference)
        .def("book_hist2D", &HistogramService::book_hist2D,
             py::arg("path"), py::arg("title"),
             py::arg("nbinsx"), py::arg("xmin"), py::arg("xmax"),
             py::arg("nbinsy"), py::arg("ymin"), py::arg("ymax"),
             py::return_value_policy::reference);

    // ConditionsService
    py::class_<ConditionsService, Service, std::shared_ptr<ConditionsService>>(m, "ConditionsService")
        .def(py::init<std::string>(), py::arg("name") = "ConditionsService")
        .def("set_run_number", &ConditionsService::set_run_number)
        .def("run_number", &ConditionsService::run_number)
        .def("load_from_database", &ConditionsService::load_from_database);

    // Application
    py::class_<Application>(m, "Application")
        .def(py::init<std::string>())
        .def("set_input_files", &Application::set_input_files)
        .def("set_max_events", &Application::set_max_events)
        .def("set_num_threads", &Application::set_num_threads)
        .def("initialize", &Application::initialize)
        .def("run", &Application::run)
        .def("finalize", &Application::finalize)
        .def("service_manager", &Application::service_manager,
             py::return_value_policy::reference);

    // ServiceManager
    py::class_<ServiceManager>(m, "ServiceManager")
        .def("add_service", [](ServiceManager& mgr, std::shared_ptr<IService> svc) {
            // Convert shared_ptr to unique_ptr for adding
            return mgr.add_service(std::unique_ptr<IService>(svc.get()));
        })
        .def("initialize_all", &ServiceManager::initialize_all)
        .def("finalize_all", &ServiceManager::finalize_all);
}
