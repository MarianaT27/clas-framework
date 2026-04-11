#pragma once

// The physics-view bank structs (CalorimeterBank, CherenkovBank, ...) are
// generated from PHYSICS_VIEWS in scripts/gen_bankdefs.py. This header is a
// thin forwarding shim so existing `#include <clas12/Banks.hpp>` call sites
// keep working after the migration to the declarative codegen.

#include <clas12/bankdefs/generated_readers.hpp>
