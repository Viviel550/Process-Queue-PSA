#pragma once
#include <vector>

#include "core/process.h"
#include "core/sim_config.h"

// FIFO / FCFS: procesy w kolejnosci przybycia, bez wywlaszczania.
RunResult runFCFS(std::vector<Process> in, const SimConfig& cfg = SimConfig());
