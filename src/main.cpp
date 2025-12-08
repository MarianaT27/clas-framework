// C++ headers
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <numeric>
#include <ranges>
#include <vector>

// fmt headers
#include <fmt/core.h>
#include <fmt/ranges.h>

// ROOT headers
#include <TCanvas.h>
#include <TH1D.h>
#include <TLine.h>
#include <TROOT.h>
#include <TStyle.h>

// Hipo headers
#include "CLAS/Target.hpp"
#include "fmt/base.h"
#include "hipo4/hipoeventiterator.h"
#include "hipo4/chain.h"


// Project headers
#include <CLAS/Constants.hpp>
#include <CLAS/FileUtils.hpp>
#include <study1/Drawing.hpp>
#include <study1/Histograms.hpp>
#include <study1/Reader.hpp>

int main() {
    ROOT::EnableThreadSafety();

    // Read the files
    std::vector<std::string> files = CLAS::FileUtils::read_recursive_hipo_files("/cache/clas12/rg-d/production/pass1/recon/LD2/dst/train/SIDIS/");

    // Process the data
    CLAS::Target target = CLAS::Target::LD2;
    CLAS::Polarity polarity = CLAS::Polarity::Outbending;
    study1::Histograms histograms;
    study1::Reader reader(histograms, target, polarity);

    hipo::chain chain(10, true, false);
    chain.add(files);
    chain.open();
    chain.process([&reader](hipo::iter_event& event, int file_idx, long evt_idx_in_file) {
        reader(event);
    }, 1);

    // Draw the histograms
    study1::Drawing drawing(histograms);
    drawing.draw_electron_kinematics();

    // Print summary
    fmt::print("===== Summary =====\n");
    fmt::println("Target: {} | Polarity: {}", CLAS::to_string(target), CLAS::to_string(polarity));
    fmt::print("Total number of events processed: {}\n", reader.get_event_count());
    fmt::print("Number of trigger electrons found: {}\n", reader.get_nb_trigger_electron());
    fmt::print("Number of survivors after momentum cut: {}\n", reader.get_mom_cut_survivors());
    fmt::print("Number of survivors after sampling fraction cut: {}\n", reader.get_sf_cut_survivors());
    fmt::print("Number of survivors after PCAL cut: {}\n", reader.get_pcal_cut_survivors());
    fmt::print("Number of survivors after DC cut: {}\n", reader.get_dc_cut_survivors());
    fmt::print("Number of survivors after Vz cut: {}\n", reader.get_vz_cut_survivors());

    return EXIT_SUCCESS;
}
