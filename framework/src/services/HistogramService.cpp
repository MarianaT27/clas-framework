// Out-of-line definitions for HistogramService.

#include <clas12/framework/services/HistogramService.hpp>

#include <memory>
#include <mutex>
#include <string>

#include "TFile.h"
#include "TObject.h"

namespace framework {

auto HistogramService::write_histograms() -> void {
    if (!m_output_svc || !m_output_svc->is_file_open()) {
        Log::error(name(), "OutputFileService not available for writing histograms");
        return;
    }
    if (m_registry.empty()) {
        Log::info(name(), "No histograms to write");
        return;
    }

    Log::info(name(), "Writing {} histograms via OutputFileService", m_registry.size());

    std::lock_guard<std::mutex> lock(m_output_svc->mutex());
    TFile* file = m_output_svc->get_file();
    if (!file) return;

    std::size_t failed = 0;
    for (auto& entry : m_registry) {
        auto obj = entry.to_root();
        if (!obj) {
            ++failed;
            Log::error(name(), "to_root() returned null for '{}'", entry.path);
            continue;
        }
        create_directories(file, entry.path);
        if (obj->Write("", TObject::kOverwrite) <= 0) {
            ++failed;
            Log::error(name(), "Failed to write histogram '{}'", entry.path);
        }
    }

    Log::info(name(), "Wrote {} histograms", m_registry.size() - failed);
    if (failed > 0) Log::error(name(), "{} histogram(s) failed to write", failed);
}

auto HistogramService::extract_name(const std::string& path) -> std::string {
    size_t pos = path.find_last_of('/');
    return pos != std::string::npos ? path.substr(pos + 1) : path;
}

auto HistogramService::create_directories(TFile* file, const std::string& path) -> void {
    size_t pos = path.find_last_of('/');
    if (pos == std::string::npos || pos == 0) {
        file->cd();
        return;
    }

    std::string dir_path = path.substr(0, pos);
    if (dir_path[0] == '/') {
        dir_path = dir_path.substr(1);
    }

    file->cd();
    size_t start = 0;
    while (start < dir_path.length()) {
        size_t end = dir_path.find('/', start);
        if (end == std::string::npos) end = dir_path.length();
        std::string dir_name = dir_path.substr(start, end - start);
        if (!gDirectory->GetDirectory(dir_name.c_str())) {
            gDirectory->mkdir(dir_name.c_str());
        }
        gDirectory->cd(dir_name.c_str());
        start = end + 1;
    }
}

}  // namespace framework
