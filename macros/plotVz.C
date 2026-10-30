// plotVz.C : raw vs corrected Vz with the note's Table 2 foil windows (OB)
// Usage: root -l 'macros/plotVz.C("baseline_CxC_018465.root","CxC")'
void plotVz(const char* fname = "baseline_CuSn_018561.root", TString target = "CuSn") {
    auto f = TFile::Open(fname);
    auto raw  = (TH1*)f->Get("Electron/Kinematics/before_cuts/vz");
    auto corr = (TH1*)f->Get("Electron/Kinematics/before_cuts/vz_corr");
    if (!raw || !corr) { printf("histograms not found, check with .ls\n"); return; }

    // Note Table 2, OB: upstream foil (mu, sigma_eff), downstream foil (mu, sigma_eff)
    double mu1, s1, mu2, s2;
    if (target == "CuSn")     { mu1 = -7.861; s1 = 0.415; mu2 = -2.916; s2 = 0.370; }
    else if (target == "CxC") { mu1 = -7.887; s1 = 0.395; mu2 = -2.906; s2 = 0.373; }
    else { printf("target must be CuSn or CxC\n"); return; }
    double lo1 = mu1 - 3*s1, hi1 = mu1 + 3*s1;
    double lo2 = mu2 - 3*s2, hi2 = mu2 + 3*s2;

    auto I = [&](double a, double b) { return corr->Integral(corr->FindBin(a + 1e-6), corr->FindBin(b - 1e-6)); };
    double total = corr->Integral(0, corr->GetNbinsX() + 1);
    double win   = I(lo1, hi1) + I(lo2, hi2);
    printf("%s: in foil windows / all:        %.3f\n", target.Data(), win / total);
    printf("%s: in foil windows / [-10,0] cm: %.3f\n", target.Data(), win / I(-10, 0));
    printf("%s: upstream (< %.2f):   %.3f\n", target.Data(), lo1, (corr->GetBinContent(0) + I(-20, lo1)) / total);
    printf("%s: between foils:       %.3f\n", target.Data(), I(hi1, lo2) / total);
    printf("%s: downstream (> %.2f): %.3f\n", target.Data(), hi2, (I(hi2, 5) + corr->GetBinContent(corr->GetNbinsX() + 1)) / total);

    auto c = new TCanvas("cvz", Form("Vz %s", target.Data()), 900, 600);
    corr->SetLineColor(kBlue); corr->SetLineWidth(2);
    raw->SetLineColor(kRed);   raw->SetLineWidth(2);
    corr->SetTitle(Form("%s OB;v_{z} [cm];Events", target.Data()));
    corr->Draw("hist");
    raw->Draw("hist same");
    gPad->SetLogy();

    double ymax = corr->GetMaximum() * 2;
    for (double x : {lo1, hi1, lo2, hi2}) {
        auto l = new TLine(x, 0.5, x, ymax);
        l->SetLineStyle(2); l->Draw();
    }
    auto leg = new TLegend(0.15, 0.75, 0.40, 0.88);
    leg->AddEntry(corr, "corrected v_{z}", "l");
    leg->AddEntry(raw,  "raw v_{z}", "l");
    leg->Draw();
}