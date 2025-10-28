#pragma once

#include <limits>

namespace Core {

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
};


}  // namespace Core