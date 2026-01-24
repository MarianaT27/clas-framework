#pragma once

/**
 * @file CLAS12.hpp
 * @brief All-in-one convenience include for CLAS12-specific components
 *
 * Include this header to get access to all CLAS12-specific components
 * including particle definitions, bank readers, and physics utilities.
 */

// Core CLAS12 types
#include <clas12/Particle.hpp>
#include <clas12/Banks.hpp>
#include <clas12/PIDs.hpp>
#include <clas12/Detectors.hpp>
#include <clas12/Constants.hpp>
#include <clas12/Target.hpp>
#include <clas12/Runs.hpp>

// Data reading utilities
#include <clas12/ReadBank.hpp>

// Physics utilities
#include <clas12/physics/Kinematics.hpp>
#include <clas12/physics/ParticleUtils.hpp>
#include <clas12/physics/DISAnalysis.hpp>
#include <clas12/physics/CompositeParticle.hpp>

// File utilities
#include <clas12/FileUtils.hpp>
#include <clas12/StringUtils.hpp>
