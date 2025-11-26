#include "Framework/Histogram.hpp"
#include <algorithm>
#include <cctype>
#include "Framework/Services/HistogramService.hpp"

namespace Framework {

// HistogramGroup implementation

auto HistogramGroup::add_hist1D(const std::string& name, const std::string& title, int nbins, double xmin, double xmax) -> Hist1D& {
    std::string path = full_path(name);
    TH1D* hist = m_service->book_hist1D(path, title, nbins, xmin, xmax);
    m_hist1d[name] = Hist1D(hist);
    return m_hist1d[name];
}

auto HistogramGroup::add_hist2D(const std::string& name, const std::string& title, int nbinsx, double xmin, double xmax, int nbinsy, double ymin, double ymax) -> Hist2D& {
    std::string path = full_path(name);
    TH2D* hist = m_service->book_hist2D(path, title, nbinsx, xmin, xmax, nbinsy, ymin, ymax);
    m_hist2d[name] = Hist2D(hist);
    return m_hist2d[name];
}

auto HistogramGroup::add_profile(const std::string& name, const std::string& title, int nbins, double xmin, double xmax) -> Profile& {
    std::string path = full_path(name);
    TProfile* prof = m_service->book_profile(path, title, nbins, xmin, xmax);
    m_profiles[name] = Profile(prof);
    return m_profiles[name];
}

// CutflowHistogram implementation

auto CutflowHistogram::sanitize_name(const std::string& name) -> std::string {
    std::string result = name;
    std::replace_if(result.begin(), result.end(), [](char c) { return std::isspace(c) || c == '>' || c == '<' || c == '/'; }, '_');
    return result;
}

auto CutflowHistogram::register_cut(const std::string& cut_name) -> void {
    std::string sanitized = sanitize_name(cut_name);
    std::string path = m_base_path + "_" + sanitized;
    std::string title = m_title + " (after " + cut_name + ")";

    TH1D* hist = m_service->book_hist1D(path, title, m_nbins, m_xmin, m_xmax);
    m_histograms[cut_name] = Hist1D(hist);
}

auto CutflowHistogram::fill(const std::string& cut_name, double value) -> void {
    auto it = m_histograms.find(cut_name);
    if (it != m_histograms.end()) it->second.fill(value);
}

auto CutflowHistogram::fill(const std::string& cut_name, double value, double weight) -> void {
    auto it = m_histograms.find(cut_name);
    if (it != m_histograms.end()) it->second.fill(value, weight);
}

auto CutflowHistogram::get_histogram(const std::string& cut_name) -> std::optional<Hist1D> {
    auto it = m_histograms.find(cut_name);
    if (it != m_histograms.end()) return it->second;
    return std::nullopt;
}

}  // namespace Framework
