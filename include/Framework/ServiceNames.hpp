#pragma once

#include <string_view>

namespace Framework {

/**
 * @brief Compile-time constants for service names
 *
 * This namespace provides standard service name constants to avoid
 * magic strings throughout the codebase. Using these constants ensures
 * consistency and enables compile-time error detection for typos.
 *
 * Example usage:
 * @code
 * // Instead of:
 * auto& hist_svc = manager.get_service<HistogramService>("HistogramService");
 *
 * // Use:
 * auto& hist_svc = manager.get_service<HistogramService>(ServiceNames::HistogramService);
 * @endcode
 *
 * @see Requirements 5.1
 */
namespace ServiceNames {

// Core framework services
inline constexpr std::string_view LoggingService = "LoggingService";
inline constexpr std::string_view ConfigService = "ConfigService";
inline constexpr std::string_view ProfilingService = "ProfilingService";
inline constexpr std::string_view SummaryService = "SummaryService";
inline constexpr std::string_view MetadataService = "MetadataService";

// File I/O services
inline constexpr std::string_view HistogramService = "HistogramService";
inline constexpr std::string_view NTupleService = "NTupleService";
inline constexpr std::string_view OutputFileService = "OutputFileService";
inline constexpr std::string_view RootFileService = "RootFileService";
inline constexpr std::string_view HipoFileService = "HipoFileService";

// Physics services
inline constexpr std::string_view ConditionsService = "ConditionsService";
inline constexpr std::string_view CorrectionsService = "CorrectionsService";
inline constexpr std::string_view SamplingFractionService = "SamplingFractionService";

// Selection and cuts services
inline constexpr std::string_view SelectionService = "SelectionService";
inline constexpr std::string_view ParticleSelectorService = "ParticleSelectorService";
inline constexpr std::string_view ElectronCutsService = "ElectronCutsService";
inline constexpr std::string_view PhotonCutsService = "PhotonCutsService";

// Visualization services
inline constexpr std::string_view WorkflowVisualizationService = "WorkflowVisualizationService";

}  // namespace ServiceNames

}  // namespace Framework
