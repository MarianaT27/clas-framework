#pragma once

// std includes
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

// <fmt> headers
#include <fmt/core.h>
#include <fmt/ranges.h>

// ROOT includes
#include <TCanvas.h>
#include <TColor.h>
#include <TF1.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <THStack.h>
#include <TLegend.h>
#include <TLine.h>
#include <TMath.h>
#include <TPaveStats.h>
#include <TStyle.h>
#include <ROOT/TThreadedObject.hxx>

// Project headers
#include <Plotting/Colors.hpp>
#include "fmt/base.h"

namespace Plotting {

using TTO_TH1D = ROOT::TThreadedObject<TH1D>;
using TTO_TH2D = ROOT::TThreadedObject<TH2D>;
using up_TTO_TH1D = std::unique_ptr<TTO_TH1D>;
using up_TTO_TH2D = std::unique_ptr<TTO_TH2D>;

/**
 * @brief Saves a ROOT object into an open ROOT file.
 *
 * This function writes the provided object to the specified ROOT file.
 * The file must be open and valid; otherwise, an exception is thrown.
 *
 * @tparam T Type of the ROOT object to be saved (e.g., TH1, TTree).
 * @param file A unique pointer to an open TFile where the object will be saved.
 * @param object Reference to the ROOT object to be written into the file.
 *
 * @throws std::invalid_argument If the file is not open or is invalid.
 */
template <typename T>
inline auto save_in_root_file(const std::unique_ptr<TFile>& file, T& object) -> void {
    if (!file || !file->IsOpen()) throw std::invalid_argument("File is not open or is invalid");
    object->Write();
}

/**
 * @brief Options for drawing 1D histograms with styling controls.
 */
struct OptionTH1 {
    // Pad options
    bool log_x = false;
    bool log_y = false;

    // Histogram style options
    int16_t color1 = Color::kBlack;
    float alpha_color1 = 0.0;
    std::string draw_option = "histo";
    std::string opt_stat = "emr";

    // Scaling and range options
    std::pair<double, double> x_range = {0.0, 0.0};  // {0,0} means auto-range
    std::pair<double, double> y_range = {0.0, 0.0};  // {0,0} means auto-range
    double scale1 = 0.0;                             // 0 means no scaling

    // Legend and labeling
    std::string legend1 = "";
    std::string legend2 = "";
    std::string title = "";

    // Axes styling
    std::string x_label = "";
    float x_label_offset = 0.8f;
    float x_label_size = 0.05f;
    std::string y_label = "";
    float y_label_offset = 0.8f;
    float y_label_size = 0.05f;
};

/**
 * @brief Draws a 1D histogram with full styling and feature implementation.
 *
 * This function merges the input histogram, applies scaling, sets ranges,
 * draws cut lines, adds legends, and saves the result to a ROOT file.
 *
 * @param h Unique pointer to the histogram object to be merged and drawn.
 * @param file Unique pointer to the ROOT file where the histogram will be saved.
 * @param args Optional drawing and styling parameters (see OptionTH1).
 *
 * @throws std::invalid_argument if the merged histogram pointer is null.
 */
inline auto draw_hist1D(const up_TTO_TH1D& h, const std::unique_ptr<TFile>& file, const OptionTH1& args = {}) -> void {
    const std::shared_ptr<TH1D> h_merged = h->Merge();
    if (!h_merged) {
        fmt::println("Histogram pointer is null, cannot draw {}", h->Get()->GetName());
        return;
    }
    gStyle->SetOptStat(args.opt_stat.data());  // Set statistics display options

    if (args.scale1 > 0.0) h_merged->Scale(args.scale1);  // Apply scaling if specified
    h_merged->SetLineColor(args.color1);                  // Set histogram style
    h_merged->SetFillColorAlpha(args.color1, args.alpha_color1);
    if (!args.title.empty()) h_merged->SetTitle(args.title.data());  // Set title

    // Configure X-axis
    h_merged->GetXaxis()->SetTitle(args.x_label.data());
    h_merged->GetXaxis()->SetTitleOffset(args.x_label_offset);
    h_merged->GetXaxis()->SetTitleSize(args.x_label_size);

    // Set X-axis range if specified
    if (args.x_range.first != 0.0 || args.x_range.second != 0.0) {
        h_merged->GetXaxis()->SetRangeUser(args.x_range.first, args.x_range.second);
    }

    // Configure Y-axis
    if (!args.y_label.empty()) {
        h_merged->GetYaxis()->SetTitle(args.y_label.data());
        h_merged->GetYaxis()->SetTitleOffset(args.y_label_offset);
        h_merged->GetYaxis()->SetTitleSize(args.y_label_size);
    }

    if (args.y_range.first != 0.0 || args.y_range.second != 0.0) h_merged->GetYaxis()->SetRangeUser(args.y_range.first, args.y_range.second);  // Set Y-axis range if specified
    h_merged->Draw(args.draw_option.data());                                                                                                   // Draw histogram

    if (args.log_x) gPad->SetLogx();  // Set logarithmic scales if requested
    if (args.log_y) gPad->SetLogy();

    save_in_root_file(file, h_merged);
}

/**
 * @brief Options for drawing 2D histograms with styling controls.
 */
struct OptionTH2 {
    bool log_x = false;
    bool log_y = false;
    bool log_z = false;

