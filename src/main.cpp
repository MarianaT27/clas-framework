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

#include <toml++/toml.hpp>

// ROOT headers
#include <TCanvas.h>
#include <TH1D.h>
#include <TLine.h>
#include <TROOT.h>
#include <TStyle.h>

// Project headers
#include <Core/Constants.hpp>
#include <Core/Helpers.hpp>
#include <study1/Drawing.hpp>
#include <study1/Histograms.hpp>
#include <study1/Reader.hpp>
#include <thread_pool/multi_thread.hpp>

int main() {
    ROOT::EnableThreadSafety(); 

    // Read the files
    // std::vector<std::string> files = Core::read_recursive_file_in_directory("/volatile/clas12/ouillon/RGD/rgd_simulation_pion_7/recon/", 100);
    std::vector<std::string> files = Core::read_recursive_file_in_directory("/cache/clas12/rg-d/production/pass1/recon/LD2/dst/train/SIDIS/", 100);

    // Process the data
    study1::Histograms histograms;
    study1::Reader reader(histograms);

    multithread_reader(reader, files, 10);

    // Draw the histograms
    study1::Drawing drawing(histograms);
    drawing.draw_pion_minus();

    return EXIT_SUCCESS;
}
