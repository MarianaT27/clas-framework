// fig312.C : reproduce note Fig. 3.12 (Voigt fit to corrected Vz) and Table 2
// Model (note Sec. 3.1.2.2): two area-normalized Voigt profiles + flat pedestal,
// FWHM from Olivero-Longbothum, sigma_eff = FWHM / (2 sqrt(2 ln2)), window mu +/- 3 sigma_eff.
// Reads the existing vz_corr histogram (no code change needed).
// Usage: root -l 'macros/fig312_hist.C("baseline_CuSn_018561.root","CuSn OB")'

// TMath::Voigt is area-normalized; its Lorentzian argument is the FWHM (= 2 gamma_L).
double twoVoigt(double* x, double* p) {
    // p: [0] N_hi [1] mu_hi [2] sG_hi [3] gL_hi   (downstream foil)
    //    [4] N_lo [5] mu_lo [6] sG_lo [7] gL_lo   (upstream foil)
    //    [8] pedestal
    return p[0] * TMath::Voigt(x[0] - p[1], p[2], 2 * p[3])
         + p[4] * TMath::Voigt(x[0] - p[5], p[6], 2 * p[7]) + p[8];
}

void fig312_hist(const char* fname = "baseline_CuSn_018561.root", const char* label = "CuSn OB",
                 const char* hname = "Electron/Kinematics/before_cuts/vz_corr") {
    auto file = TFile::Open(fname);
    auto hin = file ? (TH1*)file->Get(hname) : nullptr;
    if (!hin) { printf("histogram %s not found in %s\n", hname, fname); return; }
    TH1D* hv = (TH1D*)hin->Clone("hvz");
    hv->SetDirectory(nullptr);
    hv->SetTitle(Form("%s  e^{-};v_{z}^{corr} [cm];counts", label));
    hv->GetXaxis()->SetRangeUser(-10, 0);   // note's fit range
    const double bw = hv->GetBinWidth(1);

    TF1 f("f", twoVoigt, -10, 0, 9);
    double n = hv->Integral(hv->FindBin(-10 + 1e-6), hv->FindBin(-1e-6)) * bw;   // counts x bin width, split between the foils
    f.SetParameters(0.5 * n, -2.9, 0.33, 0.1, 0.5 * n, -7.85, 0.35, 0.1, 1.0);
    f.SetParLimits(2, 0.05, 1.5); f.SetParLimits(3, 0.0, 1.0);
    f.SetParLimits(6, 0.05, 1.5); f.SetParLimits(7, 0.0, 1.0);
    f.SetParLimits(8, 0.0, hv->GetMaximum());
    f.SetNpx(2000);
    hv->Fit(&f, "RQ");
    hv->Fit(&f, "R");   // second pass from converged start values

    auto report = [&](const char* name, int i0) {
        double mu = f.GetParameter(i0 + 1), sG = f.GetParameter(i0 + 2), gL = f.GetParameter(i0 + 3);
        double fG = 2 * std::sqrt(2 * std::log(2)) * sG, fL = 2 * gL;
        double fwhm = 0.5346 * fL + std::sqrt(0.2166 * fL * fL + fG * fG);   // Olivero-Longbothum
        double sEff = fwhm / (2 * std::sqrt(2 * std::log(2)));
        printf("%-4s mu = %7.3f  sigma_G = %.3f  gamma_L = %.3f  FWHM = %.3f  sigma_eff = %.3f  window = [%.2f, %.2f]\n",
               name, mu, sG, gL, fwhm, sEff, mu - 3 * sEff, mu + 3 * sEff);
        return std::make_pair(mu - 3 * sEff, mu + 3 * sEff);
    };
    printf("\n%s   chi2/ndf = %.1f/%d = %.2f\n", label, f.GetChisquare(), f.GetNDF(), f.GetChisquare() / f.GetNDF());
    auto wHi = report("hi", 0);   // downstream foil (Sn or C2)
    auto wLo = report("lo", 4);   // upstream foil (Cu or C1)

    auto c = new TCanvas("c312", Form("Fig 3.12 %s", label), 800, 600);
    hv->SetLineColor(kBlack); hv->SetMarkerStyle(20); hv->SetMarkerSize(0.4);
    hv->Draw("E");
    f.SetLineColor(kGreen + 2); f.SetLineWidth(2); f.Draw("same");
    double ymax = hv->GetMaximum() * 1.05;
    for (double x : {wLo.first, wLo.second, wHi.first, wHi.second}) {
        auto l = new TLine(x, 0, x, ymax); l->SetLineStyle(2); l->SetLineColor(kGreen + 2); l->Draw();
    }
    c->SaveAs(Form("fig312hist_%s.png", TString(label).ReplaceAll(" ", "_").Data()));
}