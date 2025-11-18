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
#include "hipo4/hipoeventiterator.h"
#include "hipo4/chain.h"


// Project headers
#include <Core/Constants.hpp>
#include <Core/Helpers.hpp>
#include <study1/Drawing.hpp>
#include <study1/Histograms.hpp>
#include <study1/Reader.hpp>

int main() {
    ROOT::EnableThreadSafety(); 

    // Read the files
    std::vector<std::string> files = Core::read_recursive_file_in_directory("/volatile/clas12/ouillon/RGD/rgd_simulation_pion_7/recon/", 10);
    // std::vector<std::string> files = Core::read_recursive_file_in_directory("/cache/clas12/rg-d/production/pass1/recon/LD2/dst/train/SIDIS/", 100);

    // Process the data
    study1::Histograms histograms;
    study1::Reader reader(histograms);

    hipo::chain chain(10, true, false);
    chain.add(files);
    chain.open();

    chain.process([&reader](hipo::iter_event& event, int file_idx, long evt_idx_in_file) {
        reader(event);
    }, 100.0);

    // Draw the histograms
    study1::Drawing drawing(histograms);
    drawing.draw_pion_minus();
    drawing.draw_pion_tracks();

    return EXIT_SUCCESS;
}
