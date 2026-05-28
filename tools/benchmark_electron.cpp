// tools/benchmark_electron.cpp
//
// Standalone raw-chain throughput benchmark for the CLAS12 electron
// selection. Uses ONLY hipo::chain (the banklist fast path, ByPage v2 +
// mmap) plus the exact cut logic of ElectronSelectionAlg — no framework
// EDM, no histograms, no ntuples. Its events/sec is the ceiling that the
// full framework is compared against (see tools/compare_electron_benchmark.sh).
//
// The cut logic mirrors, line for line:
//   algorithms/.../selection/ElectronSelectionAlg.hpp   (cut order + SF formula)
//   clas12/.../services/ElectronCutsService.hpp         (thresholds)
//   clas12/.../services/SamplingFractionService.hpp     (OB/IB poly coeffs)
//   clas12/.../bankdefs/generated_readers.hpp           (calo/traj aggregation)
//
// Build: tools/meson.build target 'benchmark_electron'
//        deps [hipo4_dep, fmt_dep, cxxopts_dep].

#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <string>
#include <vector>

#include <cxxopts.hpp>
#include <fmt/format.h>

#include <chain.h>   // hipo::chain, hipo::banklist, hipo::getBanklistIndex
#include <bank.h>

// ----------------------------------------------------------------------------
// Detector constants (mirror clas12/Detectors.hpp; copied so this TU has no
// framework include dependency).
// ----------------------------------------------------------------------------
namespace det {
constexpr int FD = 2000, CD = 4000;          // status windows
constexpr int ECAL = 7, DC = 6;              // detector ids
constexpr int PCAL = 1, ECIN = 4, ECOUT = 7; // calorimeter layers
constexpr int DC1 = 6, DC3 = 18, DC6 = 36;   // DC region layers
constexpr int ELECTRON_PID = 11;
}  // namespace det

// ----------------------------------------------------------------------------
// Cut constants (mirror ElectronCutsService / SamplingFractionService).
// ----------------------------------------------------------------------------
namespace cut {
constexpr double MIN_CHI2PID = -5.0, MAX_CHI2PID = 5.0;
constexpr double MIN_P       = 2.0;   // ElectronCutsService::MIN_ELECTRON_MOMENTUM
                                      // (framework cutflow label says 0.8 — stale)
constexpr double N_SIGMA     = 3.5;
constexpr double PCAL_FID    = 9.0;   // PCALFiducialLevel::Loose
constexpr double EDGE_R1 = 1.680, EDGE_R2 = 2.0, EDGE_R3 = 8.750;

// RG-D fixed vz windows by target (ElectronCutsService::get_vz_range).
struct VzRange { double lo, hi; };
inline VzRange vz_range_for(const std::string& target) {
    if (target == "LD2")  return {-15.0, 5.0};
    if (target == "CxC")  return {-10.6, 5.0};
    if (target == "Cu")   return {-10.6, -6.5};
    if (target == "Sn")   return {-5.5, 5.0};
    if (target == "CuSn") return {-10.6, 5.0};
    if (target == "Custom") return {-12.0, 5.0};
    return {-30.0, 30.0};  // fallback
}

// Sampling-fraction poly3 coefficients, verbatim from
// SamplingFractionService.hpp m_data["OB"] / m_data["IB"].
// Layout per sector: {a, b, c, d};  value(p) = a + b*p + c*p^2 + d*p^3.
constexpr double OB_MU[6][4] = {
    {2.300896E-01, 4.862074E-03, -6.441645E-04,  1.247456E-05},
    {2.286452E-01, 4.985634E-03, -3.668113E-04, -1.110160E-05},
    {2.258053E-01, 9.073840E-03, -1.240745E-03,  3.579321E-05},
    {2.281016E-01, 6.073295E-03, -5.916479E-04,  2.569914E-06},
    {2.302059E-01, 3.020237E-03, -4.307581E-05, -2.216133E-05},
    {2.262583E-01, 6.849767E-03, -8.027577E-04,  1.288497E-05},
};
constexpr double OB_SIGMA[6][4] = {
    {2.226749E-02, -3.186742E-03, 3.087387E-04, -1.247691E-05},
    {2.150934E-02, -2.549354E-03, 1.855833E-04, -4.394923E-06},
    {2.399538E-02, -4.208668E-03, 4.503779E-04, -1.783519E-05},
    {2.217527E-02, -2.788589E-03, 2.266746E-04, -7.717066E-06},
    {2.277357E-02, -3.941980E-03, 4.455085E-04, -1.912118E-05},
    {2.253050E-02, -3.334553E-03, 3.374127E-04, -1.413483E-05},
};
constexpr double IB_MU[6][4] = {
    {2.311088E-01, 4.309933E-03, -5.451627E-04,  4.965823E-06},
    {2.304337E-01, 3.585716E-03, -1.672223E-04, -7.317175E-06},
    {2.269123E-01, 8.797582E-03, -1.171416E-03,  3.051342E-05},
    {2.288870E-01, 6.786520E-03, -8.022602E-04,  1.557251E-05},
    {2.288645E-01, 4.637362E-03, -3.662892E-04, -4.233509E-06},
    {2.272093E-01, 6.581981E-03, -8.197177E-04,  1.632178E-05},
};
constexpr double IB_SIGMA[6][4] = {
    {2.235239E-02, -3.117249E-03, 2.887205E-04, -1.130317E-05},
    {2.170118E-02, -2.428037E-03, 1.302147E-04, -2.503248E-09},
    {2.423467E-02, -4.301928E-03, 4.742880E-04, -1.983022E-05},
    {2.313626E-02, -3.260271E-03, 3.018634E-04, -1.156770E-05},
    {2.310895E-02, -3.892876E-03, 4.115173E-04, -1.656769E-05},
    {2.321748E-02, -3.665685E-03, 3.804588E-04, -1.587616E-05},
};

inline double poly3(const double c[4], double p) {
    return c[0] + c[1] * p + c[2] * p * p + c[3] * p * p * p;
}
}  // namespace cut

