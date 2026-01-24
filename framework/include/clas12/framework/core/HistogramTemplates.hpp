#pragma once

#include <fmt/format.h>
#include <map>
#include <optional>
#include <string>
#include <vector>
#include <clas12/framework/core/Histogram.hpp>
#include <clas12/framework/services/HistogramService.hpp>

namespace clas12::framework {

/**
 * @brief Declarative histogram specification for 1D and 2D histograms
 *
 * HistSpec provides a declarative way to specify histogram parameters
 * that can be used for automatic booking and registration.
 *
 * Example:
 * @code
 * // 1D histogram specification
 * HistSpec momentum_spec{"/Electron/momentum", "Electron Momentum;p [GeV/c];Events", 100, 0.0, 10.0};
 *
 * // 2D histogram specification
 * HistSpec theta_phi_spec{"/Electron/theta_vs_phi", "#theta vs #phi;#phi [deg];#theta [deg]",
 *                         360, -180.0, 180.0, 180, 0.0, 180.0};
 * @endcode
 */
struct HistSpec {
    std::string path;   ///< Full path for the histogram (e.g., "/Electron/momentum")
    std::string title;  ///< Histogram title with axis labels (ROOT format)
    int nbins;          ///< Number of bins for x-axis
    double xmin;        ///< Minimum x value
    double xmax;        ///< Maximum x value

    // Optional 2D parameters
    std::optional<int> nbinsy;     ///< Number of bins for y-axis (2D only)
    std::optional<double> ymin;    ///< Minimum y value (2D only)
    std::optional<double> ymax;    ///< Maximum y value (2D only)

    /**
     * @brief Check if this is a 2D histogram specification
     */
    [[nodiscard]] auto is_2d() const -> bool {
        return nbinsy.has_value() && ymin.has_value() && ymax.has_value();
    }

    /**
     * @brief Create a 1D histogram specification
     */
    static auto hist1D(std::string path, std::string title,
                       int nbins, double xmin, double xmax) -> HistSpec {
        return HistSpec{std::move(path), std::move(title), nbins, xmin, xmax,
                        std::nullopt, std::nullopt, std::nullopt};
    }

    /**
     * @brief Create a 2D histogram specification
     */
    static auto hist2D(std::string path, std::string title,
                       int nbinsx, double xmin, double xmax,
                       int nbinsy, double ymin, double ymax) -> HistSpec {
        return HistSpec{std::move(path), std::move(title), nbinsx, xmin, xmax,
                        nbinsy, ymin, ymax};
    }
};


/**
 * @brief Template for creating per-sector histograms
 *
 * SectorHistograms automatically creates and manages histograms for each
 * detector sector (typically 6 sectors for CLAS12). This is useful for
 * studying sector-dependent effects.
 *
 * Example:
 * @code
 * SectorHistograms<6> h_momentum(hist_svc, "/Electron/momentum",
 *                                "Electron Momentum", 100, 0.0, 10.0);
 *
 * // Fill for a specific sector
 * h_momentum.fill(sector, electron.p());
 *
 * // Access individual sector histogram
 * TH1D* sector1_hist = h_momentum[1];
 * @endcode
 *
 * @tparam NSectors Number of sectors (default: 6 for CLAS12)
 */
template <int NSectors = 6>
class SectorHistograms {
   public:
    /**
     * @brief Construct sector histograms from a HistSpec
     * @param svc Histogram service for booking
     * @param spec Histogram specification (path will be suffixed with _S{sector})
     */
    SectorHistograms(HistogramService& svc, const HistSpec& spec)
        : m_base_path(spec.path), m_title(spec.title) {
        book_histograms(svc, spec.nbins, spec.xmin, spec.xmax);
    }

    /**
     * @brief Construct sector histograms with explicit parameters
     * @param svc Histogram service for booking
     * @param base_path Base path for histograms (will be suffixed with _S{sector})
     * @param title Base title for histograms
     * @param nbins Number of bins
     * @param xmin Minimum x value
     * @param xmax Maximum x value
     */
    SectorHistograms(HistogramService& svc, const std::string& base_path,
                     const std::string& title, int nbins, double xmin, double xmax)
        : m_base_path(base_path), m_title(title) {
        book_histograms(svc, nbins, xmin, xmax);
    }

    /**
     * @brief Fill histogram for a specific sector
     * @param sector Sector number (1-based indexing)
     * @param value Value to fill
     */
    auto fill(int sector, double value) -> void {
        if (auto it = m_histograms.find(sector); it != m_histograms.end()) {
            it->second.fill(value);
        }
    }

    /**
     * @brief Fill histogram for a specific sector with weight
     * @param sector Sector number (1-based indexing)
     * @param value Value to fill
     * @param weight Weight for the fill
     */
    auto fill(int sector, double value, double weight) -> void {
        if (auto it = m_histograms.find(sector); it != m_histograms.end()) {
            it->second.fill(value, weight);
        }
    }

