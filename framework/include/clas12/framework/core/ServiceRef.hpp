#pragma once

/**
 * @file ServiceRef.hpp
 * @brief Non-owning value-type wrapper around a service pointer.
 *
 * Algorithms used to hold raw `HistogramService*` / `NTupleService*` etc.
 * members. The service is owned by ServiceManager and the algorithm only
 * needs an observer, so a raw pointer is technically fine — but it leaks
 * `= nullptr` initializers, has no default `operator bool`, and lets the
 * type system confuse "not yet wired" with "legitimately absent".
 *
 * `ServiceRef<T>` is a thin observer handle that:
 *   - Default-constructs to a null state.
 *   - Accepts assignment from `T*` via an implicit constructor, so it
 *     plugs into the existing `SimpleAlgorithm::require_service(...)`
 *     lambda that does `*target = svc<T>()`.
 *   - Forwards `operator->` and `operator*` to the wrapped pointer, so
 *     every existing call site `m_hist->book_hist1D(...)` keeps working
 *     with no source-level churn.
 *   - Converts to `bool` for nullability checks and exposes `.get()`
 *     as an escape hatch.
 *
 * Ownership: the service lives in `ServiceManager`. A `ServiceRef<T>` is
 * a non-owning observer, trivially copyable, safe to live in algorithm
 * members. It integrates with the require_service/declare_* helpers on
 * SimpleAlgorithm.
 */

namespace clas12::framework {

template <typename T>
class ServiceRef {
public:
    ServiceRef() = default;

    /// Implicit construction from the raw pointer, so that the lambda
    /// pushed by `require_service` can assign the result of
    /// `svc<T>()` into a ServiceRef member without special-casing.
    ServiceRef(T* p) noexcept : m_p(p) {}

    auto operator=(T* p) noexcept -> ServiceRef& {
        m_p = p;
        return *this;
    }

    /// Forward method calls as if this were a pointer:
    /// `m_hist->book_hist1D(...)` still works.
    auto operator->() const noexcept -> T* { return m_p; }

    /// Forward dereference for code that wants a reference.
    auto operator*() const noexcept -> T& { return *m_p; }

    /// True once a service pointer has been bound.
    explicit operator bool() const noexcept { return m_p != nullptr; }

    /// Escape hatch for code that still needs the raw pointer, e.g. to
    /// forward to a helper that has not been migrated yet.
    [[nodiscard]] auto get() const noexcept -> T* { return m_p; }

private:
    T* m_p = nullptr;
};

}  // namespace clas12::framework
