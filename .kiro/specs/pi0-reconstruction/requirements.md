# Requirements Document

## Introduction

This document specifies requirements for implementing a Pi0 (π⁰) reconstruction algorithm and analysis executable for the CLAS12 Analysis Framework. The π⁰ meson decays predominantly into two photons (γγ), and this analysis will reconstruct π⁰ candidates from photon pairs in events that contain a good electron. The output will be written to an NTuple containing the electron and the two photons forming the π⁰ candidate.

## Glossary

- **Pi0 (π⁰)**: A neutral pion meson with mass ~135 MeV/c², decaying to two photons with ~99% branching ratio
- **Photon (γ)**: A neutral particle detected in the electromagnetic calorimeter
- **Invariant Mass**: The Lorentz-invariant mass calculated from the four-momenta of decay products
- **Good Electron**: An electron passing standard CLAS12 selection cuts (chi2pid, momentum, vertex)
- **NTuple**: A ROOT TTree structure for storing event-by-event analysis data
- **Opening Angle**: The angle between the two photon momentum vectors

## Requirements

### Requirement 1: Photon Selection

**User Story:** As a physicist, I want to select good photon candidates from the event, so that I can use them for π⁰ reconstruction.

#### Acceptance Criteria

1. WHEN the algorithm processes an event THEN the Pi0ReconstructionAlg SHALL retrieve all photons from the Event particle collection
2. WHEN a photon is evaluated THEN the Pi0ReconstructionAlg SHALL apply a minimum energy cut to reject low-energy background
3. WHEN photons are selected THEN the Pi0ReconstructionAlg SHALL require at least two photons to proceed with reconstruction

### Requirement 2: Pi0 Candidate Reconstruction

**User Story:** As a physicist, I want to reconstruct π⁰ candidates from photon pairs, so that I can study π⁰ production in electron scattering.

#### Acceptance Criteria

1. WHEN two or more photons are available THEN the Pi0ReconstructionAlg SHALL form all possible photon pair combinations
2. WHEN a photon pair is formed THEN the Pi0ReconstructionAlg SHALL calculate the invariant mass from the four-momenta
3. WHEN the invariant mass is calculated THEN the Pi0ReconstructionAlg SHALL apply a mass window cut around the π⁰ mass (135 MeV/c²)
4. WHEN multiple π⁰ candidates pass the mass cut THEN the Pi0ReconstructionAlg SHALL select the candidate closest to the nominal π⁰ mass

### Requirement 3: Electron-Pi0 Event Selection

**User Story:** As a physicist, I want to analyze events with both a good electron and a π⁰, so that I can study electroproduction of π⁰ mesons.

#### Acceptance Criteria

1. WHEN the main_pi0 executable processes events THEN the Application SHALL first apply electron selection cuts
2. WHEN an event passes electron selection THEN the Application SHALL proceed to π⁰ reconstruction
3. WHEN both electron and π⁰ are found THEN the Application SHALL store the event data in the NTuple

### Requirement 4: NTuple Output

**User Story:** As a physicist, I want the analysis results written to an NTuple, so that I can perform further analysis and make plots.

#### Acceptance Criteria

1. WHEN an event has a good electron and π⁰ candidate THEN the NTupleService SHALL store electron kinematics (p, theta, phi, vz)
2. WHEN an event has a good electron and π⁰ candidate THEN the NTupleService SHALL store both photon kinematics (E, theta, phi for each)
3. WHEN an event has a good electron and π⁰ candidate THEN the NTupleService SHALL store π⁰ kinematics (mass, momentum, theta, phi)
4. WHEN the analysis completes THEN the NTupleService SHALL write the NTuple to the output ROOT file

### Requirement 5: Cutflow and Diagnostics

**User Story:** As a physicist, I want to see the cutflow statistics, so that I can understand the selection efficiency at each step.

#### Acceptance Criteria

1. WHEN the algorithm finalizes THEN the Pi0ReconstructionAlg SHALL print a cutflow table showing events at each selection stage
2. WHEN histograms are booked THEN the Pi0ReconstructionAlg SHALL create diagnostic histograms for photon and π⁰ kinematics
