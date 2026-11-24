// ecorr_step1.C : photon energy correction, first step (CLAS note 2006-015, Sec. 2.1), from the
// PhotonAnalysis/Pi0/mgg_vs_E1 histogram of photon_check/run_photon.sh (photon_check v4).
//
// Usage (from the clas-framework folder):
//   root -l -b -q 'photon_check/ecorr_step1.C("<photon_check_dir>", "<out_dir>", "OB", 0.5, 0.9, 0.08, 0.30)'
//   arguments: base folder, output folder, polarity (OB|IB), E1 range [GeV], IM(gg) fit range [GeV]
//
// Reads:  <photon_check_dir>/runs.txt                          the run list run_photon.sh used ("<T> <OB|IB> <run>")
//         <photon_check_dir>/root/photon_<T>_<POL>_<run>.root  PhotonAnalysis/Pi0/mgg_vs_E1: IM(gg) vs E1 for the
//                                                              pairs with E1 - E2 < 0.01 GeV (PhotonAnalysisAlg.hpp)
// Writes: <out_dir>/<pol>_ecorr_step1_slices.pdf   IM(gg) in every E1 bin with its fit, one page per target
//         <out_dir>/<pol>_ecorr_step1_corr.pdf     IM(gg)/m_pi0 vs E1 with the fit of corr(E), one panel per target
//         and prints, per target, the fit of every E1 bin and the corr(E) parameters.
//
// Method (note Sec. 2.1, every target separately):
//   - the mgg_vs_E1 histograms of all the runs of a target in runs.txt are summed before fitting;
//   - only the E1 bins inside [emin, emax] are used (note step 1: 0.5 to 0.9 GeV);
//   - each E1 slice (ProjectionY of that bin) is fitted in [mlo, mhi] with gaus + pol1 (note: Gaussian plus
//     linear background), binned Poisson likelihood (fit option "L"), 0.09 <= mu <= 0.18 GeV and
//     0.003 <= sigma <= 0.05 GeV. Every bin is fitted, none is skipped for low statistics;
//   - mu / m_pi0, with m_pi0 = 0.1349768 GeV (rgd-pi0 kPi0MassGeV), at the E1 bin centre (the histogram has no
//     per-pair E1), error sigma_mu / m_pi0;
//   - fitted with corr(E) = a + b/E + c/E^2 (note eq. 6). A slice fit that did not converge, ends on a
//     limit of mu or sigma, or has no error is drawn open and left out of the corr(E) fit (a fit whose
//     status is not 0 is refitted once from its result first).

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "TCanvas.h"
#include "TColor.h"
#include "TF1.h"
#include "TFile.h"
#include "TFitResult.h"
#include "TGraphErrors.h"
#include "TH1D.h"
#include "TH2.h"
#include "TLatex.h"
#include "TLegend.h"
#include "TLine.h"
#include "TBox.h"
#include "TStyle.h"
#include "TSystem.h"

namespace {
const double kMPi0 = 0.1349768;  // GeV, rgd-pi0 src/core/Constants.hpp kPi0MassGeV
const double kMuLo = 0.09, kMuHi = 0.18, kSgLo = 0.003, kSgHi = 0.05;

// rgd-pi0 target colours (D black, C blue, Cu red, Sn green); CuSn (both foils) as in photon_figs.py
int target_color(const std::string& t) {
    if (t == "LD2") return kBlack;
    if (t == "CxC") return TColor::GetColor("#1f77b4");
    if (t == "Cu") return TColor::GetColor("#d62728");
    if (t == "Sn") return TColor::GetColor("#2ca02c");
    if (t == "CuSn") return TColor::GetColor("#9467bd");
    return kGray + 2;
}
std::string target_name(const std::string& t) {
    if (t == "LD2") return "D (LD2)";
    if (t == "CxC") return "C (CxC)";
    return t;
}
struct Slice {
    double lo, hi, n_in;
    TH1D* h = nullptr;
    TF1* f = nullptr;
    bool fitted = false, ok = false;
    int status = -1;  // Minuit status of the (last) fit, 0 = converged
    double mu = 0, dmu = 0, sg = 0, dsg = 0;
};
}  // namespace

