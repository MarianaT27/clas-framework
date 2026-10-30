// electronCutCheck.C
// Independent check of the RG-D electron selection (common note, Table 4 / Table 5).
// Reads one recon HIPO file directly with hipo4 and prints the cutflow.
// No ROOT file is written.
//
// Run from the clas-framework folder (so the Vz parameter file is found):
//   clas12root -b -q 'electronCutCheck.C("<recon file>","LD2")'
//   clas12root -b -q 'electronCutCheck.C("<recon file>","CxC")'
//   clas12root -b -q 'electronCutCheck.C("<recon file>","CuSn")'
//
// Vz cut:
//   LD2       : raw Vz in [-15, 5] cm (note Sec. 3.1.2.2)
//   CxC, CuSn : per-event corrected Vz (note Sec. 3.1.2.1, Eq. 6), then inside either
//               foil window mu +/- 3 sigma_eff (note Table 2, OB)

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cmath>
#include <cctype>
#include <chrono>
#include "hipo4/reader.h"

using namespace std;

// ---------------------------------------------------------------------------
// Cut values (common note)
// ---------------------------------------------------------------------------
const double P_MIN     = 1.8;    // GeV, Table 4 ("to-be-verified")
const double N_SIGMA   = 3.5;    // SF mu +/- 3.5 sigma vs p, Table 4
const double PCAL_MIN  = 9.0;    // cm, lv > 9 and lw > 9, Sec. 3.2.1
const double VZ_LD2_LO = -15.0;  // cm, LD2 window, Sec. 3.1.2.2
const double VZ_LD2_HI = 5.0;

// DC edge thresholds per region [cm]
// Note, Table 3 (average over sectors, OB):
const double DC_NOTE[3] = {1.88, 2.08, 8.62};
// Values hard-coded in clas-framework / rgd-pi0 (ElectronCutsService.hpp), for comparison only:
const double DC_CODE[3] = {1.68, 2.00, 8.75};

// Vz foil windows in the corrected frame, note Table 2 (OB): {mu_upstream, sig_upstream, mu_downstream, sig_downstream}
const double VZWIN_CUSN[4] = {-7.861, 0.415, -2.916, 0.370};  // Cu, Sn
const double VZWIN_CXC[4]  = {-7.887, 0.395, -2.906, 0.373};  // C1, C2
const double VZWIN_NSIG    = 3.0;

// SF parameterization, outbending, per sector: mu = a + b p + c p^2 + d p^3 (same for sigma).
// The note shows these only as curves (Fig. 3.7, App. C); the numbers are taken from
// clas-framework SamplingFractionService.hpp ("Target-averaged Outbending parameters").
const double SF_MU[6][4] = {
    {2.300896E-01, 4.862074E-03, -6.441645E-04,  1.247456E-05},
    {2.286452E-01, 4.985634E-03, -3.668113E-04, -1.110160E-05},
    {2.258053E-01, 9.073840E-03, -1.240745E-03,  3.579321E-05},
    {2.281016E-01, 6.073295E-03, -5.916479E-04,  2.569914E-06},
    {2.302059E-01, 3.020237E-03, -4.307581E-05, -2.216133E-05},
    {2.262583E-01, 6.849767E-03, -8.027577E-04,  1.288497E-05}};
const double SF_SIG[6][4] = {
    {2.226749E-02, -3.186742E-03, 3.087387E-04, -1.247691E-05},
    {2.150934E-02, -2.549354E-03, 1.855833E-04, -4.394923E-06},
    {2.399538E-02, -4.208668E-03, 4.503779E-04, -1.783519E-05},
    {2.217527E-02, -2.788589E-03, 2.266746E-04, -7.717066E-06},
    {2.277357E-02, -3.941980E-03, 4.455085E-04, -1.912118E-05},
    {2.253050E-02, -3.334553E-03, 3.374127E-04, -1.413483E-05}};

double poly3(const double c[4], double x) { return c[0] + c[1] * x + c[2] * x * x + c[3] * x * x * x; }

