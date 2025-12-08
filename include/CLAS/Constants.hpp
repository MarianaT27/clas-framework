#pragma once

#include <numbers>

namespace CLAS::Constants {
static constexpr double ELECTRONMASS = 0.00051099891;
static constexpr double PROTONMASS = 0.938272;
static constexpr double PROTONMASS2 = PROTONMASS * PROTONMASS;
static constexpr double NEUTRONMASS = 0.93956536;
static constexpr double NEUTRONMASS2 = NEUTRONMASS * NEUTRONMASS;
static constexpr double NUCLEONMASS = (PROTONMASS + NEUTRONMASS) / 2;
static constexpr double PI0MASS = 0.1349768;
static constexpr double DEUTERIUMMASS = 1.87561294257;
static constexpr double DEUTERIUMMASS2 = DEUTERIUMMASS * DEUTERIUMMASS;
static constexpr double PIMASS = 0.1396;
static constexpr double RHO0MASS = 0.770;
static constexpr double RHO0MASS2 = RHO0MASS * RHO0MASS;
static constexpr double KMASS = 0.493677;
static constexpr double KMASS2 = KMASS * KMASS;


static constexpr double DEG_TO_RAD = std::numbers::pi / 180.0;
static constexpr double RAD_TO_DEG = 180.0 / std::numbers::pi;
static constexpr double SPEED_OF_LIGHT = 29.9792458;  // cm/ns

}  // namespace CLAS::Constantes