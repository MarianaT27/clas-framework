// fig311.C : reproduce note Fig. 3.11 (per-sector Vz migration)
// Top row: raw Vz split into 10 contiguous intervals per sector.
// Bottom row: corrected Vz of the same electrons, keeping each interval's color.
// Needs the ntuple with e_vz, e_vz_corr, e_sector.
// Usage: root -l 'macros/fig311.C("vzstudy_CuSn_018561.root","CuSn OB")'
void fig311(const char* fname = "vzstudy_CuSn_018561.root", const char* label = "CuSn OB",
            double vz_lo = -10.5, double vz_hi = -0.5) {
    ROOT::RDataFrame df("electron_kinematics", fname);

    const int nInt = 10, nSec = 6, nBins = 220;
    const double xlo = -11.0, xhi = 0.5;
    const double width = (vz_hi - vz_lo) / nInt;
    const int colors[nInt] = {kBlue+1, kOrange+1, kGreen+2, kRed+1, kViolet+1,
                              kOrange+3, kPink+6, kGray+1, kYellow+2, kCyan+1};

    // Book all histograms before running the event loop once
    std::vector<ROOT::RDF::RResultPtr<TH1D>> hRaw, hCorr;
    for (int s = 1; s <= nSec; ++s) {
        for (int k = 0; k < nInt; ++k) {
            double a = vz_lo + k * width, b = a + width;
            auto sel = df.Filter(Form("e_sector == %d && e_vz >= %f && e_vz < %f", s, a, b));
            hRaw.push_back(sel.Histo1D({Form("raw_s%d_%d", s, k), "", nBins, xlo, xhi}, "e_vz"));
            hCorr.push_back(sel.Histo1D({Form("corr_s%d_%d", s, k), "", nBins, xlo, xhi}, "e_vz_corr"));
        }
    }

    auto c = new TCanvas("c311", Form("Fig 3.11 %s", label), 1800, 650);
    c->Divide(nSec, 2, 0.002, 0.004);
    for (int s = 1; s <= nSec; ++s) {
        auto stRaw  = new THStack(Form("stRaw%d", s),  Form("s%d before;v_{z} [cm];counts", s));
        auto stCorr = new THStack(Form("stCorr%d", s), Form("s%d after;v_{z} [cm];counts", s));
        for (int k = 0; k < nInt; ++k) {
            int i = (s - 1) * nInt + k;
            for (auto* h : {hRaw[i].GetPtr(), hCorr[i].GetPtr()}) {
                h->SetFillColor(colors[k]); h->SetLineColor(colors[k]);
            }
            stRaw->Add(hRaw[i].GetPtr());
            stCorr->Add(hCorr[i].GetPtr());
        }
        c->cd(s);        stRaw->Draw("hist");
        for (int k = 0; k <= nInt; ++k) {      // interval boundaries
            double x = vz_lo + k * width;
            auto l = new TLine(x, 0, x, stRaw->GetMaximum()); l->SetLineStyle(3); l->Draw();
        }
        c->cd(s + nSec); stCorr->Draw("hist");
    }
    c->SaveAs(Form("fig311_%s.png", TString(label).ReplaceAll(" ", "_").Data()));
}