// plotVz.C : raw vs corrected Vz for CuSn, with the note's foil windows
void plotVz(const char* fname = "baseline_CuSn_018561.root") {
    auto f = TFile::Open(fname);
    auto raw  = (TH1*)f->Get("Electron/Kinematics/before_cuts/vz");
    auto corr = (TH1*)f->Get("Electron/Kinematics/before_cuts/vz_corr");
    if (!raw || !corr) { printf("histograms not found, check with .ls\n"); return; }

    // Note Table 2, CuSn OB: mu +/- 3 sigma_eff
    double lo1 = -7.861 - 3*0.415, hi1 = -7.861 + 3*0.415;   // upstream foil
    double lo2 = -2.916 - 3*0.370, hi2 = -2.916 + 3*0.370;   // downstream foil

    // fraction of corrected Vz inside the two windows (incl. under/overflow in total)
    auto in = [&](double a, double b) {
        return corr->Integral(corr->FindBin(a), corr->FindBin(b));
    };
    double total = corr->Integral(0, corr->GetNbinsX() + 1);
    printf("in foil windows: %.3f\n", (in(lo1, hi1) + in(lo2, hi2)) / total);

    auto c = new TCanvas("cvz", "Vz CuSn", 900, 600);
    corr->SetLineColor(kBlue); corr->SetLineWidth(2);
    raw->SetLineColor(kRed);   raw->SetLineWidth(2);
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