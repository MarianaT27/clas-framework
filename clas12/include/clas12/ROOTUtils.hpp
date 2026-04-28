#pragma once

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

#include <TError.h>
#include <TH1.h>
#include <TROOT.h>
#include <TStyle.h>


/**
 * @brief ROOT-specific utility functions
 */
namespace ROOTUtils {

/**
 * @brief Configures ROOT global options for analysis.
 *
 * Sets up thread safety, implicit MT, histogram defaults, and style options.
 */
inline auto configure_ROOT() -> void {
    ROOT::EnableThreadSafety();
    ROOT::EnableImplicitMT();
    TH1::SetDefaultSumw2(kTRUE);

    gErrorIgnoreLevel = kPrint;

    gStyle->SetOptFit(1111);
    gStyle->SetNumberContours(255);
    gStyle->SetImageScaling(3.);

    gStyle->SetLineWidth(1);
    gStyle->SetFrameLineWidth(1);
    gStyle->SetHistLineWidth(1);
    gStyle->SetFuncWidth(1);
    gStyle->SetGridWidth(1);

    gStyle->SetPadTickX(1);
    gStyle->SetPadTickY(1);
    gStyle->SetNdivisions(505, "x");
    gStyle->SetNdivisions(510, "y");

    gStyle->SetTextFont(42);
}

/**
 * @brief Finds optimal bin edges for equal-count binning.
 *
 * @param values Vector of values to bin
 * @param numBins Number of bins (default 6)
 */
inline auto find_equal_count_bins(const std::vector<double>& values, int numBins = 6) -> void {
    if (values.empty() || numBins <= 0) {
        std::cerr << "Error: Invalid input. Values must not be empty, and numBins must be positive." << std::endl;
        return;
    }

    int counts_per_bin = static_cast<int>(values.size()) / numBins;
    if (counts_per_bin == 0) {
        std::cerr << "Error: Number of bins exceeds the number of values." << std::endl;
        return;
    }

    auto sorted_values = values;
    std::sort(sorted_values.begin(), sorted_values.end());

    std::vector<double> bin_edges;
    bin_edges.reserve(numBins + 1);
    bin_edges.push_back(sorted_values.front());

    for (int edge = 1; edge < numBins; ++edge) {
        bin_edges.push_back(sorted_values.at(counts_per_bin * edge));
    }
    bin_edges.push_back(sorted_values.back());

    for (auto& elm : bin_edges) {
        elm = std::ceil(elm * 100.0) / 100.0;
    }

    std::cout << "bin_edges = [";
    for (size_t i = 0; i < bin_edges.size(); ++i) {
        std::cout << bin_edges[i];
        if (i != bin_edges.size() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
}

}  // namespace ROOTUtils

