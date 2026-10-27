#pragma once
#include <vector>

#include "core/process.h"
#include "core/sim_config.h"

// SJF niewywlaszczajacy: sposrod gotowych wybiera najkrotszy burstTime
// (remis: wczesniejsze przyjscie, potem id).
RunResult runSJF(std::vector<Process> in, const SimConfig& cfg = SimConfig());