    std::string x_label = "";
    float x_label_offset = 0.8f;
    float x_label_size = 0.05f;

    std::string y_label = "";
    float y_label_offset = 1.0f;
    float y_label_size = 0.05f;

    std::string z_label = "";
    float z_label_offset = 1.0f;
    float z_label_size = 0.05f;

    std::string title = "";
    std::string draw_option = "COLZ";
};

/**
 * @brief Draws a 2D histogram using ROOT and saves it to a ROOT file.
 *
 * This function merges the provided histogram, draws it with the specified options,
 * sets axis labels, applies logarithmic scales if requested, and saves the result.
 *
 * @param h Unique pointer to a TTO_TH2D histogram object to be merged and drawn.
 * @param file Unique pointer to an open ROOT TFile where the histogram will be saved.
 * @param args Optional drawing and axis formatting options.
 *
 * @throws std::invalid_argument if the merged histogram pointer is null.
 */
inline auto draw_hist2D(const up_TTO_TH2D& h, const std::unique_ptr<TFile>& file, const OptionTH2& args = {}) -> void {
    const std::shared_ptr<TH2D> h_merged = h->Merge();
    if (!h_merged) {
        fmt::println("Histogram pointer is null, cannot draw {}", h->Get()->GetName());
        return;
    }

    if (!args.title.empty()) h_merged->SetTitle(args.title.data());  // Set title
    h_merged->Draw(args.draw_option.data());                         // Draw histogram

    if (!args.x_label.empty()) h_merged->GetXaxis()->SetTitle(args.x_label.data());  // Configure X-axis
    h_merged->GetXaxis()->SetTitleOffset(args.x_label_offset);
    h_merged->GetXaxis()->SetTitleSize(args.x_label_size);

    if (!args.y_label.empty()) h_merged->GetYaxis()->SetTitle(args.y_label.data());  // Configure Y-axis
    h_merged->GetYaxis()->SetTitleOffset(args.y_label_offset);
    h_merged->GetYaxis()->SetTitleSize(args.y_label_size);

    if (!args.z_label.empty()) h_merged->GetZaxis()->SetTitle(args.z_label.data());  // Configure Z-axis
    h_merged->GetZaxis()->SetTitleOffset(args.z_label_offset);
    h_merged->GetZaxis()->SetTitleSize(args.z_label_size);

    // Set logarithmic scales if requested
    if (args.log_x) gPad->SetLogx();
    if (args.log_y) gPad->SetLogy();
    if (args.log_z) gPad->SetLogz();

    save_in_root_file(file, h_merged);
}

}  // namespace Plotting