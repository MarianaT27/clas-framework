#pragma once

#include <clas12/framework/core/TypedEventStore.hpp>

namespace clas12::framework {

/**
 * @brief Standard decision keys for inter-algorithm communication
 *
 * This namespace provides pre-defined TypedKey<bool> constants for common
 * algorithm decisions. Using these standardized keys ensures consistency
 * across algorithms and enables compile-time type checking.
 *
 * Decisions are boolean flags that algorithms set to communicate whether
 * certain conditions were met. Downstream algorithms can then check these
 * decisions to determine their processing path.
 *
 * Example usage:
 * @code
 * // In electron selection algorithm:
 * event.set_decision_typed(Decisions::HasGoodElectron, !electrons.empty());
 *
 * // In downstream algorithm:
 * if (event.passes_decision_typed(Decisions::HasGoodElectron)) {
 *     // Process event with good electron
 * }
 * @endcode
 *
 * @see Requirements 8.4
 */
namespace Decisions {

// =============================================================================
// Electron-related decisions
// =============================================================================

/// Event has at least one good electron passing all cuts
inline const TypedKey<bool> HasGoodElectron{"HasGoodElectron"};

/// Event has a trigger electron (first electron in FD)
inline const TypedKey<bool> HasTriggerElectron{"HasTriggerElectron"};

/// Event has a DIS-quality electron (good electron + DIS kinematics)
inline const TypedKey<bool> HasGoodDISElectron{"HasGoodDISElectron"};

/// Event passed electron selection cuts
inline const TypedKey<bool> PassedElectronCuts{"PassedElectronCuts"};

// =============================================================================
// Photon-related decisions
// =============================================================================

/// Event has at least one good photon
inline const TypedKey<bool> HasGoodPhoton{"HasGoodPhoton"};

/// Event has a photon pair suitable for pi0 reconstruction
inline const TypedKey<bool> HasPhotonPair{"HasPhotonPair"};

/// Event passed photon selection cuts
inline const TypedKey<bool> PassedPhotonCuts{"PassedPhotonCuts"};

// =============================================================================
// Meson reconstruction decisions
// =============================================================================

/// Event has a reconstructed pi0
inline const TypedKey<bool> HasPi0{"HasPi0"};

/// Event has a reconstructed eta meson
inline const TypedKey<bool> HasEta{"HasEta"};

/// Event passed pi0 mass window cut
inline const TypedKey<bool> PassedPi0MassCut{"PassedPi0MassCut"};

// =============================================================================
// Kinematic decisions
// =============================================================================

/// Event is in DIS kinematic regime (Q2 > 1, W > 2, etc.)
inline const TypedKey<bool> IsDIS{"IsDIS"};

/// Event passed fiducial volume cuts
inline const TypedKey<bool> PassedFiducialCuts{"PassedFiducialCuts"};

/// Event is in exclusive kinematic regime
inline const TypedKey<bool> IsExclusive{"IsExclusive"};

/// Event passed missing mass cuts
inline const TypedKey<bool> PassedMissingMassCut{"PassedMissingMassCut"};

// =============================================================================
// Quality and trigger decisions
// =============================================================================

/// Event passed data quality cuts
inline const TypedKey<bool> PassedDataQuality{"PassedDataQuality"};

/// Event fired the electron trigger
inline const TypedKey<bool> HasElectronTrigger{"HasElectronTrigger"};

/// Event has valid start time
inline const TypedKey<bool> HasValidStartTime{"HasValidStartTime"};

// =============================================================================
// Hadron-related decisions
// =============================================================================

/// Event has at least one good proton
inline const TypedKey<bool> HasGoodProton{"HasGoodProton"};

/// Event has at least one good pi+
inline const TypedKey<bool> HasGoodPiPlus{"HasGoodPiPlus"};

/// Event has at least one good pi-
inline const TypedKey<bool> HasGoodPiMinus{"HasGoodPiMinus"};

// =============================================================================
// Baryon-related decisions
// =============================================================================

/// Event has a reconstructed Lambda baryon
inline const TypedKey<bool> HasLambda{"HasLambda"};

// =============================================================================
// Charged meson reconstruction decisions
// =============================================================================

/// Event has a reconstructed rho0 meson
inline const TypedKey<bool> HasRho0{"HasRho0"};

/// Event has an elastic electron
inline const TypedKey<bool> HasElasticElectron{"HasElasticElectron"};

/// Event has a di-pion event
inline const TypedKey<bool> HasDiPionEvent{"HasDiPionEvent"};

/// Event has a reconstructed Lambda from decays
inline const TypedKey<bool> HasDecaysLambda{"HasDecaysLambda"};

}  // namespace Decisions

}  // namespace clas12::framework
