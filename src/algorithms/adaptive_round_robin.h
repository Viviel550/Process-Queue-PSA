#pragma once
#include <vector>

#include "core/process.h"
#include "core/sim_config.h"

// Kwant adaptacyjny: q(N) = max(qmin, ceil(q0 / N)), N >= 1 (osobno, zeby mozna go testowac).
int adaptiveQuantum(int q0, int qmin, int N);

// Adaptacyjny Round Robin.
RunResult runAdaptiveRR(std::vector<Process> in, int q0, int qmin, const SimConfig& cfg = SimConfig());
