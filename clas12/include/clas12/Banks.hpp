#pragma once

#include <limits>

namespace clas12 {

struct CherenkovBank {
    int sector = std::numeric_limits<int>::quiet_NaN();
    float nphe = std::numeric_limits<float>::quiet_NaN();
    float time = std::numeric_limits<float>::quiet_NaN();
    float path = std::numeric_limits<float>::quiet_NaN();
    float chi2 = std::numeric_limits<float>::quiet_NaN();
    float x = std::numeric_limits<float>::quiet_NaN();
    float y = std::numeric_limits<float>::quiet_NaN();
    float z = std::numeric_limits<float>::quiet_NaN();
    float dtheta = std::numeric_limits<float>::quiet_NaN();
    float dphi = std::numeric_limits<float>::quiet_NaN();
    int status = std::numeric_limits<int>::quiet_NaN();
};

struct CalorimeterBank {
    struct CalorimeterStruct {
        int sector = std::numeric_limits<int>::quiet_NaN();
        int layer = std::numeric_limits<int>::quiet_NaN();
        float energy = 0;
        float time = std::numeric_limits<float>::quiet_NaN();
        float path = std::numeric_limits<float>::quiet_NaN();
        float chi2 = std::numeric_limits<float>::quiet_NaN();
        float x = std::numeric_limits<float>::quiet_NaN();
        float y = std::numeric_limits<float>::quiet_NaN();
        float z = std::numeric_limits<float>::quiet_NaN();
        float lu = std::numeric_limits<float>::quiet_NaN();
        float lv = std::numeric_limits<float>::quiet_NaN();
        float lw = std::numeric_limits<float>::quiet_NaN();
        float du = std::numeric_limits<float>::quiet_NaN();
        float dv = std::numeric_limits<float>::quiet_NaN();
        float dw = std::numeric_limits<float>::quiet_NaN();
        float m2u = std::numeric_limits<float>::quiet_NaN();
        float m2v = std::numeric_limits<float>::quiet_NaN();
        float m2w = std::numeric_limits<float>::quiet_NaN();
        float m3u = std::numeric_limits<float>::quiet_NaN();
        float m3v = std::numeric_limits<float>::quiet_NaN();
        float m3w = std::numeric_limits<float>::quiet_NaN();

        // Extra fields from REC::CaloExtras bank
        short dbstU = std::numeric_limits<short>::quiet_NaN();
        short dbstV = std::numeric_limits<short>::quiet_NaN();
        short dbstW = std::numeric_limits<short>::quiet_NaN();
        float rawEU = std::numeric_limits<float>::quiet_NaN();
        float rawEV = std::numeric_limits<float>::quiet_NaN();
        float rawEW = std::numeric_limits<float>::quiet_NaN();
        float recEU = std::numeric_limits<float>::quiet_NaN();
        float recEV = std::numeric_limits<float>::quiet_NaN();
        float recEW = std::numeric_limits<float>::quiet_NaN();
        float recDTU = std::numeric_limits<float>::quiet_NaN();
        float recDTV = std::numeric_limits<float>::quiet_NaN();
        float recDTW = std::numeric_limits<float>::quiet_NaN();
        float recFTU = std::numeric_limits<float>::quiet_NaN();
        float recFTV = std::numeric_limits<float>::quiet_NaN();
        float recFTW = std::numeric_limits<float>::quiet_NaN();
        float size = std::numeric_limits<float>::quiet_NaN();
    };