// ----------------------------------------------------------------------------
// Cutflow: struct of relaxed atomics, shared by reference across the chain's
// worker threads. All worker futures are joined before we read it, so the
// post-run read is a clean synchronization point.
// ----------------------------------------------------------------------------
enum Stage {
    kAll = 0, kHasElectron, kChi2pid, kMomentum, kVz,
    kSF, kPcalFid, kDcR1, kDcR2, kDcR3, kNStages
};
static constexpr const char* kStageNames[kNStages] = {
    "All Events", "Has Electron", "Chi2 PID", "Momentum > 2.0 GeV", "Vertex Z",
    "Sampling Fraction", "PCAL Fiducial", "DC R1 Edge", "DC R2 Edge", "DC R3 Edge"
};
struct Cutflow {
    std::atomic<long> n[kNStages];
    Cutflow() { for (auto& a : n) a.store(0, std::memory_order_relaxed); }
    inline void pass(Stage s) { n[s].fetch_add(1, std::memory_order_relaxed); }
};

int main(int argc, char** argv) {
    cxxopts::Options opts("benchmark_electron",
        "Raw hipo::chain electron-selection throughput benchmark (no framework)");
    opts.add_options()
        ("i,input",      "Input HIPO file(s)",
            cxxopts::value<std::vector<std::string>>())
        ("t,threads",    "Worker threads (0 = hardware concurrency)",
            cxxopts::value<int>()->default_value("0"))
        ("p,percentage", "Percentage of events to process",
            cxxopts::value<double>()->default_value("100.0"))
        ("target",       "Target (LD2/CxC/Cu/Sn/CuSn/Custom) — selects vz window",
            cxxopts::value<std::string>()->default_value("LD2"))
        ("polarity",     "Polarity OB (Outbending) or IB (Inbending) — selects SF coeffs",
            cxxopts::value<std::string>()->default_value("OB"))
        ("progress",     "Show the chain progress bar (off for clean timing)",
            cxxopts::value<bool>()->default_value("false"))
        ("h,help",       "Print help");
    opts.parse_positional({"input"});
    opts.positional_help("<file.hipo> [more.hipo ...]");

    auto args = opts.parse(argc, argv);
    if (args.count("help") || !args.count("input")) {
        fmt::print("{}\n", opts.help());
        return args.count("help") ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    const auto   files      = args["input"].as<std::vector<std::string>>();
    const int    threads    = args["threads"].as<int>();
    const double percentage = args["percentage"].as<double>();
    const auto   target     = args["target"].as<std::string>();
    auto         polarity   = args["polarity"].as<std::string>();
    const bool   progress   = args["progress"].as<bool>();

    const bool inbending = (polarity == "IB" || polarity == "Inbending");
    polarity = inbending ? "IB" : "OB";
    const auto vzr = cut::vz_range_for(target);
    const auto (&SF_MU)[6][4]    = inbending ? cut::IB_MU    : cut::OB_MU;
    const auto (&SF_SIGMA)[6][4] = inbending ? cut::IB_SIGMA : cut::OB_SIGMA;

    // --- chain setup (banklist fast path) ---
    hipo::chain chain(threads, /*progress=*/progress, /*verbose=*/false);
    for (const auto& f : files) chain.add(f);
    chain.set_progress(progress);

    hipo::banklist banks =
        chain.getBanks({"REC::Particle", "REC::Calorimeter", "REC::Traj"});
    const auto bPart = hipo::getBanklistIndex(banks, "REC::Particle");
    const auto bCalo = hipo::getBanklistIndex(banks, "REC::Calorimeter");
    const auto bTraj = hipo::getBanklistIndex(banks, "REC::Traj");

    // Resolve column item indices ONCE so the hot loop never does a name
    // lookup (mirrors how the framework's load_particles caches item ids).
    auto col = [](hipo::bank& b, const char* name) {
        return b.getSchema().getEntryOrder(name);
    };
    const int cP_pid = col(banks[bPart], "pid"), cP_status = col(banks[bPart], "status");
    const int cP_px = col(banks[bPart], "px"), cP_py = col(banks[bPart], "py"),
              cP_pz = col(banks[bPart], "pz");
    const int cP_vz = col(banks[bPart], "vz"), cP_chi2 = col(banks[bPart], "chi2pid");

    const int cC_pindex = col(banks[bCalo], "pindex"), cC_det = col(banks[bCalo], "detector");
    const int cC_sector = col(banks[bCalo], "sector"), cC_layer = col(banks[bCalo], "layer");
    const int cC_energy = col(banks[bCalo], "energy"), cC_lv = col(banks[bCalo], "lv"),
              cC_lw = col(banks[bCalo], "lw");

    const int cT_pindex = col(banks[bTraj], "pindex"), cT_det = col(banks[bTraj], "detector");
    const int cT_layer = col(banks[bTraj], "layer"), cT_edge = col(banks[bTraj], "edge");

    Cutflow cf;
    const double NaN = std::nan("");

    // --- per-event selection (runs on each worker; bl is the thread-local
    //     banklist copy supplied by chain.process). ---
    auto process = [&](hipo::banklist& bl, int /*file_idx*/, long /*evt_idx*/) {
        cf.pass(kAll);

        const hipo::bank& P = bl[bPart];
        const hipo::bank& C = bl[bCalo];
        const hipo::bank& T = bl[bTraj];

        // (1) trigger electron: first REC::Particle row (ascending) with
        //     pid==11 && status<0 && p!=0 && 2000<=|status|<3000.
        const int nP = P.getRows();
        int eRow = -1;
        double e_p = 0, e_vz = 0, e_chi2 = 0;
        for (int i = 0; i < nP; ++i) {
            if (P.getInt(cP_pid, i) != det::ELECTRON_PID) continue;
            const int st = P.getShort(cP_status, i);
            if (st >= 0) continue;
            const int ast = std::abs(st);
            if (ast < det::FD || ast >= det::CD) continue;
            const double px = P.getFloat(cP_px, i);
            const double py = P.getFloat(cP_py, i);
            const double pz = P.getFloat(cP_pz, i);
            const double p  = std::sqrt(px * px + py * py + pz * pz);
            if (p == 0.0) continue;
            eRow = i; e_p = p;
            e_vz = P.getFloat(cP_vz, i);
            e_chi2 = P.getFloat(cP_chi2, i);
            break;  // first match wins
        }
        if (eRow < 0) return;
        cf.pass(kHasElectron);

        // (2) calorimeter aggregation: pindex==eRow, detector==ECAL, by layer.
        //     "Last row wins" per layer (do NOT break early). Energies start
        //     NaN so a missing layer -> SF NaN -> SF cut fails (matches fw).
        double pcal_e = NaN, in_e = NaN, out_e = NaN, pcal_lv = NaN, pcal_lw = NaN;
        int pcal_sector = 0;
        const int nC = C.getRows();
        for (int i = 0; i < nC; ++i) {
            if (C.getShort(cC_pindex, i) != eRow) continue;
            if (C.getByte(cC_det, i) != det::ECAL) continue;
            const int layer = C.getByte(cC_layer, i);
            const double en = C.getFloat(cC_energy, i);
            switch (layer) {
                case det::PCAL:
                    pcal_e = en;
                    pcal_sector = C.getByte(cC_sector, i);
                    pcal_lv = C.getFloat(cC_lv, i);
                    pcal_lw = C.getFloat(cC_lw, i);
                    break;
                case det::ECIN:  in_e  = en; break;
                case det::ECOUT: out_e = en; break;
                default: break;
            }
        }

        // (3) traj aggregation: pindex==eRow, detector==DC, by region layer.
        double r1 = NaN, r2 = NaN, r3 = NaN;
        const int nT = T.getRows();
        for (int i = 0; i < nT; ++i) {
            if (T.getShort(cT_pindex, i) != eRow) continue;
            if (T.getByte(cT_det, i) != det::DC) continue;
            const int layer = T.getByte(cT_layer, i);
            const double edge = T.getFloat(cT_edge, i);
            switch (layer) {
                case det::DC1: r1 = edge; break;
                case det::DC3: r2 = edge; break;
                case det::DC6: r3 = edge; break;
                default: break;
            }
        }

        // (4) sampling fraction (NaN-propagating).
        const double SF = (pcal_e + in_e + out_e) / e_p;

        // (5) cuts in order, each a cutflow stage.
        if (!(cut::MIN_CHI2PID < e_chi2 && e_chi2 < cut::MAX_CHI2PID)) return;
        cf.pass(kChi2pid);

        if (!(e_p > cut::MIN_P)) return;
        cf.pass(kMomentum);

        if (!(e_vz > vzr.lo && e_vz < vzr.hi)) return;
        cf.pass(kVz);

        bool sf_ok = false;
        if (pcal_sector >= 1 && pcal_sector <= 6) {
            const double mu  = cut::poly3(SF_MU[pcal_sector - 1], e_p);
            const double sig = cut::poly3(SF_SIGMA[pcal_sector - 1], e_p);
            sf_ok = (mu - cut::N_SIGMA * sig < SF) && (SF < mu + cut::N_SIGMA * sig);
        }
        if (!sf_ok) return;  // also false when SF is NaN
        cf.pass(kSF);

        if (!(pcal_lv > cut::PCAL_FID && pcal_lw > cut::PCAL_FID)) return;  // NaN -> false
        cf.pass(kPcalFid);

        // DC edge: single combined decision, three lockstep cutflow stages.
        if (!(r1 > cut::EDGE_R1 && r2 > cut::EDGE_R2 && r3 > cut::EDGE_R3)) return;
        cf.pass(kDcR1);
        cf.pass(kDcR2);
        cf.pass(kDcR3);
    };

    // --- timed run ---
    const auto t0 = std::chrono::steady_clock::now();
    chain.process(banks, process, percentage);
    const auto t1 = std::chrono::steady_clock::now();
    const double secs = std::chrono::duration<double>(t1 - t0).count();

    // --- report ---
    const long  events  = chain.events_processed();
    const long  records = chain.records_processed();
    const double evps = secs > 0 ? events  / secs : 0.0;
    const double rcps = secs > 0 ? records / secs : 0.0;

    fmt::print("\n================ benchmark_electron (chain-only) ================\n");
    fmt::print("  files             : {}\n", files.size());
    fmt::print("  threads           : {}\n", threads == 0 ? -1 : threads);
    fmt::print("  target / polarity : {} / {}  (vz window [{:.2f}, {:.2f}])\n",
               target, polarity, vzr.lo, vzr.hi);
    fmt::print("  percentage        : {:.1f}\n", percentage);
    fmt::print("  wall time         : {:.3f} s\n", secs);
    fmt::print("  events processed  : {}\n", events);
    fmt::print("  records processed : {}\n", records);
    fmt::print("  throughput        : {:.1f} events/s  ({:.1f} records/s)\n", evps, rcps);
    fmt::print("-----------------------------------------------------------------\n");
    fmt::print("  {:<22} {:>12} {:>9} {:>10}\n", "stage", "passed", "abs%", "step%");
    const long base = cf.n[kAll].load();
    long prev = base;
    for (int s = 0; s < kNStages; ++s) {
        const long c = cf.n[s].load();
        const double abs_pct  = base ? 100.0 * static_cast<double>(c) / base : 0.0;
        const double step_pct = prev ? 100.0 * static_cast<double>(c) / prev : 0.0;
        fmt::print("  {:<22} {:>12} {:>8.2f}% {:>9.2f}%\n",
                   kStageNames[s], c, abs_pct, step_pct);
        prev = c;
    }
    fmt::print("=================================================================\n");
    return EXIT_SUCCESS;
}
