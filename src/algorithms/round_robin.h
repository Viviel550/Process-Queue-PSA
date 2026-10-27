#pragma once
#include <vector>

#include "core/process.h"
#include "core/sim_config.h"

// Klasyczny Round Robin ze stalym kwantem q.
RunResult runRR(std::vector<Process> in, int q, const SimConfig& cfg = SimConfig());
