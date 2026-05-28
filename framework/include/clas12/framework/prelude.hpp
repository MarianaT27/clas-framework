#pragma once

/**
 * @file prelude.hpp
 * @brief Umbrella include for the framework essentials used by every algorithm.
 *
 * Pulls in the headers that virtually every CLAS12 algorithm needs so that
 * algorithm authors only have to write one `#include` for the framework
 * surface, plus whatever specific bank wrappers, services, or physics
 * helpers their algorithm uses.
 *
 * Usage in an algorithm header:
 *
 * @code
 * #include <clas12/framework/prelude.hpp>
 * #include <clas12/services/ElectronCutsService.hpp>  // alg-specific
 * #include <clas12/Particle.hpp>                                 // alg-specific
 *
 * namespace algorithms {
 *
 * struct MyAlgConfig {
 *     bool enable_cut = true;
 *     static constexpr auto config_fields() {
 *         using T = MyAlgConfig;
 *         return std::make_tuple(framework::field("enable_cut", &T::enable_cut));
 *     }
 * };
 *
 * class MyAlg : public framework::Algorithm {
 *     // ...
 * };
 *
 * }  // namespace algorithms
 * @endcode
 *
 * Two design notes:
 *
 * 1. **Core only** — this header pulls the framework runtime and the core
 *    histogram/property/event/config machinery. It does *not* pull in
 *    service-specific headers (`ElectronCutsService.hpp` etc.); algorithms
 *    include those individually because they declare a hard dependency on
 *    a specific service.
 *
 * 2. **No `using namespace` injections** — including this header does not
 *    import any names into the user's namespace. Algorithms still write
 *    `framework::Algorithm`, `framework::Hist1<>`, `framework::field(...)`,
 *    etc. (Many algorithms also pin `using framework::Log;` for brevity;
 *    do that locally inside your algorithm's namespace, not in the prelude.)
 */

// Core algorithm runtime + lifecycle.
#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/StatusCode.hpp>
#include <clas12/framework/core/Concepts.hpp>
#include <clas12/framework/core/ServiceManager.hpp>
#include <clas12/framework/core/ServiceRef.hpp>

// Per-event state plumbing.
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/EventHelpers.hpp>
#include <clas12/framework/core/TypedEventStore.hpp>
#include <clas12/framework/core/Decisions.hpp>

// Configuration / TOML wiring.
#include <clas12/framework/core/Field.hpp>
#include <clas12/framework/core/Property.hpp>

// Histograms — both raw aliases and self-registering wrappers.
#include <clas12/framework/core/HistogramAliases.hpp>
#include <clas12/framework/core/SectorHistograms.hpp>
#include <clas12/framework/services/HistogramService.hpp>

// N-Tuples.
#include <clas12/framework/core/NTupleStruct.hpp>
#include <clas12/framework/services/NTupleService.hpp>

// Cutflow + logging — used by virtually every algorithm.
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/services/LoggingService.hpp>
