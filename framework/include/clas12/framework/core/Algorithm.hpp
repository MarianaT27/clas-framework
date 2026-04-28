#pragma once

#include <fmt/format.h>
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <clas12/framework/core/Concepts.hpp>
#include <clas12/framework/core/IService.hpp>
#include <clas12/framework/core/NTupleStruct.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/ServiceManager.hpp>
#include <clas12/framework/core/ServiceRef.hpp>
#include <clas12/framework/core/StatusCode.hpp>
#include <clas12/framework/core/TypedEventStore.hpp>
#include <clas12/framework/services/LoggingService.hpp>

namespace framework {

class Event;

/**
 * @brief Algorithm lifecycle states
 *
 * Tracks the current state of an algorithm to prevent invalid state
 * transitions. Stored in std::atomic so concurrent worker threads can
 * read it from process() while the serial initialize()/finalize()
 * stages perform the writes.
 */
enum class AlgorithmState : std::uint8_t {
    Uninitialized = 0,
    Initialized   = 1,
    Running       = 2,
    Finalized     = 3,
};

/**
 * @brief Base class for every algorithm in the framework.
 *
 * Lifecycle:
 *   1. Construction
 *   2. initialize() — once, before event processing
 *   3. execute()    — once per event (concurrent across worker threads)
 *   4. finalize()   — once, after all events
 *
 * Derived classes implement the user-facing hooks and let the framework
 * drive the lifecycle:
 *   - configure()       — declare services / banks (rarely needed; most
 *                         declarations belong in the constructor)
 *   - process(event)    — REQUIRED. Per-event logic.
 *   - on_initialize()   — optional. Booking, setup.
 *   - on_finalize()     — optional. Stats, cleanup.
 *
 * THREAD SAFETY:
 * Application invokes execute() concurrently from N worker threads on the
 * same algorithm instance. Mutable state accessed from process() must be
 * one of:
 *   1. atomic (std::atomic<T>, atomic Cutflow counters)
 *   2. mutex-guarded
 *   3. thread-local (one instance per worker via ThreadLocalHistogram or
 *      thread_local storage)
 *   4. append-only after initialize() (e.g. configuration, ServiceRef)
 * initialize() and finalize() are serial.
 */
class Algorithm : public PropertyMixin {
public:
    explicit Algorithm(std::string name) : m_name(std::move(name)) {}
    virtual ~Algorithm() = default;

    // ========================================================================
    // User-facing hooks — override in derived classes
    // ========================================================================

    /// Optional: declare banks/properties up-front. Runs before service
    /// retrieval. Most algorithms can leave this as the default and do
    /// declarations in the constructor instead.
    virtual auto configure() -> void {}

    /// REQUIRED: per-event logic.
    /// Return SUCCESS to continue, FILTERED to skip, FAILURE to abort.
    virtual auto process(Event& event) -> StatusCode = 0;

    /// Optional: called once after services are injected, before event
    /// processing begins. Use for booking histograms, setup.
    virtual auto on_initialize() -> StatusCode { return StatusCode::SUCCESS(); }

    /// Optional: called once after the event loop, before destruction.
    virtual auto on_finalize() -> StatusCode { return StatusCode::SUCCESS(); }

    // ========================================================================
    // Framework-driven lifecycle — final, do not override
    // ========================================================================

    auto initialize() -> StatusCode {
        if (auto sc = validate_state_transition(AlgorithmState::Initialized); sc.is_failure()) {
            return sc;
        }

        configure();

        for (auto& retrieve : m_required_services) {
            if (!retrieve()) return StatusCode::FAILURE();
        }
        for (auto& hook : m_ntuple_init_hooks) hook();

        auto result = on_initialize();
        if (result.is_success()) set_state(AlgorithmState::Initialized);
        return result;
    }

    auto execute(Event& event) -> StatusCode {
        // Validate against current state but do NOT flip m_state here:
        // execute() runs concurrently on many worker threads; storing into
        // a shared atomic would race with every sibling worker. State
        // transitions happen only in initialize()/finalize().
        if (auto sc = validate_state_transition(AlgorithmState::Running); sc.is_failure()) {
            return sc;
        }
        return process(event);
    }

    auto finalize() -> StatusCode {
        if (auto sc = validate_state_transition(AlgorithmState::Finalized); sc.is_failure()) {
            return sc;
        }
        auto result = on_finalize();
        set_state(AlgorithmState::Finalized);
        return result;
    }

    // ========================================================================
    // Public API used by Application / ApplicationBuilder
    // ========================================================================

    [[nodiscard]] auto name() const -> const std::string& { return m_name; }

    auto set_service_manager(ServiceManager* svc_mgr) -> void { m_svc_mgr = svc_mgr; }

    [[nodiscard]] auto state() const -> AlgorithmState {
        return m_state.load(std::memory_order_acquire);
    }

    [[nodiscard]] auto is_ready() const -> bool {
        auto s = state();
        return s == AlgorithmState::Initialized || s == AlgorithmState::Running;
    }

    [[nodiscard]] auto required_banks() const -> std::vector<std::string> { return m_required_banks; }
    [[nodiscard]] auto optional_banks() const -> std::vector<std::string> { return m_optional_banks; }