// ---------------------------------------------------------------------------
// Vz correction (note Sec. 3.1.2.1)
// The note gives the method (Eq. 6) but not the fitted numbers, so the per-cell
// parameters are read from clas-framework data/Vz/vz_corrector_params.txt.
// Cells: 6 sectors x n_p momentum bins x 6 local-phi bins. In each cell, the two
// foil peaks have centroid mu(theta) and width sigma(theta) as cubics in theta (deg).
// In the file, "lo" is the downstream foil (Sn / C2, about -2.9 cm) and
// "hi" is the upstream foil (Cu / C1, about -7.9 cm).
// ---------------------------------------------------------------------------
const int NSEC = 6, NPHI = 6, NPMAX = 20;
double vzMu0Lo, vzSig0Lo, vzMu0Hi, vzSig0Hi;   // reference frame (peak positions after correction)
double vzPLo, vzPBinW;
int    vzNP = 0;
double cMuLo[NSEC][NPMAX][NPHI][4], cMuHi[NSEC][NPMAX][NPHI][4];
double cSigLo[NSEC][NPMAX][NPHI][4], cSigHi[NSEC][NPMAX][NPHI][4];
double thMin[NSEC][NPMAX][NPHI], thMax[NSEC][NPMAX][NPHI];

double readNum(istringstream& iss)
{
    string tok;
    iss >> tok;
    if (tok == "nan" || tok == "NaN") return NAN;
    return atof(tok.c_str());
}

bool loadVzParams(string path, string variant)
{
    ifstream in(path.c_str());
    if (!in) { cout << "Cannot open " << path << endl; return false; }

    string line;
    int nVariants = 0, nPhi = 0, nSec = 0, nPoly = 0;
    bool haveHeader = false;
    bool inVariant = false;
    int  nCellsRead = 0, nCells = 0;

    while (getline(in, line)) {
        if (line.find_first_not_of(" \t\r") == string::npos) continue;
        if (line[line.find_first_not_of(" \t\r")] == '#') continue;
        istringstream iss(line);

        if (!haveHeader) {
            iss >> nVariants >> nPhi >> nSec >> nPoly;
            haveHeader = true;
            continue;
        }

        // Variant header lines start with a name, cell lines start with a sector number
        if (!isdigit(line[line.find_first_not_of(" \t\r")])) {
            if (inVariant) break;           // finished the variant we wanted
            string key;
            iss >> key;
            if (key != variant) continue;
            vzMu0Lo = readNum(iss); vzSig0Lo = readNum(iss);
            vzMu0Hi = readNum(iss); vzSig0Hi = readNum(iss);
            vzPLo   = readNum(iss); vzPBinW  = readNum(iss);
            iss >> vzNP;
            inVariant = true;
            nCells = NSEC * vzNP * NPHI;
            continue;
        }
        if (!inVariant) continue;

        int s, ip, iphi;
        iss >> s >> ip >> iphi;
        for (int k = 0; k < 4; k++) cMuLo[s - 1][ip][iphi][k]  = readNum(iss);
        for (int k = 0; k < 4; k++) cMuHi[s - 1][ip][iphi][k]  = readNum(iss);
        for (int k = 0; k < 4; k++) cSigLo[s - 1][ip][iphi][k] = readNum(iss);
        for (int k = 0; k < 4; k++) cSigHi[s - 1][ip][iphi][k] = readNum(iss);
        thMin[s - 1][ip][iphi] = readNum(iss);
        thMax[s - 1][ip][iphi] = readNum(iss);
        nCellsRead++;
    }

    if (!inVariant || nCellsRead != nCells) {
        cout << "Problem reading variant " << variant << " (" << nCellsRead << " of " << nCells << " cells)" << endl;
        return false;
    }
    cout << "Loaded Vz variant " << variant << ": " << nCellsRead << " cells, "
         << vzNP << " p bins from " << vzPLo << " GeV, width " << vzPBinW << " GeV" << endl;
    return true;
}

