#pragma once

#include "TH1D.h"
#include "TH2D.h"
#include "TProfile.h"
#include "TEfficiency.h"
#include <string>
#include <memory>
#include <map>
#include <vector>
#include <optional>

namespace Framework {

/**
 * @brief Type-safe histogram wrapper for 1D histograms
 *
 * Provides a cleaner interface than raw ROOT TH1D with:
 * - Automatic null-pointer checking
 * - Fluent API for filling
 * - Weight support
 * - Optional condition-based filling
 */
class Hist1D {
public:
    explicit Hist1D(TH1D* hist = nullptr) : m_hist(hist) {}

    /**
     * @brief Fill histogram with value
     */
    auto fill(double value) -> Hist1D& {
        if (m_hist) m_hist->Fill(value);
        return *this;
    }

    /**
     * @brief Fill histogram with value and weight
     */
    auto fill(double value, double weight) -> Hist1D& {
        if (m_hist) m_hist->Fill(value, weight);
        return *this;
    }

    /**
     * @brief Fill histogram if condition is true
     */
    auto fill_if(bool condition, double value) -> Hist1D& {
        if (condition && m_hist) m_hist->Fill(value);
        return *this;
    }

    /**
     * @brief Fill histogram if condition is true, with weight
     */
    auto fill_if(bool condition, double value, double weight) -> Hist1D& {
        if (condition && m_hist) m_hist->Fill(value, weight);
        return *this;
    }

    /**
     * @brief Get underlying ROOT histogram
     */
    auto get() -> TH1D* { return m_hist; }
    auto get() const -> const TH1D* { return m_hist; }

    /**
     * @brief Check if histogram is valid
     */
    auto is_valid() const -> bool { return m_hist != nullptr; }

    /**
     * @brief Implicit conversion to bool for validity checking
     */
    explicit operator bool() const { return is_valid(); }

    /**
     * @brief Allow direct access to ROOT methods via ->
     */
    auto operator->() -> TH1D* { return m_hist; }
    auto operator->() const -> const TH1D* { return m_hist; }

private:
    TH1D* m_hist;
};

/**
 * @brief Type-safe histogram wrapper for 2D histograms
 */
class Hist2D {
public:
    explicit Hist2D(TH2D* hist = nullptr) : m_hist(hist) {}

    auto fill(double x, double y) -> Hist2D& {
        if (m_hist) m_hist->Fill(x, y);
        return *this;
    }

    auto fill(double x, double y, double weight) -> Hist2D& {
        if (m_hist) m_hist->Fill(x, y, weight);
        return *this;
    }

    auto fill_if(bool condition, double x, double y) -> Hist2D& {
        if (condition && m_hist) m_hist->Fill(x, y);
        return *this;
    }

    auto fill_if(bool condition, double x, double y, double weight) -> Hist2D& {
        if (condition && m_hist) m_hist->Fill(x, y, weight);
        return *this;
    }

    auto get() -> TH2D* { return m_hist; }
    auto get() const -> const TH2D* { return m_hist; }
    auto is_valid() const -> bool { return m_hist != nullptr; }
    explicit operator bool() const { return is_valid(); }
    auto operator->() -> TH2D* { return m_hist; }
    auto operator->() const -> const TH2D* { return m_hist; }

private:
    TH2D* m_hist;
};

/**
 * @brief Type-safe wrapper for TProfile histograms
 */
class Profile {
public:
    explicit Profile(TProfile* prof = nullptr) : m_prof(prof) {}

    auto fill(double x, double y) -> Profile& {
        if (m_prof) m_prof->Fill(x, y);
        return *this;
    }

    auto fill(double x, double y, double weight) -> Profile& {
        if (m_prof) m_prof->Fill(x, y, weight);
        return *this;
    }

    auto fill_if(bool condition, double x, double y) -> Profile& {
        if (condition && m_prof) m_prof->Fill(x, y);
        return *this;
    }

