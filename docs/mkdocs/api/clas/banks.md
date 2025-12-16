# Banks

HIPO bank access utilities.

## Common Banks

| Bank | Description |
|------|-------------|
| `REC::Particle` | Reconstructed particles |
| `REC::Calorimeter` | Calorimeter hits |
| `REC::Track` | Tracking information |
| `REC::Traj` | Trajectory information |
| `REC::Event` | Event-level information |
| `REC::Scintillator` | Scintillator hits |
| `REC::Cherenkov` | Cherenkov detector |

## Accessing Banks

```cpp
auto execute(Framework::Event& event) -> Framework::StatusCode override {
    // Get bank
    auto rec_particle = event.get_bank("REC::Particle");
    auto rec_cal = event.get_bank("REC::Calorimeter");
    
    // Iterate rows
    for (int i = 0; i < rec_particle.get_rows(); i++) {
        int pid = rec_particle.get_as<int>("pid", i);
        float px = rec_particle.get_as<float>("px", i);
        float py = rec_particle.get_as<float>("py", i);
        float pz = rec_particle.get_as<float>("pz", i);
    }
    
    return Framework::StatusCode::SUCCESS();
}
```

## REC::Particle Columns

| Column | Type | Description |
|--------|------|-------------|
| `pid` | int | PDG particle ID |
| `px`, `py`, `pz` | float | Momentum [GeV/c] |
| `vx`, `vy`, `vz` | float | Vertex [cm] |
| `vt` | float | Vertex time [ns] |
| `charge` | int | Charge |
| `beta` | float | β = v/c |
| `chi2pid` | float | χ² PID |
| `status` | int | Status code |

## REC::Calorimeter Columns

| Column | Type | Description |
|--------|------|-------------|
| `pindex` | int | Particle index |
| `sector` | int | Sector (1-6) |
| `layer` | int | Layer (1=PCAL, 4=ECIN, 7=ECOUT) |
| `energy` | float | Energy [GeV] |
| `lu`, `lv`, `lw` | float | Local coordinates |