// phi_deg: lab azimuth of the electron in degrees, sector: PCAL sector (1-6)
double correctVz(double vz, double p, double theta_deg, double phi_deg, int sector)
{
    // momentum bin
    int ip = (int)floor((p - vzPLo) / vzPBinW);
    if (ip < 0) ip = 0;
    if (ip > vzNP - 1) ip = vzNP - 1;

    // local phi inside the sector: sector centers at 0, 60, 120, 180, -120, -60 deg
    const double phiCenter[6] = {0.0, 60.0, 120.0, 180.0, -120.0, -60.0};
    double r = fmod(phi_deg - phiCenter[sector - 1] + 540.0, 360.0);
    if (r < 0) r += 360.0;
    double phiLocal = r - 180.0;                          // in [-180, 180)
    if (phiLocal < -30.0) phiLocal = -30.0;
    if (phiLocal > 30.0 - 1e-9) phiLocal = 30.0 - 1e-9;
    int iphi = (int)((phiLocal + 30.0) / 10.0);           // 6 bins of 10 deg
    if (iphi < 0) iphi = 0;
    if (iphi > NPHI - 1) iphi = NPHI - 1;

    int s = sector - 1;

    // theta clamped to the range where the cell was fitted
    double tmin = thMin[s][ip][iphi], tmax = thMax[s][ip][iphi];
    if (std::isnan(tmin)) tmin = 6.0;
    if (std::isnan(tmax)) tmax = 30.0;
    double th = theta_deg;
    if (th < tmin) th = tmin;
    if (th > tmax) th = tmax;

    double muLo  = poly3(cMuLo[s][ip][iphi], th);
    double muHi  = poly3(cMuHi[s][ip][iphi], th);
    double sigLo = max(poly3(cSigLo[s][ip][iphi], th), 0.01);
    double sigHi = max(poly3(cSigHi[s][ip][iphi], th), 0.01);

    // standardize against each peak and map to the reference frame
    double zLo = (vz - muLo) / sigLo;
    double zHi = (vz - muHi) / sigHi;
    double vzLo = vzMu0Lo + vzSig0Lo * zLo;
    double vzHi = vzMu0Hi + vzSig0Hi * zHi;

    // Eq. 6: logistic blend, weight of the "lo" branch
    double x = 0.5 * (zHi * zHi - zLo * zLo) + log(sigHi / sigLo);
    double wLo = 1.0 / (1.0 + exp(-x));
    return vzHi + wLo * (vzLo - vzHi);
}