    CalorimeterStruct pcal;
    CalorimeterStruct inner;
    CalorimeterStruct outer;
};

struct TrackBank {
    int index = std::numeric_limits<int>::quiet_NaN();
    int pindex = std::numeric_limits<int>::quiet_NaN();
    int detector = std::numeric_limits<int>::quiet_NaN();
    int sector = std::numeric_limits<int>::quiet_NaN();
    int status = std::numeric_limits<int>::quiet_NaN();
    int q = std::numeric_limits<int>::quiet_NaN();
    float chi2 = std::numeric_limits<int>::quiet_NaN();
    int ndf = std::numeric_limits<int>::quiet_NaN();
    int hbindex = std::numeric_limits<int>::quiet_NaN();
};

struct TrajBank {
    int pindex = std::numeric_limits<int>::quiet_NaN();
    int index = std::numeric_limits<int>::quiet_NaN();
    int detector = std::numeric_limits<int>::quiet_NaN();
    int layer = std::numeric_limits<int>::quiet_NaN();
    double x = std::numeric_limits<double>::quiet_NaN();
    double y = std::numeric_limits<double>::quiet_NaN();
    double z = std::numeric_limits<double>::quiet_NaN();
    double cx = std::numeric_limits<double>::quiet_NaN();
    double cy = std::numeric_limits<double>::quiet_NaN();
    double cz = std::numeric_limits<double>::quiet_NaN();
    double path = std::numeric_limits<double>::quiet_NaN();
    double edge = std::numeric_limits<double>::quiet_NaN();
};

struct DCTrajBank {
    TrajBank region1;
    TrajBank region2;
    TrajBank region3;
};

struct ScintillatorBank {
    int index = std::numeric_limits<int>::quiet_NaN();
    int pindex = std::numeric_limits<int>::quiet_NaN();
    int detector = std::numeric_limits<int>::quiet_NaN();
    int sector = std::numeric_limits<int>::quiet_NaN();
    int layer = std::numeric_limits<int>::quiet_NaN();
    int component = std::numeric_limits<int>::quiet_NaN();
    float energy = 0;
    float time = std::numeric_limits<float>::quiet_NaN();
    float path = std::numeric_limits<float>::quiet_NaN();
    float chi2 = std::numeric_limits<float>::quiet_NaN();
    float x = std::numeric_limits<float>::quiet_NaN();
    float y = std::numeric_limits<float>::quiet_NaN();
    float z = std::numeric_limits<float>::quiet_NaN();
    float hx = std::numeric_limits<float>::quiet_NaN();
    float hy = std::numeric_limits<float>::quiet_NaN();
    float hz = std::numeric_limits<float>::quiet_NaN();
    int status = std::numeric_limits<int>::quiet_NaN();

    // Extra fields from REC::ScintExtras bank
    float dedx = std::numeric_limits<float>::quiet_NaN();
    short size = std::numeric_limits<short>::quiet_NaN();
    char layermult = std::numeric_limits<char>::quiet_NaN();
};

struct RICHParticleBank {
    int id = std::numeric_limits<int>::quiet_NaN();
    int hindex = std::numeric_limits<int>::quiet_NaN();
    int pindex = std::numeric_limits<int>::quiet_NaN();

    int emilay = std::numeric_limits<int>::quiet_NaN();
    int emico = std::numeric_limits<int>::quiet_NaN();
    int enico = std::numeric_limits<int>::quiet_NaN();
    int emqua = std::numeric_limits<int>::quiet_NaN();
    float mchi2 = std::numeric_limits<float>::quiet_NaN();
    float mass = std::numeric_limits<float>::quiet_NaN();

    int best_PID = std::numeric_limits<int>::quiet_NaN();
    float RQ = std::numeric_limits<float>::quiet_NaN();
    float ReQ = std::numeric_limits<float>::quiet_NaN();
    float el_logl = std::numeric_limits<float>::quiet_NaN();
    float pi_logl = std::numeric_limits<float>::quiet_NaN();
    float k_logl = std::numeric_limits<float>::quiet_NaN();
    float pr_logl = std::numeric_limits<float>::quiet_NaN();

    float best_ch = std::numeric_limits<float>::quiet_NaN();
    float best_c2 = std::numeric_limits<float>::quiet_NaN();
    float best_RL = std::numeric_limits<float>::quiet_NaN();
    float best_ntot = std::numeric_limits<float>::quiet_NaN();
    float best_mass = std::numeric_limits<float>::quiet_NaN();
};

}  // namespace clas12