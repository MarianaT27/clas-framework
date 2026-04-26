// Out-of-line definitions for HistogramService.
//
// The histogram service is included via ApplicationBuilder.hpp into every
// analysis translation unit, so every function body defined inline used to
// cost compile time in hundreds of TUs. This .cpp takes the heavy
// ROOT-touching bodies (atomic→ROOT conversion, TFile writes, directory
// creation, clones) out of the header.

#include <clas12/framework/services/HistogramService.hpp>

#include <memory>
#include <mutex>
#include <string>
#include <utility>

#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"
#include "TObject.h"
#include "TProfile.h"

namespace clas12::framework {

auto HistogramService::merge_histograms() -> void {
    for (const auto& [path, atomic_hist] : m_atomic_hist1D) {
        if (m_hist1D.find(path) != m_hist1D.end()) {
            atomic_hist->merge_into(m_hist1D[path].get());
        } else {
            std::string hist_name = extract_name(path);
            const auto& meta = m_atomic_hist1D_meta[path];
            auto root_hist = atomic_hist->to_root(hist_name, meta.title);
            root_hist->SetDirectory(nullptr);
            m_hist1D[path] = std::move(root_hist);
        }
    }

    for (const auto& [path, atomic_hist] : m_atomic_hist2D) {
        if (m_hist2D.find(path) != m_hist2D.end()) {
            atomic_hist->merge_into(m_hist2D[path].get());
        } else {
            std::string hist_name = extract_name(path);
            const auto& meta = m_atomic_hist2D_meta[path];
            auto root_hist = atomic_hist->to_root(hist_name, meta.title);
            root_hist->SetDirectory(nullptr);
            m_hist2D[path] = std::move(root_hist);
        }
    }

    for (const auto& [path, atomic_hist] : m_atomic_hist3D) {
        if (m_hist3D.find(path) != m_hist3D.end()) {
            atomic_hist->merge_into(m_hist3D[path].get());
        } else {
            std::string hist_name = extract_name(path);
            const auto& meta = m_atomic_hist3D_meta[path];
            auto root_hist = atomic_hist->to_root(hist_name, meta.title);
            root_hist->SetDirectory(nullptr);
            m_hist3D[path] = std::move(root_hist);
        }
    }

    if (!m_atomic_hist1D.empty() || !m_atomic_hist2D.empty() || !m_atomic_hist3D.empty()) {
        Log::info(name(),
                  "Converted {} atomic 1D, {} atomic 2D, {} atomic 3D histograms to ROOT format",
                  m_atomic_hist1D.size(), m_atomic_hist2D.size(), m_atomic_hist3D.size());
    }
}

auto HistogramService::book_profile(const std::string& path, const std::string& title,
                                    int nbins, double xmin, double xmax) -> TProfile* {
    if (m_profiles.find(path) != m_profiles.end()) {
        Log::warning(name(), "Profile '{}' already exists", path);
        return m_profiles[path].get();
    }

    std::string hname = extract_name(path);
    auto prof = std::make_unique<TProfile>(hname.c_str(), title.c_str(),
                                           nbins, xmin, xmax);
    prof->SetDirectory(nullptr);

    TProfile* ptr = prof.get();
    m_profiles[path] = std::move(prof);
    return ptr;
}

auto HistogramService::write_histograms() -> void {
    if (!m_output_svc || !m_output_svc->is_file_open()) {
        Log::error(name(), "OutputFileService not available for writing histograms");
        return;
    }

    size_t total = m_hist1D.size() + m_hist2D.size() + m_hist3D.size() + m_profiles.size();
    if (total == 0) {
        Log::info(name(), "No histograms to write");
        return;
    }

    Log::info(name(), "Writing {} histograms via OutputFileService", total);

    std::lock_guard<std::mutex> lock(m_output_svc->mutex());
    TFile* file = m_output_svc->get_file();
    if (!file) return;

    for (auto& [path, hist] : m_hist1D) {
        create_directories(file, path);
        hist->Write("", TObject::kOverwrite);
    }

    for (auto& [path, hist] : m_hist2D) {
        create_directories(file, path);
        hist->Write("", TObject::kOverwrite);
    }

    for (auto& [path, hist] : m_hist3D) {
        create_directories(file, path);
        hist->Write("", TObject::kOverwrite);
    }

    for (auto& [path, prof] : m_profiles) {
        create_directories(file, path);
        prof->Write("", TObject::kOverwrite);
    }

    Log::info(name(), "Wrote {} 1D, {} 2D, {} 3D histograms, and {} profiles",
              m_hist1D.size(), m_hist2D.size(), m_hist3D.size(), m_profiles.size());
}

auto HistogramService::clone_hist1D_with_suffix(const std::string& base_path,
                                                const std::string& suffix) -> TH1D* {
    auto* base = get_output_hist1D(base_path);
    if (!base) {
        Log::warning(name(), "Cannot clone: histogram '{}' not found", base_path);
        return nullptr;
    }

    std::string new_path = base_path + suffix;
    if (m_hist1D.find(new_path) != m_hist1D.end()) {
        Log::warning(name(), "Histogram '{}' already exists", new_path);
        return m_hist1D[new_path].get();
    }

    std::string new_name = extract_name(new_path);
    auto clone = std::unique_ptr<TH1D>(static_cast<TH1D*>(base->Clone(new_name.c_str())));
    clone->Reset();
    clone->SetDirectory(nullptr);

    TH1D* ptr = clone.get();
    m_hist1D[new_path] = std::move(clone);
    return ptr;
}

auto HistogramService::clone_hist2D_with_suffix(const std::string& base_path,
                                                const std::string& suffix) -> TH2D* {
    auto* base = get_output_hist2D(base_path);
    if (!base) {
        Log::warning(name(), "Cannot clone: histogram '{}' not found", base_path);
        return nullptr;
    }

    std::string new_path = base_path + suffix;
    if (m_hist2D.find(new_path) != m_hist2D.end()) {
        Log::warning(name(), "Histogram '{}' already exists", new_path);
        return m_hist2D[new_path].get();
    }

    std::string new_name = extract_name(new_path);
    auto clone = std::unique_ptr<TH2D>(static_cast<TH2D*>(base->Clone(new_name.c_str())));
    clone->Reset();
    clone->SetDirectory(nullptr);

    TH2D* ptr = clone.get();
    m_hist2D[new_path] = std::move(clone);
    return ptr;
}

auto HistogramService::book_hist1D(const std::string& path, const std::string& title,
                                          int nbins, double xmin, double xmax) -> ThreadLocalHistogram1D* {
    if (m_atomic_hist1D.find(path) != m_atomic_hist1D.end()) {
        Log::warning(name(), "Atomic histogram '{}' already exists", path);
        return m_atomic_hist1D[path].get();
    }

    auto hist = std::make_unique<ThreadLocalHistogram1D>(nbins, xmin, xmax);
    ThreadLocalHistogram1D* ptr = hist.get();
    m_atomic_hist1D[path] = std::move(hist);
    m_atomic_hist1D_meta[path] = {title};
    return ptr;
}

auto HistogramService::book_hist2D(const std::string& path, const std::string& title,
                                          int nbinsx, double xmin, double xmax,
                                          int nbinsy, double ymin, double ymax) -> ThreadLocalHistogram2D* {
    if (m_atomic_hist2D.find(path) != m_atomic_hist2D.end()) {
        Log::warning(name(), "Atomic histogram '{}' already exists", path);
        return m_atomic_hist2D[path].get();
    }

    auto hist = std::make_unique<ThreadLocalHistogram2D>(nbinsx, xmin, xmax, nbinsy, ymin, ymax);
    ThreadLocalHistogram2D* ptr = hist.get();
    m_atomic_hist2D[path] = std::move(hist);
    m_atomic_hist2D_meta[path] = {title};
    return ptr;
}

auto HistogramService::book_hist3D(const std::string& path, const std::string& title,
                                   int nbinsx, double xmin, double xmax,
                                   int nbinsy, double ymin, double ymax,
                                   int nbinsz, double zmin, double zmax) -> ThreadLocalHistogram3D* {
    if (m_atomic_hist3D.find(path) != m_atomic_hist3D.end()) {
        Log::warning(name(), "Atomic histogram '{}' already exists", path);
        return m_atomic_hist3D[path].get();
    }

    auto hist = std::make_unique<ThreadLocalHistogram3D>(nbinsx, xmin, xmax,
                                                         nbinsy, ymin, ymax,
                                                         nbinsz, zmin, zmax);
    ThreadLocalHistogram3D* ptr = hist.get();
    m_atomic_hist3D[path] = std::move(hist);
    m_atomic_hist3D_meta[path] = {title};
    return ptr;
}

auto HistogramService::extract_name(const std::string& path) -> std::string {
    size_t pos = path.find_last_of('/');
    if (pos != std::string::npos) {
        return path.substr(pos + 1);
    }
    return path;
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
    std::string current_dir;
    size_t start = 0;
    while (start < dir_path.length()) {
        size_t end = dir_path.find('/', start);
        if (end == std::string::npos) {
            end = dir_path.length();
        }
        std::string dir_name = dir_path.substr(start, end - start);
        if (!gDirectory->GetDirectory(dir_name.c_str())) {
            gDirectory->mkdir(dir_name.c_str());
        }
        gDirectory->cd(dir_name.c_str());
        start = end + 1;
    }
}

}  // namespace clas12::framework