// ---------------------------------------------------------------------------
int electronCutCheck(string fileName = "", string target = "LD2",
                     string vzParamFile = "data/Vz/vz_corrector_params.txt", int max = 0)
{
    auto start = std::chrono::high_resolution_clock::now();

    if (fileName == "") {
        cout << "Give a recon HIPO file." << endl;
        return 1;
    }
    if (target != "LD2" && target != "CxC" && target != "CuSn") {
        cout << "Target must be LD2, CxC or CuSn." << endl;
        return 1;
    }

    const double* win = nullptr;
    if (target == "CuSn") {
        if (!loadVzParams(vzParamFile, "cusn_outbending")) return 1;
        win = VZWIN_CUSN;
    }
    if (target == "CxC") {
        if (!loadVzParams(vzParamFile, "cxc_outbending")) return 1;
        win = VZWIN_CXC;
    }

    hipo::reader reader;
    reader.open(fileName.c_str());
    hipo::dictionary factory;
    reader.readDictionary(factory);
    hipo::event event;

    hipo::bank PART(factory.getSchema("REC::Particle"));
    hipo::bank CALO(factory.getSchema("REC::Calorimeter"));
    hipo::bank TRAJ(factory.getSchema("REC::Traj"));

    // Counters, in the order of the note's Table 5
    long nEvents   = 0;
    long nBaseline = 0;  // pid 11, status in (-4000, -2000]
    long nMom      = 0;
    long nSF       = 0;
    long nPCAL     = 0;
    long nDC       = 0;
    long nVz       = 0;
    // Side chain: DC with the code thresholds, then Vz (to compare with clas-framework)
    long nDC_code  = 0;
    long nVz_code  = 0;
    // Vz breakdown for solid targets (after the main DC cut)
    long nVzUp = 0, nVzDown = 0;
    // Same foil windows applied to the RAW Vz (no correction), after the main DC cut
    long nVzRaw = 0, nVzRawUp = 0, nVzRawDown = 0;

    // Diagnostics
    long nNoPCAL = 0, nNoDC = 0;

    while (reader.next() == true)
    {
        nEvents++;
        if (max > 0 && nEvents > max) break;

        reader.read(event);
        event.getStructure(PART);
        event.getStructure(CALO);
        event.getStructure(TRAJ);

        // --- Trigger electron: first row with pid 11 and status in (-4000, -2000] ---
        int eIdx = -1;
        for (int i = 0; i < PART.getRows(); i++) {
            int pid    = PART.getInt("pid", i);
            int status = PART.getInt("status", i);
            if (pid == 11 && status <= -2000 && status > -4000) { eIdx = i; break; }
        }
        if (eIdx < 0) continue;

        double px = PART.getFloat("px", eIdx);
        double py = PART.getFloat("py", eIdx);
        double pz = PART.getFloat("pz", eIdx);
        double vz = PART.getFloat("vz", eIdx);
        double p  = sqrt(px * px + py * py + pz * pz);
        if (p <= 0) continue;
        nBaseline++;
        double theta = acos(pz / p) * 180.0 / M_PI;
        double phi   = atan2(py, px) * 180.0 / M_PI;

        // --- Calorimeter: PCAL (layer 1), ECIN (4), ECOUT (7) ---
        double ePCAL = 0, eECIN = 0, eECOUT = 0;
        double lv = -1, lw = -1;
        int sector = 0;
        for (int i = 0; i < CALO.getRows(); i++) {
            if (CALO.getShort("pindex", i) != eIdx) continue;
            int layer = CALO.getByte("layer", i);
            if (layer == 1) {
                ePCAL  = CALO.getFloat("energy", i);
                lv     = CALO.getFloat("lv", i);
                lw     = CALO.getFloat("lw", i);
                sector = CALO.getByte("sector", i);
            }
            else if (layer == 4) eECIN  = CALO.getFloat("energy", i);
            else if (layer == 7) eECOUT = CALO.getFloat("energy", i);
        }

        // --- DC edges: detector 6, layers 6 / 18 / 36 = regions 1 / 2 / 3 ---
        double edge[3] = {-999, -999, -999};
        for (int i = 0; i < TRAJ.getRows(); i++) {
            if (TRAJ.getShort("pindex", i) != eIdx) continue;
            if (TRAJ.getByte("detector", i) != 6) continue;
            int layer = TRAJ.getByte("layer", i);
            if (layer == 6)  edge[0] = TRAJ.getFloat("edge", i);
            if (layer == 18) edge[1] = TRAJ.getFloat("edge", i);
            if (layer == 36) edge[2] = TRAJ.getFloat("edge", i);
        }

        // --- Cutflow (each cut only sees electrons that passed the previous ones) ---
        if (p <= P_MIN) continue;
        nMom++;

        if (sector < 1 || sector > 6) { nNoPCAL++; continue; }  // no PCAL hit: cannot compute SF cut
        double SF  = (ePCAL + eECIN + eECOUT) / p;
        double mu  = poly3(SF_MU[sector - 1], p);
        double sig = poly3(SF_SIG[sector - 1], p);
        if (SF < mu - N_SIGMA * sig || SF > mu + N_SIGMA * sig) continue;
        nSF++;

        if (lv <= PCAL_MIN || lw <= PCAL_MIN) continue;
        nPCAL++;

        if (edge[0] == -999 || edge[1] == -999 || edge[2] == -999) nNoDC++;

        // --- Vz decision (same for both DC chains) ---
        bool passVz = false, inUp = false, inDown = false;
        bool rawUp = false, rawDown = false;
        if (target == "LD2") {
            passVz = (vz >= VZ_LD2_LO && vz <= VZ_LD2_HI);
        } else {
            double vzc = correctVz(vz, p, theta, phi, sector);
            inUp   = fabs(vzc - win[0]) < VZWIN_NSIG * win[1];
            inDown = fabs(vzc - win[2]) < VZWIN_NSIG * win[3];
            passVz = inUp || inDown;
            rawUp   = fabs(vz - win[0]) < VZWIN_NSIG * win[1];
            rawDown = fabs(vz - win[2]) < VZWIN_NSIG * win[3];
        }

        // Side chain with the code DC thresholds
        if (edge[0] > DC_CODE[0] && edge[1] > DC_CODE[1] && edge[2] > DC_CODE[2]) {
            nDC_code++;
            if (passVz) nVz_code++;
        }

        // Main chain with the note's Table 3 thresholds
        if (!(edge[0] > DC_NOTE[0] && edge[1] > DC_NOTE[1] && edge[2] > DC_NOTE[2])) continue;
        nDC++;

        if (rawUp || rawDown) nVzRaw++;
        if (rawUp)   nVzRawUp++;
        if (rawDown) nVzRawDown++;

        if (!passVz) continue;
        nVz++;
        if (inUp)   nVzUp++;
        if (inDown) nVzDown++;
    }

    // --- Print cutflow ---
    cout << endl << "File: " << fileName << "   Target: " << target << endl;
    cout << "Events read: " << nEvents << endl << endl;

    string vzName = (target == "LD2") ? "Vz [-15, 5] cm (LD2)" : "Vz corrected, foil windows";
    const char* names[6] = {"PID + status (baseline)", "Momentum > 1.8 GeV", "SF mu +/- 3.5 sigma",
                            "PCAL lv, lw > 9 cm", "DC edge (note Table 3)", vzName.c_str()};
    long counts[6] = {nBaseline, nMom, nSF, nPCAL, nDC, nVz};

    cout << left << setw(28) << "Cut" << right << setw(12) << "Count"
         << setw(14) << "Cumulative" << setw(10) << "Step" << endl;
    for (int i = 0; i < 6; i++) {
        double cum  = nBaseline > 0 ? (double)counts[i] / nBaseline : 0;
        double step = (i > 0 && counts[i - 1] > 0) ? (double)counts[i] / counts[i - 1] : 1.0;
        cout << left << setw(28) << names[i] << right << setw(12) << counts[i]
             << fixed << setprecision(4) << setw(14) << cum << setw(10) << step << endl;
    }
    if (target != "LD2")
    {
        cout << "   in upstream window: " << nVzUp << ",  in downstream window: " << nVzDown << endl;
        cout << endl << "Same windows on RAW Vz (no correction), after the DC cut above:" << endl;
        cout << "   Vz: " << nVzRaw << "  (step " << fixed << setprecision(4)
             << (nDC > 0 ? (double)nVzRaw / nDC : 0) << ")" << endl;
        cout << "   in upstream window: " << nVzRawUp << ",  in downstream window: " << nVzRawDown << endl;
    }

    cout << endl << "Side chain with code DC thresholds (1.68, 2.00, 8.75):" << endl;
    cout << "   DC: " << nDC_code << "  (step " << fixed << setprecision(4)
         << (nPCAL > 0 ? (double)nDC_code / nPCAL : 0) << ")" << endl;
    cout << "   Vz: " << nVz_code << "  (step " << fixed << setprecision(4)
         << (nDC_code > 0 ? (double)nVz_code / nDC_code : 0) << ",  cumulative "
         << (nBaseline > 0 ? (double)nVz_code / nBaseline : 0) << ")" << endl;

    cout << endl << "Electrons with no PCAL hit (failed at SF): " << nNoPCAL << endl;
    cout << "Electrons after PCAL missing a DC region in REC::Traj: " << nNoDC << endl;

    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    cout << "Elapsed time: " << elapsed.count() << " s" << endl;

    return 0;
}