    /**
     * @brief Fill histogram if condition is true
     * @param condition Condition to check
     * @param sector Sector number
     * @param value Value to fill
     */
    auto fill_if(bool condition, int sector, double value) -> void {
        if (condition) {
            fill(sector, value);
        }
    }

    /**
     * @brief Get histogram for a specific sector
     * @param sector Sector number (1-based indexing)
     * @return Pointer to TH1D, or nullptr if sector is invalid
     */
    auto operator[](int sector) -> TH1D* {
        if (auto it = m_histograms.find(sector); it != m_histograms.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    /**
     * @brief Get histogram wrapper for a specific sector
     * @param sector Sector number (1-based indexing)
     * @return Optional Hist1D wrapper
     */
    auto get(int sector) -> std::optional<Hist1D> {
        if (auto it = m_histograms.find(sector); it != m_histograms.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    /**
     * @brief Get the number of sectors
     */
    [[nodiscard]] static constexpr auto num_sectors() -> int {
        return NSectors;
    }

    /**
     * @brief Get the base path
     */
    [[nodiscard]] auto base_path() const -> const std::string& {
        return m_base_path;
    }

    /**
     * @brief Get all sector histograms
     */
    [[nodiscard]] auto histograms() const -> const std::map<int, Hist1D>& {
        return m_histograms;
    }

   private:
    std::string m_base_path;
    std::string m_title;
    std::map<int, Hist1D> m_histograms;

    auto book_histograms(HistogramService& svc, int nbins, double xmin, double xmax) -> void {
        for (int s = 1; s <= NSectors; ++s) {
            std::string path = fmt::format("{}_S{}", m_base_path, s);
            std::string title = fmt::format("{} (Sector {})", m_title, s);
            TH1D* hist = svc.book_hist1D(path, title, nbins, xmin, xmax);
            m_histograms[s] = Hist1D(hist);
        }
    }
};


/**
 * @brief Template for creating per-sector 2D histograms
 *
 * Similar to SectorHistograms but for 2D histograms.
 *
 * @tparam NSectors Number of sectors (default: 6 for CLAS12)
 */
template <int NSectors = 6>
class SectorHistograms2D {
   public:
    /**
     * @brief Construct sector 2D histograms from a HistSpec
     */
    SectorHistograms2D(HistogramService& svc, const HistSpec& spec)
        : m_base_path(spec.path), m_title(spec.title) {
        if (!spec.is_2d()) {
            throw std::invalid_argument("HistSpec must be 2D for SectorHistograms2D");
        }
        book_histograms(svc, spec.nbins, spec.xmin, spec.xmax,
                        *spec.nbinsy, *spec.ymin, *spec.ymax);
    }

    /**
     * @brief Construct sector 2D histograms with explicit parameters
     */
    SectorHistograms2D(HistogramService& svc, const std::string& base_path,
                       const std::string& title,
                       int nbinsx, double xmin, double xmax,
                       int nbinsy, double ymin, double ymax)
        : m_base_path(base_path), m_title(title) {
        book_histograms(svc, nbinsx, xmin, xmax, nbinsy, ymin, ymax);
    }

    /**
     * @brief Fill histogram for a specific sector
     */
    auto fill(int sector, double x, double y) -> void {
        if (auto it = m_histograms.find(sector); it != m_histograms.end()) {
            it->second.fill(x, y);
        }
    }

    /**
     * @brief Fill histogram for a specific sector with weight
     */
    auto fill(int sector, double x, double y, double weight) -> void {
        if (auto it = m_histograms.find(sector); it != m_histograms.end()) {
            it->second.fill(x, y, weight);
        }
    }

    /**
     * @brief Get histogram for a specific sector
     */
    auto operator[](int sector) -> TH2D* {
        if (auto it = m_histograms.find(sector); it != m_histograms.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    /**
     * @brief Get histogram wrapper for a specific sector
     */
    auto get(int sector) -> std::optional<Hist2D> {
        if (auto it = m_histograms.find(sector); it != m_histograms.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    [[nodiscard]] static constexpr auto num_sectors() -> int {
        return NSectors;
    }

   private:
    std::string m_base_path;
    std::string m_title;
    std::map<int, Hist2D> m_histograms;

    auto book_histograms(HistogramService& svc,
                         int nbinsx, double xmin, double xmax,
                         int nbinsy, double ymin, double ymax) -> void {
        for (int s = 1; s <= NSectors; ++s) {
            std::string path = fmt::format("{}_S{}", m_base_path, s);
            std::string title = fmt::format("{} (Sector {})", m_title, s);
            TH2D* hist = svc.book_hist2D(path, title, nbinsx, xmin, xmax, nbinsy, ymin, ymax);
            m_histograms[s] = Hist2D(hist);
        }
    }
};


/**
 * @brief Template for creating histograms at each cut stage
 *
 * CutStageHistograms creates a histogram for each registered cut stage,
 * allowing you to track how a variable distribution changes as cuts are applied.
 *
 * Example:
 * @code
 * CutStageHistograms h_momentum(hist_svc, "/Electron/cutflow/momentum",
 *                               "Electron Momentum", 100, 0.0, 10.0,
 *                               {"All", "Fiducial", "PID", "Momentum"});
 *
 * // Fill after each cut
 * h_momentum.fill_after_cut("All", electron.p());
 * if (pass_fiducial) h_momentum.fill_after_cut("Fiducial", electron.p());
 * if (pass_pid) h_momentum.fill_after_cut("PID", electron.p());
 * if (pass_momentum) h_momentum.fill_after_cut("Momentum", electron.p());
 * @endcode
 */
class CutStageHistograms {
   public:
    /**
     * @brief Construct cut stage histograms from a HistSpec and cut names
     * @param svc Histogram service for booking
     * @param spec Histogram specification
     * @param cut_names List of cut stage names
     */
    CutStageHistograms(HistogramService& svc, const HistSpec& spec,
                       const std::vector<std::string>& cut_names)
        : m_base_path(spec.path), m_title(spec.title) {
        book_histograms(svc, spec.nbins, spec.xmin, spec.xmax, cut_names);
    }

    /**
     * @brief Construct cut stage histograms with explicit parameters
     * @param svc Histogram service for booking
     * @param base_path Base path for histograms
     * @param title Base title for histograms
     * @param nbins Number of bins
     * @param xmin Minimum x value
     * @param xmax Maximum x value
     * @param cut_names List of cut stage names
     */
    CutStageHistograms(HistogramService& svc, const std::string& base_path,
                       const std::string& title, int nbins, double xmin, double xmax,
                       const std::vector<std::string>& cut_names)
        : m_base_path(base_path), m_title(title) {
        book_histograms(svc, nbins, xmin, xmax, cut_names);
    }

    /**
     * @brief Fill histogram at a specific cut stage
     * @param cut_name Name of the cut stage
     * @param value Value to fill
     */
    auto fill_after_cut(const std::string& cut_name, double value) -> void {
        if (auto it = m_histograms.find(cut_name); it != m_histograms.end()) {
            it->second.fill(value);
        }
    }

    /**
     * @brief Fill histogram at a specific cut stage with weight
     * @param cut_name Name of the cut stage
     * @param value Value to fill
     * @param weight Weight for the fill
     */
    auto fill_after_cut(const std::string& cut_name, double value, double weight) -> void {
        if (auto it = m_histograms.find(cut_name); it != m_histograms.end()) {
            it->second.fill(value, weight);
        }
    }

    /**
     * @brief Fill histogram if condition is true
     * @param condition Condition to check
     * @param cut_name Name of the cut stage
     * @param value Value to fill
     */
    auto fill_if(bool condition, const std::string& cut_name, double value) -> void {
        if (condition) {
            fill_after_cut(cut_name, value);
        }
    }

    /**
     * @brief Get histogram for a specific cut stage
     * @param cut_name Name of the cut stage
     * @return Pointer to TH1D, or nullptr if cut_name is not found
     */
    auto operator[](const std::string& cut_name) -> TH1D* {
        if (auto it = m_histograms.find(cut_name); it != m_histograms.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    /**
     * @brief Get histogram wrapper for a specific cut stage
     * @param cut_name Name of the cut stage
     * @return Optional Hist1D wrapper
     */
    auto get(const std::string& cut_name) -> std::optional<Hist1D> {
        if (auto it = m_histograms.find(cut_name); it != m_histograms.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    /**
     * @brief Get all cut stage names
     */
    [[nodiscard]] auto cut_names() const -> std::vector<std::string> {
        std::vector<std::string> names;
        names.reserve(m_cut_order.size());
        for (const auto& name : m_cut_order) {
            names.push_back(name);
        }
        return names;
    }

    /**
     * @brief Get the base path
     */
    [[nodiscard]] auto base_path() const -> const std::string& {
        return m_base_path;
    }

    /**
     * @brief Get all histograms
     */
    [[nodiscard]] auto histograms() const -> const std::map<std::string, Hist1D>& {
        return m_histograms;
    }

   private:
    std::string m_base_path;
    std::string m_title;
    std::map<std::string, Hist1D> m_histograms;
    std::vector<std::string> m_cut_order;  // Preserve order of cuts

    auto book_histograms(HistogramService& svc, int nbins, double xmin, double xmax,
                         const std::vector<std::string>& cut_names) -> void {
        m_cut_order = cut_names;
        for (const auto& cut_name : cut_names) {
            std::string sanitized = sanitize_name(cut_name);
            std::string path = fmt::format("{}_{}", m_base_path, sanitized);
            std::string title = fmt::format("{} (after {})", m_title, cut_name);
            TH1D* hist = svc.book_hist1D(path, title, nbins, xmin, xmax);
            m_histograms[cut_name] = Hist1D(hist);
        }
    }

    static auto sanitize_name(const std::string& name) -> std::string {
        std::string result = name;
        for (char& c : result) {
            if (std::isspace(static_cast<unsigned char>(c)) ||
                c == '>' || c == '<' || c == '/' || c == '\\' ||
                c == ':' || c == '*' || c == '?' || c == '"' ||
                c == '|') {
                c = '_';
            }
        }
        return result;
    }
};


/**
 * @brief Helper class for declarative histogram booking
 *
 * HistogramRegistry provides a way to declare histograms declaratively
 * and have them automatically booked during initialization.
 *
 * Example:
 * @code
 * class MyAlgorithm : public Algorithm {
 *     HistogramRegistry m_histograms;
 *
 *     auto initialize() -> StatusCode override {
 *         m_histograms.set_service(&hist_svc);
 *         m_histograms.declare("/Electron/momentum", "p;p [GeV/c];Events", 100, 0, 10);
 *         m_histograms.declare("/Electron/theta", "#theta;#theta [deg];Events", 180, 0, 180);
 *         m_histograms.book_all();
 *         return StatusCode::SUCCESS();
 *     }
 *
 *     auto execute(Event& event) -> StatusCode override {
 *         m_histograms["momentum"].fill(electron.p());
 *         return StatusCode::SUCCESS();
 *     }
 * };
 * @endcode
 */
class HistogramRegistry {
   public:
    HistogramRegistry() = default;

    /**
     * @brief Set the histogram service
     */
    auto set_service(HistogramService* svc) -> void {
        m_service = svc;
    }

    /**
     * @brief Declare a 1D histogram specification
     * @param path Full path for the histogram
     * @param title Histogram title with axis labels
     * @param nbins Number of bins
     * @param xmin Minimum x value
     * @param xmax Maximum x value
     */
    auto declare(const std::string& path, const std::string& title,
                 int nbins, double xmin, double xmax) -> void {
        m_specs_1d.push_back(HistSpec::hist1D(path, title, nbins, xmin, xmax));
    }

    /**
     * @brief Declare a 2D histogram specification
     */
    auto declare_2d(const std::string& path, const std::string& title,
                    int nbinsx, double xmin, double xmax,
                    int nbinsy, double ymin, double ymax) -> void {
        m_specs_2d.push_back(HistSpec::hist2D(path, title, nbinsx, xmin, xmax,
                                               nbinsy, ymin, ymax));
    }

    /**
     * @brief Book all declared histograms
     * @return true if all histograms were booked successfully
     */
    auto book_all() -> bool {
        if (!m_service) {
            return false;
        }

        for (const auto& spec : m_specs_1d) {
            TH1D* hist = m_service->book_hist1D(spec.path, spec.title,
                                                 spec.nbins, spec.xmin, spec.xmax);
            if (hist) {
                std::string name = extract_name(spec.path);
                m_hist1d[name] = Hist1D(hist);
            }
        }

        for (const auto& spec : m_specs_2d) {
            TH2D* hist = m_service->book_hist2D(spec.path, spec.title,
                                                 spec.nbins, spec.xmin, spec.xmax,
                                                 *spec.nbinsy, *spec.ymin, *spec.ymax);
            if (hist) {
                std::string name = extract_name(spec.path);
                m_hist2d[name] = Hist2D(hist);
            }
        }

        return true;
    }

    /**
     * @brief Get a 1D histogram by name
     */
    auto operator[](const std::string& name) -> Hist1D& {
        return m_hist1d[name];
    }

    /**
     * @brief Get a 2D histogram by name
     */
    auto hist2d(const std::string& name) -> Hist2D& {
        return m_hist2d[name];
    }

    /**
     * @brief Get number of declared 1D histograms
     */
    [[nodiscard]] auto num_hist1d() const -> size_t {
        return m_specs_1d.size();
    }

    /**
     * @brief Get number of declared 2D histograms
     */
    [[nodiscard]] auto num_hist2d() const -> size_t {
        return m_specs_2d.size();
    }

   private:
    HistogramService* m_service = nullptr;
    std::vector<HistSpec> m_specs_1d;
    std::vector<HistSpec> m_specs_2d;
    std::map<std::string, Hist1D> m_hist1d;
    std::map<std::string, Hist2D> m_hist2d;

    static auto extract_name(const std::string& path) -> std::string {
        size_t pos = path.find_last_of('/');
        if (pos != std::string::npos) {
            return path.substr(pos + 1);
        }
        return path;
    }
};

/**
 * @brief Pre-defined histogram templates for common physics analyses
 *
 * These templates provide standard histogram configurations for common
 * particle physics quantities, reducing boilerplate in analysis code.
 *
 * Templates support "before" and "after" cuts stages for systematic studies.
 */
namespace HistogramTemplates {

/**
 * @brief Standard particle kinematics histograms (comprehensive)
 *
 * Creates histograms for:
 * - Basic kinematics: p, px, py, pz, theta, phi, E, pt
 * - Vertex: vx, vy, vz, vr
 * - 2D correlations: p vs theta, phi vs theta, pt vs eta
 */
struct ParticleKinematics {
    static auto create(HistogramService* svc, const std::string& base_path)
        -> std::unique_ptr<HistogramGroup> {
        auto group = std::make_unique<HistogramGroup>(svc, base_path);

        // Basic kinematics (1D)
        group->add_hist1D("p", "Momentum;p [GeV/c];Events", 100, 0.0, 10.0);
        group->add_hist1D("px", "Momentum X;p_{x} [GeV/c];Events", 100, -5.0, 5.0);
        group->add_hist1D("py", "Momentum Y;p_{y} [GeV/c];Events", 100, -5.0, 5.0);
        group->add_hist1D("pz", "Momentum Z;p_{z} [GeV/c];Events", 100, -2.0, 12.0);
        group->add_hist1D("pt", "Transverse Momentum;p_{T} [GeV/c];Events", 100, 0.0, 5.0);
        group->add_hist1D("E", "Energy;E [GeV];Events", 100, 0.0, 12.0);
        group->add_hist1D("theta", "Polar Angle;#theta [deg];Events", 180, 0.0, 180.0);
        group->add_hist1D("phi", "Azimuthal Angle;#phi [deg];Events", 360, -180.0, 180.0);

        // Vertex (1D)
        group->add_hist1D("vx", "Vertex X;v_{x} [cm];Events", 100, -5.0, 5.0);
        group->add_hist1D("vy", "Vertex Y;v_{y} [cm];Events", 100, -5.0, 5.0);
        group->add_hist1D("vz", "Vertex Z;v_{z} [cm];Events", 200, -40.0, 40.0);
        group->add_hist1D("vr", "Vertex R;v_{r} [cm];Events", 100, 0.0, 5.0);

        // 2D correlations
        group->add_hist2D("p_vs_theta", "p vs #theta;#theta [deg];p [GeV/c]",
                          90, 0.0, 90.0, 100, 0.0, 10.0);
        group->add_hist2D("phi_vs_theta", "#phi vs #theta;#theta [deg];#phi [deg]",
                          90, 0.0, 90.0, 180, -180.0, 180.0);
        group->add_hist2D("pt_vs_eta", "p_{T} vs #eta;#eta;p_{T} [GeV/c]",
                          100, -2.0, 6.0, 100, 0.0, 5.0);
        group->add_hist2D("vz_vs_phi", "v_{z} vs #phi;#phi [deg];v_{z} [cm]",
                          180, -180.0, 180.0, 100, -40.0, 40.0);

        return group;
    }
};

/**
 * @brief Comprehensive electron histograms with before/after cuts stages
 *
 * Creates histograms for:
 * - Kinematics: p, theta, phi, vz
 * - PID: chi2pid, beta, sampling fraction
 * - Calorimeter: PCAL/ECIN/ECOUT energies, SF by layer
 * - DC: chi2/NDF, edge distance
 * - 2D correlations for cut optimization
 */
struct ElectronHistograms {
    static auto create(HistogramService* svc, const std::string& base_path)
        -> std::unique_ptr<HistogramGroup> {
        auto group = std::make_unique<HistogramGroup>(svc, base_path);

        // ============================================================
        // Basic Kinematics
        // ============================================================
        group->add_hist1D("p", "Electron Momentum;p [GeV/c];Events", 100, 0.0, 12.0);
        group->add_hist1D("theta", "Electron #theta;#theta [deg];Events", 90, 0.0, 45.0);
        group->add_hist1D("phi", "Electron #phi;#phi [deg];Events", 360, -180.0, 180.0);
        group->add_hist1D("pt", "Electron p_{T};p_{T} [GeV/c];Events", 100, 0.0, 4.0);
        group->add_hist1D("sector", "Electron Sector;Sector;Events", 7, 0.5, 7.5);

        // ============================================================
        // Vertex
        // ============================================================
        group->add_hist1D("vz", "Vertex Z;v_{z} [cm];Events", 200, -40.0, 40.0);
        group->add_hist1D("vx", "Vertex X;v_{x} [cm];Events", 100, -2.0, 2.0);
        group->add_hist1D("vy", "Vertex Y;v_{y} [cm];Events", 100, -2.0, 2.0);

        // ============================================================
        // Particle Identification
        // ============================================================
        group->add_hist1D("chi2pid", "Electron #chi^{2}_{PID};#chi^{2}_{PID};Events", 200, -10.0, 10.0);
        group->add_hist1D("beta", "Electron #beta;#beta;Events", 100, 0.9, 1.1);
        group->add_hist1D("nphe", "Number of Photoelectrons;N_{phe};Events", 100, 0.0, 50.0);

        // ============================================================
        // Calorimeter
        // ============================================================
        group->add_hist1D("pcal_E", "PCAL Energy;E_{PCAL} [GeV];Events", 100, 0.0, 2.0);
        group->add_hist1D("ecin_E", "ECIN Energy;E_{ECIN} [GeV];Events", 100, 0.0, 2.0);
        group->add_hist1D("ecout_E", "ECOUT Energy;E_{ECOUT} [GeV];Events", 100, 0.0, 2.0);
        group->add_hist1D("ecal_E", "Total ECal Energy;E_{ECal} [GeV];Events", 100, 0.0, 4.0);
        group->add_hist1D("sampling_fraction", "Sampling Fraction;SF;Events", 100, 0.0, 0.5);

        // ============================================================
        // Drift Chambers
        // ============================================================
        group->add_hist1D("dc_chi2_R1", "DC #chi^{2}/NDF R1;#chi^{2}/NDF;Events", 100, 0.0, 10.0);
        group->add_hist1D("dc_chi2_R2", "DC #chi^{2}/NDF R2;#chi^{2}/NDF;Events", 100, 0.0, 10.0);
        group->add_hist1D("dc_chi2_R3", "DC #chi^{2}/NDF R3;#chi^{2}/NDF;Events", 100, 0.0, 10.0);

        // ============================================================
        // Fiducial Cuts (PCAL)
        // ============================================================
        group->add_hist1D("pcal_lu", "PCAL Local U;PCAL_{lu} [cm];Events", 100, 0.0, 450.0);
        group->add_hist1D("pcal_lv", "PCAL Local V;PCAL_{lv} [cm];Events", 100, 0.0, 450.0);
        group->add_hist1D("pcal_lw", "PCAL Local W;PCAL_{lw} [cm];Events", 100, 0.0, 450.0);

        // ============================================================
        // 2D Correlations for Cut Studies
        // ============================================================
        group->add_hist2D("sf_vs_p", "SF vs p;p [GeV/c];SF",
                          100, 0.0, 12.0, 100, 0.0, 0.5);
        group->add_hist2D("chi2pid_vs_p", "#chi^{2}_{PID} vs p;p [GeV/c];#chi^{2}_{PID}",
                          100, 0.0, 12.0, 200, -10.0, 10.0);
        group->add_hist2D("vz_vs_phi", "v_{z} vs #phi;#phi [deg];v_{z} [cm]", 180, -180.0, 180.0, 100, -40.0, 40.0);
        group->add_hist2D("vz_vs_theta", "v_{z} vs #theta;#theta [deg];v_{z} [cm]",
                          90, 0.0, 45.0, 100, -40.0, 40.0);
        group->add_hist2D("theta_vs_phi", "#theta vs #phi;#phi [deg];#theta [deg]",
                          180, -180.0, 180.0, 90, 0.0, 45.0);
        group->add_hist2D("p_vs_theta", "p vs #theta;#theta [deg];p [GeV/c]",
                          90, 0.0, 45.0, 100, 0.0, 12.0);
        group->add_hist2D("pcal_lv_vs_lu", "PCAL lv vs lu;PCAL_{lu} [cm];PCAL_{lv} [cm]",
                          100, 0.0, 450.0, 100, 0.0, 450.0);
        group->add_hist2D("ecin_vs_pcal", "E_{ECIN} vs E_{PCAL};E_{PCAL} [GeV];E_{ECIN} [GeV]",
                          100, 0.0, 2.0, 100, 0.0, 2.0);

        return group;
    }

    /**
     * @brief Create before/after cut histograms
     * @param svc Histogram service
     * @param base_path Base path (e.g., "/Electron")
     * @param stages List of cut stages {"before_cuts", "after_fiducial", "after_pid", ...}
     */
    static auto create_with_stages(HistogramService* svc, const std::string& base_path,
                                   const std::vector<std::string>& stages)
        -> std::map<std::string, std::unique_ptr<HistogramGroup>> {
        std::map<std::string, std::unique_ptr<HistogramGroup>> groups;
        for (const auto& stage : stages) {
            std::string path = fmt::format("{}/{}", base_path, stage);
            groups[stage] = create(svc, path);
        }
        return groups;
    }
};

/**
 * @brief Standard electron identification histograms (legacy, simpler version)
 */
struct ElectronID {
    static auto create(HistogramService* svc, const std::string& base_path)
        -> std::unique_ptr<HistogramGroup> {
        auto group = std::make_unique<HistogramGroup>(svc, base_path);

        group->add_hist1D("chi2pid", "#chi^{2}_{PID};#chi^{2}_{PID};Events", 200, -10.0, 10.0);
        group->add_hist1D("vz", "Vertex Z;v_{z} [cm];Events", 200, -40.0, 40.0);
        group->add_hist1D("sampling_fraction", "Sampling Fraction;SF;Events", 100, 0.0, 0.5);
        group->add_hist2D("sf_vs_p", "SF vs p;p [GeV/c];SF",
                          100, 0.0, 12.0, 100, 0.0, 0.5);

        return group;
    }
};

/**
 * @brief Comprehensive hadron (pion/kaon/proton) histograms
 *
 * Creates histograms for:
 * - Kinematics: p, theta, phi, pt
 * - PID: chi2pid, beta, time-of-flight
 * - 2D correlations for cut optimization
 */
struct HadronHistograms {
    static auto create(HistogramService* svc, const std::string& base_path,
                       [[maybe_unused]] double mass_hypothesis = 0.13957)  // Default: pion mass
        -> std::unique_ptr<HistogramGroup> {
        auto group = std::make_unique<HistogramGroup>(svc, base_path);

        // ============================================================
        // Basic Kinematics
        // ============================================================
        group->add_hist1D("p", "Momentum;p [GeV/c];Events", 100, 0.0, 8.0);
        group->add_hist1D("theta", "#theta;#theta [deg];Events", 180, 0.0, 180.0);
        group->add_hist1D("phi", "#phi;#phi [deg];Events", 360, -180.0, 180.0);
        group->add_hist1D("pt", "p_{T};p_{T} [GeV/c];Events", 100, 0.0, 3.0);
        group->add_hist1D("eta", "#eta;#eta;Events", 100, -2.0, 6.0);
        group->add_hist1D("sector", "Sector;Sector;Events", 7, 0.5, 7.5);

        // ============================================================
        // Vertex
        // ============================================================
        group->add_hist1D("vz", "Vertex Z;v_{z} [cm];Events", 200, -40.0, 40.0);
        group->add_hist1D("vx", "Vertex X;v_{x} [cm];Events", 100, -2.0, 2.0);
        group->add_hist1D("vy", "Vertex Y;v_{y} [cm];Events", 100, -2.0, 2.0);
        group->add_hist1D("dvz", "#Deltav_{z} (hadron - electron);#Deltav_{z} [cm];Events", 100, -20.0, 20.0);

        // ============================================================
        // Particle Identification
        // ============================================================
        group->add_hist1D("chi2pid", "#chi^{2}_{PID};#chi^{2}_{PID};Events", 200, -15.0, 15.0);
        group->add_hist1D("beta", "#beta;#beta;Events", 100, 0.0, 1.5);
        group->add_hist1D("delta_beta", "#Delta#beta;#Delta#beta;Events", 100, -0.2, 0.2);

        // ============================================================
        // Time of Flight
        // ============================================================
        group->add_hist1D("tof_time", "TOF Time;t [ns];Events", 100, 0.0, 100.0);
        group->add_hist1D("tof_path", "TOF Path Length;L [cm];Events", 100, 0.0, 800.0);

        // ============================================================
        // 2D Correlations for Cut Studies
        // ============================================================
        group->add_hist2D("beta_vs_p", "#beta vs p;p [GeV/c];#beta",
                          100, 0.0, 8.0, 100, 0.0, 1.5);
        group->add_hist2D("chi2pid_vs_p", "#chi^{2}_{PID} vs p;p [GeV/c];#chi^{2}_{PID}",
                          100, 0.0, 8.0, 200, -15.0, 15.0);
        group->add_hist2D("delta_beta_vs_p", "#Delta#beta vs p;p [GeV/c];#Delta#beta",
                          100, 0.0, 8.0, 100, -0.2, 0.2);
        group->add_hist2D("theta_vs_phi", "#theta vs #phi;#phi [deg];#theta [deg]",
                          180, -180.0, 180.0, 90, 0.0, 180.0);
        group->add_hist2D("p_vs_theta", "p vs #theta;#theta [deg];p [GeV/c]",
                          90, 0.0, 180.0, 100, 0.0, 8.0);
        group->add_hist2D("vz_vs_phi", "v_{z} vs #phi;#phi [deg];v_{z} [cm]",
                          180, -180.0, 180.0, 100, -40.0, 40.0);

        return group;
    }

    /**
     * @brief Create before/after cut histograms for hadrons
     */
    static auto create_with_stages(HistogramService* svc, const std::string& base_path,
                                   const std::vector<std::string>& stages,
                                   double mass_hypothesis = 0.13957)
        -> std::map<std::string, std::unique_ptr<HistogramGroup>> {
        std::map<std::string, std::unique_ptr<HistogramGroup>> groups;
        for (const auto& stage : stages) {
            std::string path = fmt::format("{}/{}", base_path, stage);
            groups[stage] = create(svc, path, mass_hypothesis);
        }
        return groups;
    }
};

/**
 * @brief Standard photon identification histograms
 */
struct PhotonID {
    static auto create(HistogramService* svc, const std::string& base_path)
        -> std::unique_ptr<HistogramGroup> {
        auto group = std::make_unique<HistogramGroup>(svc, base_path);

        group->add_hist1D("beta", "Photon #beta;#beta;Events", 100, 0.0, 1.5);
        group->add_hist1D("energy", "Photon Energy;E [GeV];Events", 100, 0.0, 5.0);
        group->add_hist1D("theta", "Photon #theta;#theta [deg];Events", 90, 0.0, 45.0);
        group->add_hist1D("phi", "Photon #phi;#phi [deg];Events", 360, -180.0, 180.0);
        group->add_hist2D("beta_vs_p", "#beta vs p;p [GeV/c];#beta",
                          100, 0.0, 5.0, 100, 0.0, 1.5);
        group->add_hist2D("E_vs_theta", "E vs #theta;#theta [deg];E [GeV]",
                          90, 0.0, 45.0, 100, 0.0, 5.0);

        return group;
    }
};

/**
 * @brief Standard pion identification histograms (legacy)
 */
struct PionID {
    static auto create(HistogramService* svc, const std::string& base_path)
        -> std::unique_ptr<HistogramGroup> {
        auto group = std::make_unique<HistogramGroup>(svc, base_path);

        group->add_hist1D("chi2pid", "#chi^{2}_{PID};#chi^{2}_{PID};Events", 200, -15.0, 15.0);
        group->add_hist1D("beta", "#beta;#beta;Events", 100, 0.0, 1.5);
        group->add_hist1D("p", "Momentum;p [GeV/c];Events", 100, 0.0, 8.0);
        group->add_hist1D("theta", "#theta;#theta [deg];Events", 180, 0.0, 180.0);
        group->add_hist2D("beta_vs_p", "#beta vs p;p [GeV/c];#beta",
                          100, 0.0, 8.0, 100, 0.0, 1.5);
        group->add_hist2D("chi2pid_vs_p", "#chi^{2}_{PID} vs p;p [GeV/c];#chi^{2}_{PID}",
                          100, 0.0, 8.0, 200, -15.0, 15.0);

        return group;
    }
};

/**
 * @brief DIS kinematics histograms
 *
 * Creates histograms for DIS variables:
 * - Q2, W, x_Bjorken, y, nu
 * - 2D correlations
 */
struct DISKinematics {
    static auto create(HistogramService* svc, const std::string& base_path)
        -> std::unique_ptr<HistogramGroup> {
        auto group = std::make_unique<HistogramGroup>(svc, base_path);

        // 1D DIS variables
        group->add_hist1D("Q2", "Q^{2};Q^{2} [GeV^{2}];Events", 100, 0.0, 12.0);
        group->add_hist1D("W", "W;W [GeV];Events", 100, 0.0, 5.0);
        group->add_hist1D("xB", "x_{Bjorken};x_{B};Events", 100, 0.0, 1.0);
        group->add_hist1D("y", "y;y;Events", 100, 0.0, 1.0);
        group->add_hist1D("nu", "#nu;#nu [GeV];Events", 100, 0.0, 12.0);

        // 2D correlations
        group->add_hist2D("Q2_vs_W", "Q^{2} vs W;W [GeV];Q^{2} [GeV^{2}]",
                          100, 0.0, 5.0, 100, 0.0, 12.0);
        group->add_hist2D("Q2_vs_xB", "Q^{2} vs x_{B};x_{B};Q^{2} [GeV^{2}]",
                          100, 0.0, 1.0, 100, 0.0, 12.0);
        group->add_hist2D("y_vs_xB", "y vs x_{B};x_{B};y",
                          100, 0.0, 1.0, 100, 0.0, 1.0);
        group->add_hist2D("W_vs_xB", "W vs x_{B};x_{B};W [GeV]",
                          100, 0.0, 1.0, 100, 0.0, 5.0);

        return group;
    }
};

/**
 * @brief Missing mass/momentum histograms
 */
struct MissingKinematics {
    static auto create(HistogramService* svc, const std::string& base_path)
        -> std::unique_ptr<HistogramGroup> {
        auto group = std::make_unique<HistogramGroup>(svc, base_path);

        group->add_hist1D("Mx", "Missing Mass;M_{x} [GeV];Events", 200, 0.0, 4.0);
        group->add_hist1D("Mx2", "Missing Mass Squared;M_{x}^{2} [GeV^{2}];Events", 200, -2.0, 8.0);
        group->add_hist1D("Pmiss", "Missing Momentum;P_{miss} [GeV/c];Events", 100, 0.0, 5.0);
        group->add_hist1D("Emiss", "Missing Energy;E_{miss} [GeV];Events", 100, -2.0, 5.0);
        group->add_hist2D("Mx_vs_Q2", "M_{x} vs Q^{2};Q^{2} [GeV^{2}];M_{x} [GeV]",
                          100, 0.0, 12.0, 100, 0.0, 4.0);
        group->add_hist2D("Mx_vs_W", "M_{x} vs W;W [GeV];M_{x} [GeV]",
                          100, 0.0, 5.0, 100, 0.0, 4.0);

        return group;
    }
};

}  // namespace HistogramTemplates

}  // namespace clas12::framework
