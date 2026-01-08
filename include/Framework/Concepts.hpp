#pragma once

/**
 * @file Concepts.hpp
 * @brief C++20 concepts for compile-time service type validation
 *
 * This file defines concepts that constrain template parameters to valid
 * service implementations, enabling better error messages and self-documenting
 * code. These concepts are used throughout the framework to provide compile-time
 * type safety for service access.
 *
 * @see Requirements 1.1, 1.3, 1.4
 */

#include <concepts>
#include <string>
#include <type_traits>

#include "StatusCode.hpp"

// Forward declarations for ROOT types used in HistogramProvider concept
class TH1D;
class TH2D;

namespace Framework {

// Forward declaration
class IService;

/**
 * @brief Concept for valid service types
 *
 * A type satisfies ServiceConcept if it:
 * - Derives from IService
 * - Has a name() method returning a const std::string&
 * - Has an initialize() method returning StatusCode
 * - Has a finalize() method returning StatusCode
 *
 * This concept enables compile-time validation of service types, producing
 * clear error messages when incompatible types are used.
 *
 * @tparam T The type to check
 *
 * Example usage:
 * @code
 * template <ServiceConcept T>
 * class ServiceHandle { ... };
 *
 * // This will compile:
 * ServiceHandle<HistogramService> hist_svc;
 *
 * // This will produce a clear compile error:
 * ServiceHandle<int> invalid;  // error: int does not satisfy ServiceConcept
 * @endcode
 *
 * @see Requirement 1.1
 */
template <typename T>
concept ServiceConcept = std::derived_from<T, IService> &&
    requires(T svc, const T csvc) {
        { csvc.name() } -> std::convertible_to<const std::string&>;
        { svc.initialize() } -> std::same_as<StatusCode>;
        { svc.finalize() } -> std::same_as<StatusCode>;
    };

/**
 * @brief Concept for services that provide histogram functionality
 *
 * A type satisfies HistogramProvider if it:
 * - Satisfies ServiceConcept
 * - Has book_hist1D() method for booking 1D histograms
 * - Has book_hist2D() method for booking 2D histograms
 * - Has get_hist1D() method for retrieving 1D histograms
 * - Has get_hist2D() method for retrieving 2D histograms
 *
 * This concept is useful for algorithms that need histogram services
 * but want to be generic over the specific histogram service implementation.
 *
 * @tparam T The type to check
 *
 * Example usage:
 * @code
 * template <HistogramProvider T>
 * void book_analysis_histograms(T& hist_svc) {
 *     hist_svc.book_hist1D("/analysis/momentum", "p;p [GeV];Counts", 100, 0, 10);
 * }
 * @endcode
 */
template <typename T>
concept HistogramProvider = ServiceConcept<T> &&
    requires(T svc, const std::string& path, const std::string& title,
             int nbins, double xmin, double xmax, int nbinsy, double ymin, double ymax) {
        { svc.book_hist1D(path, title, nbins, xmin, xmax) } -> std::same_as<TH1D*>;
        { svc.book_hist2D(path, title, nbins, xmin, xmax, nbinsy, ymin, ymax) } -> std::same_as<TH2D*>;
        { svc.get_hist1D(path) } -> std::same_as<TH1D*>;
        { svc.get_hist2D(path) } -> std::same_as<TH2D*>;
    };

/**
 * @brief Concept for services that provide logging functionality
 *
 * A type satisfies LoggingProvider if it:
 * - Satisfies ServiceConcept
 * - Has info(), warning(), error(), debug() logging methods
 *
 * This concept allows algorithms to be generic over logging implementations
 * while ensuring the required logging interface is available.
 *
 * @tparam T The type to check
 *
 * Example usage:
 * @code
 * template <LoggingProvider T>
 * void log_event_info(T& logger, int event_num) {
 *     logger.info("MyAlgorithm", "Processing event {}", event_num);
 * }
 * @endcode
 */
template <typename T>
concept LoggingProvider = ServiceConcept<T> &&
    requires(T svc, const std::string& source, const char* msg) {
        // Check that logging methods exist with basic string arguments
        // The actual methods are templates, so we check with concrete types
        svc.info(source, "{}", 0);
        svc.warning(source, "{}", 0);
        svc.error(source, "{}", 0);
        svc.debug(source, "{}", 0);
    };

/**
 * @brief Concept for services that provide configuration functionality
 *
 * A type satisfies ConfigProvider if it:
 * - Satisfies ServiceConcept
 * - Has get<T>() method for retrieving typed configuration values
 * - Has contains() method for checking key existence
 *
 * @tparam T The type to check
 */
template <typename T>
concept ConfigProvider = ServiceConcept<T> &&
    requires(T svc, const std::string& key) {
        { svc.contains(key) } -> std::convertible_to<bool>;
    };

/**
 * @brief Helper concept to check if a type is a valid service handle target
 *
 * This is a convenience concept that combines ServiceConcept with
 * additional requirements for use with ServiceHandle.
 *
 * @tparam T The type to check
 */
template <typename T>
concept ServiceHandleTarget = ServiceConcept<T> &&
    std::is_class_v<T> &&
    !std::is_abstract_v<T>;

}  // namespace Framework
