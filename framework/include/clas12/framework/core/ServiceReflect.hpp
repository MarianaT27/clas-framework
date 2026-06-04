#pragma once

/**
 * @file ServiceReflect.hpp
 * @brief Reflection-based service auto-wiring (refl-cpp).
 *
 * Instead of listing every service in `required_services(m_a, m_b, ...)`, an
 * algorithm reflects over its own `ServiceRef<T>` members and wires each one
 * automatically. Opt in with two annotations and one call:
 *
 *     class MyAlg : public framework::ConfigurableAlgorithm<MyConfig> {
 *         FRAMEWORK_REFLECT_FRIEND;                     // private-member access
 *         framework::ServiceRef<framework::HistogramService> m_hist;
 *         framework::ServiceRef<framework::NTupleService>    m_nt;
 *      public:
 *         explicit MyAlg(...) : ConfigurableAlgorithm(...) {
 *             framework::wire_services(*this);          // replaces required_services
 *         }
 *     };
 *     REFL_AUTO(type(MyAlg), field(m_hist), field(m_nt))   // at namespace scope
 *
 * This header is the ONLY place that pulls in the (large) refl-cpp header, so
 * algorithms that don't opt in pay nothing for it.
 */

#include <type_traits>

#include <refl-cpp/refl.hpp>

#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/ServiceRef.hpp>

namespace framework {

template <typename T> struct is_service_ref : std::false_type {};
template <typename T> struct is_service_ref<ServiceRef<T>> : std::true_type {};
template <typename T> inline constexpr bool is_service_ref_v = is_service_ref<T>::value;

/**
 * @brief Declare a required service for every `ServiceRef<T>` data member of
 *        @p alg, discovered by reflection.
 *
 * Equivalent to calling `required_services(...)` with every `ServiceRef`
 * member listed explicitly. Non-`ServiceRef` members are ignored. The
 * algorithm must carry `FRAMEWORK_REFLECT_FRIEND;` (for private access) and a
 * `REFL_AUTO(type(Alg), field(...))` annotation listing its reflected members.
 */
template <typename Alg>
auto wire_services(Alg& alg) -> void {
    refl::util::for_each(refl::reflect(alg).members, [&](auto member) {
        if constexpr (refl::trait::is_field_v<decltype(member)>) {
            auto& m = member(alg);
            using M = std::remove_reference_t<decltype(m)>;
            if constexpr (is_service_ref_v<M>) {
                alg.require_service_ref(m);
            }
        }
    });
}

}  // namespace framework

/// Befriend the refl-cpp metadata so its generated accessors can read the
/// algorithm's private `ServiceRef` members. Place anywhere in the class body.
#define FRAMEWORK_REFLECT_FRIEND \
    template <typename> friend struct refl_impl::metadata::type_info__
