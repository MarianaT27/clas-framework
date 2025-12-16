#pragma once

#include <memory>
#include <mutex>
#include "TH1D.h"
#include "TH2D.h"
#include "TProfile.h"

namespace Framework {

/**
 * @brief Thread-safe wrapper for TH1D
 *
 * Provides mutex-protected histogram filling for parallel processing.
 * Each fill operation is atomic and thread-safe.
 */
class ThreadSafeHist1D {
   public:
    explicit ThreadSafeHist1D(TH1D* hist = nullptr) : m_hist(hist) {}

    auto fill(double value) -> ThreadSafeHist1D& {
        if (m_hist) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_hist->Fill(value);
        }
        return *this;
    }

    auto fill_weighted(double value, double weight) -> ThreadSafeHist1D& {
        if (m_hist) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_hist->Fill(value, weight);
        }
        return *this;
    }

    auto fill_if(bool condition, double value) -> ThreadSafeHist1D& {
        if (condition && m_hist) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_hist->Fill(value);
        }
        return *this;
    }

    auto get() -> TH1D* { return m_hist; }

    auto get() const -> const TH1D* { return m_hist; }

    auto is_valid() const -> bool { return m_hist != nullptr; }

    explicit operator bool() const { return is_valid(); }

   private:
    TH1D* m_hist;
    mutable std::mutex m_mutex;
};

/**
 * @brief Thread-safe wrapper for TH2D
 */
class ThreadSafeHist2D {
   public:
    explicit ThreadSafeHist2D(TH2D* hist = nullptr) : m_hist(hist) {}

    auto fill(double x, double y) -> ThreadSafeHist2D& {
        if (m_hist) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_hist->Fill(x, y);
        }
        return *this;
    }

    auto fill_weighted(double x, double y, double weight) -> ThreadSafeHist2D& {
        if (m_hist) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_hist->Fill(x, y, weight);
        }
        return *this;
    }

    auto fill_if(bool condition, double x, double y) -> ThreadSafeHist2D& {
        if (condition && m_hist) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_hist->Fill(x, y);
        }
        return *this;
    }

    auto get() -> TH2D* { return m_hist; }

    auto get() const -> const TH2D* { return m_hist; }

    auto is_valid() const -> bool { return m_hist != nullptr; }

    explicit operator bool() const { return is_valid(); }

   private:
    TH2D* m_hist;
    mutable std::mutex m_mutex;
};

/**
 * @brief Thread-safe wrapper for TProfile
 */
class ThreadSafeProfile {
   public:
    explicit ThreadSafeProfile(TProfile* prof = nullptr) : m_prof(prof) {}

    auto fill(double x, double y) -> ThreadSafeProfile& {
        if (m_prof) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_prof->Fill(x, y);
        }
        return *this;
    }

    auto fill_weighted(double x, double y, double weight) -> ThreadSafeProfile& {
        if (m_prof) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_prof->Fill(x, y, weight);
        }
        return *this;
    }

    auto fill_if(bool condition, double x, double y) -> ThreadSafeProfile& {
        if (condition && m_prof) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_prof->Fill(x, y);
        }
        return *this;
    }

    auto get() -> TProfile* { return m_prof; }

    auto get() const -> const TProfile* { return m_prof; }

    auto is_valid() const -> bool { return m_prof != nullptr; }

    explicit operator bool() const { return is_valid(); }

   private:
    TProfile* m_prof;
    mutable std::mutex m_mutex;
};

/**
 * @brief Documentation for thread-safe histogram usage
 *
 * Thread-safe histogram wrappers provide atomic fill operations.
 * Use these when processing events in parallel threads.
 *
 * Example:
 * @code
 * // Book histogram normally
 * TH1D* h = hist_svc->book_hist1D("/path", "title", 100, 0, 10);
 *
 * // Wrap for thread-safe access
 * ThreadSafeHist1D thread_safe_h(h);
 *
 * // Fill from multiple threads - thread safe!
 * #pragma omp parallel for
 * for (int i = 0; i < n_events; ++i) {
 *     thread_safe_h.fill(some_value[i]);
 * }
 * @endcode
 *
 * Note: ROOT histograms are NOT thread-safe by default!
 * Always use these wrappers for parallel processing.
 */

}  // namespace Framework
