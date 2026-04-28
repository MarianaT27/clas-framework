#pragma once

#include <fmt/format.h>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

#include <clas12/framework/core/IService.hpp>
#include <clas12/framework/core/ServiceManager.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/OutputFileService.hpp>

#include <mh/mh.hpp>
#include <mh/io/root.hpp>

#include "TFile.h"
#include "TObject.h"

namespace framework {

class OutputFileService;

/**
 * @brief I/O registry for `mh::Histogram` instances owned by algorithms.
 *
 * Algorithms hold their histograms as value members and call
 * `register_hist(path, title, h)` once per histogram during `on_initialize()`.
 * The service stores a type-erased closure that calls `mh::to_root(h, …)` at
 * `finalize()` time and writes the result through `OutputFileService`.
 *
 * The service does NOT own any histograms. All it stores is a reference (via
 * the captured lambda) to the algorithm-owned object plus the path/title for
 * placement inside the ROOT file.
 */
class HistogramService : public Service {
   public:
    static constexpr std::string_view service_name() { return "HistogramService"; }

    explicit HistogramService(std::string name = "HistogramService")
        : Service(std::move(name)) {}

    auto initialize() -> StatusCode override {
        if (!m_output_svc) {
            m_output_svc = locate_service<OutputFileService>("OutputFileService");
        }
        if (!m_output_svc) {
            Log::error(name(), "OutputFileService not available via ServiceManager");
            return StatusCode::FAILURE();
        }
        if (!m_output_svc->is_file_open()) {
            Log::error(name(), "OutputFileService file is not open");
            return StatusCode::FAILURE();
        }
        Log::info(name(), "Initialized with OutputFileService");
        return StatusCode::SUCCESS();
    }

    auto finalize() -> StatusCode override {
        if (m_auto_write) write_histograms();
        Log::info(name(), "Finalized ({} histograms registered)", m_registry.size());
        return StatusCode::SUCCESS();
    }

    /**
     * @brief Register a histogram for ROOT writeout at finalize time.
     *
     * The caller owns @p h. The service captures it by reference inside a
     * type-erased to_root() closure; @p h must outlive the service's call to
     * `write_histograms()` (always true for algorithm members — algorithms
     * are finalized before services).
     */
    template <typename Hist>
    auto register_hist(std::string path, std::string title, Hist& h) -> void {
        std::string hname = extract_name(path);
        m_registry.push_back(Entry{
            std::move(path),
            title,
            [&h, hname = std::move(hname), title]() -> std::unique_ptr<TObject> {
                auto* root_obj = mh::to_root(h, hname.c_str(), title.c_str());
                root_obj->SetDirectory(nullptr);
                return std::unique_ptr<TObject>(root_obj);
            }
        });
    }

    /**
     * @brief Walk the registry, materialise each `TH*D*` via mh::to_root(),
     *        and write through OutputFileService. Called from finalize() when
     *        auto-write is enabled.
     */
    auto write_histograms() -> void;

    /// Disable automatic write on finalize (default: enabled).
    auto set_auto_write(bool enable) -> void { m_auto_write = enable; }

    /// Toggle post-write verification logging (default: disabled).
    auto set_verification(bool enable) -> void { m_verify_output = enable; }

    [[nodiscard]] auto num_histograms() const -> std::size_t { return m_registry.size(); }

    [[nodiscard]] auto get_output_file() const -> std::string {
        return m_output_svc ? m_output_svc->get_output_file() : "";
    }

   private:
    struct Entry {
        std::string path;
        std::string title;
        std::function<std::unique_ptr<TObject>()> to_root;
    };

    std::vector<Entry> m_registry;
    bool m_auto_write = true;
    bool m_verify_output = false;
    OutputFileService* m_output_svc = nullptr;

    static auto extract_name(const std::string& path) -> std::string;
    auto create_directories(TFile* file, const std::string& path) -> void;
};

}  // namespace framework