    [[nodiscard]] auto consumed_keys() const -> const std::vector<std::string>& { return m_consumed_keys; }
    [[nodiscard]] auto produced_keys() const -> const std::vector<std::string>& { return m_produced_keys; }

protected:
    // ========================================================================
    // Helpers for derived classes
    // ========================================================================

    /// Look up a service by its static service_name(). Returns nullptr if
    /// the service is not registered or the manager is unavailable.
    template <ServiceConcept T>
    auto svc() -> T* {
        if (!m_svc_mgr) return nullptr;
        if constexpr (requires { T::service_name(); }) {
            return m_svc_mgr->get_service<T>(std::string(T::service_name()));
        } else {
            return nullptr;
        }
    }

    /// Declare a required service. Call this in the derived-class
    /// constructor. During initialize() the framework retrieves the
    /// service via svc<T>() and assigns it to @p member_ref. Missing
    /// services cause initialize() to fail before on_initialize() runs.
    template <ServiceConcept T>
    void require_service(T*& member_ref) {
        m_required_services.push_back([this, target = &member_ref]() -> bool {
            auto* s = this->svc<T>();
            *target = s;
            if (!s) {
                if constexpr (requires { T::service_name(); }) {
                    Log::error(this->name(), "Required service not available: {}", T::service_name());
                } else {
                    Log::error(this->name(), "Required service not available");
                }
                return false;
            }
            return true;
        });
    }

    /// Overload for ServiceRef<T> members.
    template <ServiceConcept T>
    void require_service(ServiceRef<T>& member_ref) {
        m_required_services.push_back([this, target = &member_ref]() -> bool {
            auto* s = this->svc<T>();
            *target = s;
            if (!s) {
                if constexpr (requires { T::service_name(); }) {
                    Log::error(this->name(), "Required service not available: {}", T::service_name());
                } else {
                    Log::error(this->name(), "Required service not available");
                }
                return false;
            }
            return true;
        });
    }

    /// Variadic: declare several required services in one call. Accepts
    /// any mix of raw `T*` members and `ServiceRef<T>` members.
    template <typename... Args>
    void required_services(Args&... member_refs) {
        (require_service(member_refs), ...);
    }

    /// Declare an NTupleWriter<T> for automatic init against NTupleService.
    /// If NTupleService is not registered, the writer stays uninitialized
    /// (soft failure).
    template <typename T>
    void declare_ntuple(NTupleWriter<T>& writer, std::string tree_name, bool enabled) {
        if (!enabled) return;
        m_ntuple_init_hooks.push_back(
            [this, &writer, tree_name = std::move(tree_name)]() {
                if (auto* s = this->svc<NTupleService>()) writer.init(s, tree_name);
            });
    }

    /// Gated overload: the gate is evaluated lazily at initialize() time.
    template <typename T>
    void declare_ntuple(NTupleWriter<T>& writer,
                        std::string tree_name,
                        const Property<bool>& enabled) {
        m_ntuple_init_hooks.push_back(
            [this, &writer, tree_name = std::move(tree_name), &enabled]() {
                if (!enabled.get()) return;
                if (auto* s = this->svc<NTupleService>()) writer.init(s, tree_name);
            });
    }

    void declare_bank(std::string_view bank_name) {
        m_required_banks.emplace_back(bank_name);
    }

    template <typename... Args>
    void declare_banks(Args&&... bank_names) {
        (declare_bank(std::forward<Args>(bank_names)), ...);
    }

    /// Declare a bank that this algorithm reads only if present in the
    /// input file's schema dictionary. Lets one source tree handle MC and
    /// real-data runs.
    void declare_optional_bank(std::string_view bank_name) {
        m_optional_banks.emplace_back(bank_name);
    }

    template <typename T>
    void consumes(const TypedKey<T>& key) { m_consumed_keys.push_back(key.name); }

    template <typename T>
    void produces(const TypedKey<T>& key) { m_produced_keys.push_back(key.name); }

    auto set_state(AlgorithmState new_state) -> void {
        m_state.store(new_state, std::memory_order_release);
    }

    [[nodiscard]] auto validate_state_transition(AlgorithmState target) const -> StatusCode {
        const auto current = m_state.load(std::memory_order_acquire);
        switch (target) {
            case AlgorithmState::Initialized:
                if (current != AlgorithmState::Uninitialized) {
                    return StatusCode::failure(
                        fmt::format("Algorithm '{}': cannot initialize from state {}",
                                    m_name, static_cast<int>(current)));
                }
                break;
            case AlgorithmState::Running:
                if (current != AlgorithmState::Initialized && current != AlgorithmState::Running) {
                    return StatusCode::failure(
                        fmt::format("Algorithm '{}': cannot execute from state {}",
                                    m_name, static_cast<int>(current)));
                }
                break;
            case AlgorithmState::Finalized:
                // Allow finalize from any state for safe cleanup, even
                // when initialize() failed.
                break;
            default:
                break;
        }
        return StatusCode::SUCCESS();
    }

    ServiceManager* m_svc_mgr = nullptr;

private:
    std::string m_name;
    std::atomic<AlgorithmState> m_state{AlgorithmState::Uninitialized};

    std::vector<std::string> m_required_banks;
    std::vector<std::string> m_optional_banks;
    std::vector<std::string> m_consumed_keys;
    std::vector<std::string> m_produced_keys;
    std::vector<std::function<bool()>> m_required_services;
    std::vector<std::function<void()>> m_ntuple_init_hooks;
};

}  // namespace framework