    auto get() -> TProfile* { return m_prof; }
    auto get() const -> const TProfile* { return m_prof; }
    auto is_valid() const -> bool { return m_prof != nullptr; }
    explicit operator bool() const { return is_valid(); }
    auto operator->() -> TProfile* { return m_prof; }
    auto operator->() const -> const TProfile* { return m_prof; }

private:
    TProfile* m_prof;
};

/**
 * @brief Collection of related histograms
 *
 * Organizes related histograms into a group with:
 * - Common path prefix
 * - Structured naming
 * - Easy access by name
 *
 * Example:
 * @code
 * HistogramGroup electron_hists(hist_svc, "/Electron");
 * electron_hists.add_hist1D("momentum", "p;p [GeV/c];Events", 100, 0, 10);
 * electron_hists.add_hist1D("theta", "#theta;#theta [deg];Events", 180, 0, 180);
 *
 * // Fill by name
 * electron_hists["momentum"].fill(electron.p());
 * electron_hists["theta"].fill(electron.theta());
 * @endcode
 */
class HistogramGroup {
public:
    HistogramGroup(class HistogramService* service, std::string path_prefix)
        : m_service(service), m_path_prefix(std::move(path_prefix)) {}

    /**
     * @brief Add and book a 1D histogram
     */
    auto add_hist1D(const std::string& name, const std::string& title,
                    int nbins, double xmin, double xmax) -> Hist1D&;

    /**
     * @brief Add and book a 2D histogram
     */
    auto add_hist2D(const std::string& name, const std::string& title,
                    int nbinsx, double xmin, double xmax,
                    int nbinsy, double ymin, double ymax) -> Hist2D&;

    /**
     * @brief Add and book a TProfile
     */
    auto add_profile(const std::string& name, const std::string& title,
                     int nbins, double xmin, double xmax) -> Profile&;

    /**
     * @brief Get histogram by name
     */
    auto operator[](const std::string& name) -> Hist1D& {
        return m_hist1d[name];
    }

    /**
     * @brief Get 2D histogram by name
     */
    auto hist2d(const std::string& name) -> Hist2D& {
        return m_hist2d[name];
    }

    /**
     * @brief Get profile by name
     */
    auto profile(const std::string& name) -> Profile& {
        return m_profiles[name];
    }

    /**
     * @brief Get full path for a histogram
     */
    auto full_path(const std::string& name) const -> std::string {
        return m_path_prefix + "/" + name;
    }

private:
    class HistogramService* m_service;
    std::string m_path_prefix;
    std::map<std::string, Hist1D> m_hist1d;
    std::map<std::string, Hist2D> m_hist2d;
    std::map<std::string, Profile> m_profiles;
};

/**
 * @brief Histogram that automatically fills at each cut stage
 *
 * CutflowHistogram tracks a variable through sequential cuts, creating
 * a separate histogram for "after each cut" automatically.
 *
 * Example:
 * @code
 * CutflowHistogram h_momentum(hist_svc, "/Electron/cutflow/momentum",
 *                             "Electron Momentum", 100, 0, 10);
 * h_momentum.register_cut("All Events");
 * h_momentum.register_cut("Chi2 PID");
 * h_momentum.register_cut("Momentum");
 *
 * // In event loop:
 * h_momentum.fill("All Events", electron.p());
 * if (pass_chi2pid) h_momentum.fill("Chi2 PID", electron.p());
 * if (pass_momentum) h_momentum.fill("Momentum", electron.p());
 * @endcode
 *
 * Creates:
 * - /Electron/cutflow/momentum_AllEvents
 * - /Electron/cutflow/momentum_Chi2PID
 * - /Electron/cutflow/momentum_Momentum
 */
class CutflowHistogram {
public:
    CutflowHistogram(class HistogramService* service,
                     const std::string& base_path,
                     const std::string& title,
                     int nbins, double xmin, double xmax)
        : m_service(service)
        , m_base_path(base_path)
        , m_title(title)
        , m_nbins(nbins)
        , m_xmin(xmin)
        , m_xmax(xmax) {}

    /**
     * @brief Register a cut stage
     */
    auto register_cut(const std::string& cut_name) -> void;

    /**
     * @brief Fill histogram at specific cut stage
     */
    auto fill(const std::string& cut_name, double value) -> void;

    /**
     * @brief Fill histogram at specific cut stage with weight
     */
    auto fill(const std::string& cut_name, double value, double weight) -> void;

    /**
     * @brief Get histogram for specific cut stage
     */
    auto get_histogram(const std::string& cut_name) -> std::optional<Hist1D>;

private:
    class HistogramService* m_service;
    std::string m_base_path;
    std::string m_title;
    int m_nbins;
    double m_xmin;
    double m_xmax;
    std::map<std::string, Hist1D> m_histograms;

    auto sanitize_name(const std::string& name) -> std::string;
};

}  // namespace Framework