void ecorr_step1(const char* base, const char* out, const char* pol = "OB", double emin = 0.5, double emax = 0.9,
                 double mlo = 0.08, double mhi = 0.30) {
    gStyle->SetOptStat(0);
    gStyle->SetOptTitle(1);
    gStyle->SetTitleFontSize(0.07);
    gSystem->mkdir(out, kTRUE);
    const std::string P = pol;
    std::string pl = P;
    for (auto& c : pl) c = std::tolower(c);

    // runs.txt -> targets of this polarity, in the order LD2, CxC, Cu, Sn, CuSn
    std::vector<std::pair<std::string, int>> runs;
    {
        std::ifstream in(std::string(base) + "/runs.txt");
        std::string line;
        while (std::getline(in, line)) {
            std::istringstream ss(line);
            std::string t, p;
            int run;
            if (!(ss >> t >> p >> run) || t[0] == '#' || p != P) continue;
            runs.push_back({t, run});
        }
        const std::vector<std::string> order = {"LD2", "CxC", "Cu", "Sn", "CuSn"};
        std::stable_sort(runs.begin(), runs.end(), [&](auto& a, auto& b) {
            auto ia = std::find(order.begin(), order.end(), a.first) - order.begin();
            auto ib = std::find(order.begin(), order.end(), b.first) - order.begin();
            return ia < ib;
        });
    }

    const std::string slices_pdf = std::string(out) + "/" + pl + "_ecorr_step1_slices.pdf";
    const std::string corr_pdf = std::string(out) + "/" + pl + "_ecorr_step1_corr.pdf";
    std::vector<TGraphErrors*> g_ok;
    std::vector<TGraphErrors*> g_bad;
    std::vector<TF1*> f_corr;
    std::vector<std::string> labels;
    // sum the mgg_vs_E1 histograms of all the runs of each target (runs.txt order kept within a target)
    std::vector<std::string> targets;                // in the sorted order of runs
    std::map<std::string, TH2*> h2sum;               // target -> summed histogram
    std::map<std::string, std::string> runs_used;    // target -> "18424+18421"
    for (auto& [t, run] : runs) {
        const std::string fname = Form("%s/root/photon_%s_%s_%d.root", base, t.c_str(), pol, run);
        TFile* f = TFile::Open(fname.c_str());
        if (!f || f->IsZombie()) { printf("no output for %s %s run %d, skipped\n", t.c_str(), pol, run); continue; }
        auto* h2 = dynamic_cast<TH2*>(f->Get("PhotonAnalysis/Pi0/mgg_vs_E1"));
        if (!h2) { printf("%s: no PhotonAnalysis/Pi0/mgg_vs_E1, skipped\n", fname.c_str()); continue; }
        if (!h2sum.count(t)) {
            auto* hs = static_cast<TH2*>(h2->Clone(Form("mgg_vs_E1_%s", t.c_str())));
            hs->SetDirectory(nullptr);
            h2sum[t] = hs;
            runs_used[t] = std::to_string(run);
            targets.push_back(t);
        } else {
            h2sum[t]->Add(h2);
            runs_used[t] += "+" + std::to_string(run);
        }
        f->Close();
    }
    const int n_pages = targets.size();

    int page = 0;
    for (const auto& t : targets) {
        TH2* h2 = h2sum[t];
        const std::string runlab = (runs_used[t].find('+') == std::string::npos ? "run " : "runs ") + runs_used[t];
        const int col = target_color(t);

        // ---- fit every E1 slice ----
        std::vector<Slice> sl;
        for (int ix = 1; ix <= h2->GetNbinsX(); ++ix) {
            const double lo = h2->GetXaxis()->GetBinLowEdge(ix), hi = h2->GetXaxis()->GetBinUpEdge(ix);
            if (lo < emin - 1e-9 || hi > emax + 1e-9) continue;
            Slice s;
            s.lo = lo;
            s.hi = hi;
            s.h = h2->ProjectionY(Form("h_%s_%d", t.c_str(), ix), ix, ix);
            s.h->SetDirectory(nullptr);
            s.n_in = s.h->Integral(s.h->FindBin(mlo + 1e-9), s.h->FindBin(mhi - 1e-9));
            s.f = new TF1(Form("f_%s_%d", t.c_str(), ix), "gaus(0)+pol1(3)", mlo, mhi);
            // start values: background line through the two ends of the range, peak at m_pi0
            const int blo = s.h->FindBin(mlo + 1e-9), bhi = s.h->FindBin(mhi - 1e-9), k = std::max((bhi - blo) / 10, 1);
            const double ylo = s.h->Integral(blo, blo + k - 1) / k, yhi = s.h->Integral(bhi - k + 1, bhi) / k;
            const double p1 = (yhi - ylo) / (mhi - mlo), p0 = ylo - p1 * mlo;
            const double A0 = std::max(s.h->GetBinContent(s.h->FindBin(kMPi0)) - (p0 + p1 * kMPi0), 1.0);
            s.f->SetParameters(A0, kMPi0, 0.015, p0, p1);
            s.f->SetParLimits(1, kMuLo, kMuHi);
            s.f->SetParLimits(2, kSgLo, kSgHi);
            s.f->SetLineColor(kRed);
            s.f->SetLineWidth(1);
            if (s.n_in >= 5) {
                TFitResultPtr r = s.h->Fit(s.f, "LRSQ0B");
                if (r.Get() != nullptr && int(r) != 0) r = s.h->Fit(s.f, "LRSQ0B");  // one refit from the result
                s.fitted = (r.Get() != nullptr);
                s.status = int(r);
                if (s.fitted) {
                    s.mu = s.f->GetParameter(1);
                    s.dmu = s.f->GetParError(1);
                    s.sg = s.f->GetParameter(2);
                    s.dsg = s.f->GetParError(2);
                    const bool at_limit = std::abs(s.mu - kMuLo) < 1e-4 || std::abs(s.mu - kMuHi) < 1e-4 ||
                                          std::abs(s.sg - kSgLo) < 1e-4 || std::abs(s.sg - kSgHi) < 1e-4;
                    s.ok = (int(r) == 0) && !at_limit && s.dmu > 0 && std::isfinite(s.dmu);
                }
            }
            sl.push_back(s);
        }

        // ---- printout and graphs ----
        printf("\n=== %s %s %s: E1 in [%g, %g] GeV, IM(gg) fit in [%g, %g] GeV, m_pi0 = %.7f GeV\n",
               target_name(t).c_str(), pol, runlab.c_str(), emin, emax, mlo, mhi, kMPi0);
        printf("%16s %7s %16s %16s %10s %8s %7s\n", "E1 bin [GeV]", "pairs", "mu [MeV]", "sigma [MeV]", "mu/m_pi0", "err", "status");
        auto* go = new TGraphErrors();
        auto* gb = new TGraphErrors();
        for (auto& s : sl) {
            const std::string tag = Form("[%.2f, %.2f)", s.lo, s.hi);
            if (!s.fitted) { printf("%16s %7.0f   fit failed\n", tag.c_str(), s.n_in); continue; }
            printf("%16s %7.0f %7.2f +- %-6.2f %7.2f +- %-6.2f %10.4f %8.4f %7d%s\n", tag.c_str(), s.n_in, 1e3 * s.mu,
                   1e3 * s.dmu, 1e3 * s.sg, 1e3 * s.dsg, s.mu / kMPi0, s.dmu / kMPi0, s.status,
                   s.ok ? "" : "   not converged / at a limit / no error: not used in corr(E)");
            const double E = 0.5 * (s.lo + s.hi);
            if (s.ok) {
                const int i = go->GetN();
                go->SetPoint(i, E, s.mu / kMPi0);
                go->SetPointError(i, 0, s.dmu / kMPi0);
            } else {
                gb->SetPoint(gb->GetN(), E, s.mu / kMPi0);
            }
        }
        go->SetMarkerStyle(20); go->SetMarkerSize(0.8); go->SetMarkerColor(col); go->SetLineColor(col);
        gb->SetMarkerStyle(24); gb->SetMarkerSize(0.9); gb->SetMarkerColor(col);
        TF1* fc = nullptr;
        if (go->GetN() >= 4) {
            fc = new TF1(Form("corr_%s", t.c_str()), "[0]+[1]/x+[2]/(x*x)", emin, emax);
            fc->SetParameters(1.0, 0.0, 0.0);
            fc->SetLineColor(col);
            fc->SetLineWidth(1);
            TFitResultPtr rc = go->Fit(fc, "SQ0");
            printf("corr(E) = a + b/E + c/E^2:  a = %.5f +- %.5f,  b = %.5f +- %.5f,  c = %.6f +- %.6f   "
                   "chi2/ndf = %.1f/%d%s\n",
                   fc->GetParameter(0), fc->GetParError(0), fc->GetParameter(1), fc->GetParError(1),
                   fc->GetParameter(2), fc->GetParError(2), fc->GetChisquare(), fc->GetNDF(),
                   int(rc) == 0 ? "" : "   (corr fit did not converge)");
        } else {
            printf("fewer than 4 good slices: no corr(E) fit\n");
        }
        g_ok.push_back(go);
        g_bad.push_back(gb);
        f_corr.push_back(fc);
        labels.push_back(target_name(t) + " " + runlab);

        // ---- slices page ----
        const int n = sl.size();
        const int ncol = std::min(n, 4), nrow = (n + ncol - 1) / std::max(ncol, 1);
        TCanvas c("c_sl", "", 300 * ncol, 240 * nrow + 40);
        c.Divide(ncol, nrow, 0.002, 0.002);
        for (int k = 0; k < n; ++k) {
            auto& s = sl[k];
            c.cd(k + 1);
            gPad->SetLeftMargin(0.14); gPad->SetBottomMargin(0.14);
            s.h->SetTitle(Form("%s: E_{1} #in [%.2f, %.2f) GeV, %.0f pairs;IM(#gamma#gamma) [GeV];pairs", t.c_str(), s.lo, s.hi, s.n_in));
            s.h->SetLineColor(col); s.h->SetMarkerColor(col); s.h->SetMarkerStyle(20); s.h->SetMarkerSize(0.3);
            s.h->GetXaxis()->SetRangeUser(0.0, 0.3);
            s.h->Draw("E");
            const double ymax = s.h->GetMaximum() * 1.05;
            auto* box = new TBox(mlo, 0, mhi, ymax); box->SetFillColorAlpha(kGray, 0.25); box->Draw();
            auto* ln = new TLine(kMPi0, 0, kMPi0, ymax); ln->SetLineStyle(3); ln->SetLineColor(kGray + 2); ln->Draw();
            s.h->Draw("E same");
            TLatex tx; tx.SetNDC(); tx.SetTextSize(0.06); tx.SetTextAlign(33);
            if (s.fitted) {
                s.f->Draw("same");
                auto* bg = new TF1(Form("bg_%s_%d", t.c_str(), k), "pol1", mlo, mhi);
                bg->SetParameters(s.f->GetParameter(3), s.f->GetParameter(4));
                bg->SetLineColor(kRed); bg->SetLineStyle(2); bg->SetLineWidth(1); bg->Draw("same");
                tx.DrawLatex(0.95, 0.88, Form("#mu = %.1f MeV", 1e3 * s.mu));
                tx.DrawLatex(0.95, 0.80, Form("#sigma = %.1f MeV", 1e3 * s.sg));
                if (!s.ok) tx.DrawLatex(0.95, 0.72, "not used");
            } else {
                tx.DrawLatex(0.95, 0.88, "fit failed");
            }
        }
        ++page;
        const char* mode = (page == 1 && n_pages > 1) ? "(" : (page == n_pages && n_pages > 1) ? ")" : "";
        c.SetTitle(Form("%s %s %s", target_name(t).c_str(), pol, runlab.c_str()));
        c.Print((slices_pdf + mode).c_str(), Form("Title:%s %s %s", target_name(t).c_str(), pol, runlab.c_str()));
    }

    // ---- corr page: one panel per target, same axes ----
    const int nt = g_ok.size();
    const int pcol = (nt == 4) ? 2 : std::max(nt, 1), prow = (nt == 4) ? 2 : 1;
    TCanvas cc("c_corr", "", 500 * pcol, 380 * prow);
    cc.Divide(pcol, prow, 0.002, 0.002);
    const double ylo = 0.85, yhi = 1.05;
    std::vector<TH1D*> frames;
    for (int i = 0; i < nt; ++i) {
        cc.cd(i + 1);
        gPad->SetLeftMargin(0.14); gPad->SetBottomMargin(0.13);
        auto* frame = new TH1D(Form("frame_%d", i),
                               Form("%s %s, E_{1} - E_{2} < 0.01 GeV;E_{1} [GeV] (bin centre);IM(#gamma#gamma) / m_{#pi^{0}}",
                                    labels[i].c_str(), pol),
                               10, emin - 0.02, emax + 0.02);
        frame->SetDirectory(nullptr);
        frame->SetMinimum(ylo); frame->SetMaximum(yhi);
        frame->Draw("AXIS");
        frames.push_back(frame);
        auto* one = new TLine(emin - 0.02, 1.0, emax + 0.02, 1.0); one->SetLineStyle(3); one->Draw();
        if (g_ok[i]->GetN()) g_ok[i]->Draw("P same");
        if (g_bad[i]->GetN()) g_bad[i]->Draw("P same");
        if (f_corr[i]) {
            f_corr[i]->Draw("same");
            TLatex tp; tp.SetNDC(); tp.SetTextSize(0.045); tp.SetTextColor(f_corr[i]->GetLineColor());
            tp.DrawLatex(0.17, 0.84, Form("a = %.4f, b = %.4f, c = %.5f", f_corr[i]->GetParameter(0),
                                          f_corr[i]->GetParameter(1), f_corr[i]->GetParameter(2)));
        }
        TLatex ttl; ttl.SetNDC(); ttl.SetTextSize(0.05); ttl.SetTextAlign(21);
        ttl.DrawLatex(0.55, 0.94, Form("%s %s, E_{1} - E_{2} < 0.01 GeV", labels[i].c_str(), pol));
        TLatex note; note.SetNDC(); note.SetTextSize(0.04);
        note.DrawLatex(0.17, 0.17, "line: a + b/E + c/E^{2};  open: not used in the line fit");
    }
    cc.Print(corr_pdf.c_str());
    printf("\nwrote %s and %s\n", slices_pdf.c_str(), corr_pdf.c_str());
}
