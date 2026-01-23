#pragma once

/**
 * @file ServiceHelpers.hpp
 * @brief Safe service access utilities to reduce null-checking boilerplate
 *
 * This file provides SafeService wrapper and helper functions for cleaner
 * service access patterns, reducing scattered null-checks throughout code.
 *
 * Example usage:
 * @code
 * // BEFORE: Verbose null-checks
 * if (m_hist_svc && m_hist_svc->is_valid()) {
 *     m_hist_svc->get()->book_hist1D(...);
 * }
 *
 * // AFTER: Fluent safe access
 * safe(m_hist_svc).with([](auto* svc) { svc->book_hist1D(...); });
 *
 * // Or for optional return values
 * auto tree = safe(m_ntuple_svc).map([](auto* svc) { return svc->get_tree("events"); });
 * @endcode
 */

#include <cassert>
#include <functional>
#include <optional>
#include <type_traits>

#include "Framework/ServiceHandle.hpp"

namespace Framework {

/**
 * @brief RAII wrapper for safe service access
 *
 * SafeService provides fluent, null-safe access to services through
 * ServiceHandle pointers. It eliminates repetitive null-checking patterns
 * while maintaining safety.
 *
 * Methods:
 * - with(func): Execute func with service if valid
 * - map(func): Execute func and return optional result
 * - get(): Get raw service pointer (may be null)
 * - is_valid(): Check if service is available
 * - operator->: Direct access (asserts validity)
 *
 * Example:
 * @code
 * ServiceHandle<HistogramService>* m_hist_svc;
 *
 * // Execute action if service is valid
 * safe(m_hist_svc).with([&](auto* svc) {
 *     svc->book_hist1D("/path", "title", 100, 0, 10);
 * });
 *
 * // Get optional result
 * auto entries = safe(m_ntuple_svc).map([](auto* svc) {
 *     return svc->get_entries("tree_name");
 * });
 * if (entries) {
 *     Log::info("Entries: {}", *entries);
 * }
 * @endcode
 *
 * @tparam Service The service type
 */
template <typename Service>
class SafeService {
public:
    /**
     * @brief Construct from a ServiceHandle pointer
     *
     * @param handle Pointer to ServiceHandle (may be nullptr)
     */
    explicit SafeService(ServiceHandle<Service>* handle) : m_handle(handle) {}

    /**
     * @brief Execute action if service is available
     *
     * The provided function is called with the service pointer only if
     * the service handle is valid.
     *
     * @tparam Func Callable type accepting Service*
     * @param func Function to execute with service
     * @return SafeService& Reference to this for chaining
     */
    template <typename Func>
    auto with(Func&& func) -> SafeService& {
        if (is_valid()) {
            std::forward<Func>(func)(get());
        }
        return *this;
    }

    /**
     * @brief Execute action and return optional result
     *
     * If service is valid, executes func and wraps result in optional.
     * If service is invalid, returns std::nullopt.
     *
     * @tparam Func Callable type accepting Service* and returning R
     * @param func Function to execute with service
     * @return std::optional<R> Result wrapped in optional, or nullopt
     */
    template <typename Func>
    auto map(Func&& func) -> std::optional<std::invoke_result_t<Func, Service*>> {
        if (is_valid()) {
            return std::forward<Func>(func)(get());
        }
        return std::nullopt;
    }

    /**
     * @brief Execute action or return default value
     *
     * If service is valid, executes func. Otherwise returns default_value.
     *
     * @tparam Func Callable type accepting Service* and returning T
     * @tparam T Return type
     * @param func Function to execute with service
     * @param default_value Value to return if service unavailable
     * @return T Result of func or default_value
     */
    template <typename Func, typename T>
    auto map_or(Func&& func, T default_value) -> T {
        if (is_valid()) {
            return std::forward<Func>(func)(get());
        }
        return default_value;
    }

    /**
     * @brief Get service pointer (may be nullptr)
     *
     * @return Service* Pointer to service, or nullptr if unavailable
     */
    [[nodiscard]] auto get() const -> Service* {
        return (m_handle && m_handle->is_valid()) ? m_handle->get() : nullptr;
    }

    /**
     * @brief Check if service is available
     *
     * @return bool True if handle is valid and service is available
     */
    [[nodiscard]] auto is_valid() const -> bool {
        return m_handle && m_handle->is_valid();
    }

    /**
     * @brief Implicit conversion to pointer
     */
    operator Service*() const { return get(); }

    /**
     * @brief Arrow operator for direct access
     *
     * @note Asserts that service is valid - use only when validity is guaranteed
     * @return Service* Pointer to service
     */
    auto operator->() const -> Service* {
        assert(is_valid() && "SafeService: Service not available");
        return get();
    }

    /**
     * @brief Boolean conversion
     */
    explicit operator bool() const { return is_valid(); }

private:
    ServiceHandle<Service>* m_handle;
};


/**
 * @brief Factory function to create SafeService wrapper
 *
 * Provides convenient syntax for safe service access.
 *
 * @tparam Service The service type
 * @param handle Pointer to ServiceHandle
 * @return SafeService<Service> Safe wrapper
 *
 * Example:
 * @code
 * safe(m_hist_svc).with([](auto* svc) { ... });
 * @endcode
 */
template <typename Service>
auto safe(ServiceHandle<Service>* handle) -> SafeService<Service> {
    return SafeService<Service>(handle);
}


/**
 * @brief Execute histogram filling only if service is valid
 *
 * Convenience function for conditional histogram operations.
 *
 * @tparam HistSvc Histogram service type
 * @tparam FillFunc Callable type accepting service pointer
 * @param handle Service handle pointer
 * @param fill Function to execute for filling
 */
template <typename HistSvc, typename FillFunc>
void fill_histograms_if_valid(ServiceHandle<HistSvc>* handle, FillFunc&& fill) {
    safe(handle).with(std::forward<FillFunc>(fill));
}


/**
 * @brief Execute N-tuple filling only if service is valid and enabled
 *
 * Convenience function for conditional N-tuple operations with enable flag.
 *
 * @tparam NTupleSvc N-tuple service type
 * @tparam FillFunc Callable type accepting service pointer
 * @param handle Service handle pointer
 * @param enabled Whether N-tuple is enabled
 * @param fill Function to execute for filling
 */
template <typename NTupleSvc, typename FillFunc>
void fill_ntuple_if_valid(ServiceHandle<NTupleSvc>* handle, bool enabled, FillFunc&& fill) {
    if (enabled) {
        safe(handle).with(std::forward<FillFunc>(fill));
    }
}


/**
 * @brief Service validity checker for multiple services
 *
 * Returns true only if all provided services are valid.
 *
 * Example:
 * @code
 * if (all_valid(m_hist_svc, m_ntuple_svc, m_config_svc)) {
 *     // All services available
 * }
 * @endcode
 *
 * @tparam Services Parameter pack of ServiceHandle pointer types
 * @param handles Service handle pointers to check
 * @return bool True if all services are valid
 */
template <typename... Services>
[[nodiscard]] auto all_valid(ServiceHandle<Services>*... handles) -> bool {
    return (... && (handles && handles->is_valid()));
}


/**
 * @brief Service validity checker for at least one service
 *
 * Returns true if any of the provided services is valid.
 *
 * @tparam Services Parameter pack of ServiceHandle pointer types
 * @param handles Service handle pointers to check
 * @return bool True if any service is valid
 */
template <typename... Services>
[[nodiscard]] auto any_valid(ServiceHandle<Services>*... handles) -> bool {
    return (... || (handles && handles->is_valid()));
}

}  // namespace Framework
