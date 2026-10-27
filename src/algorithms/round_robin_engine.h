#pragma once
#include <functional>
#include <vector>

#include "core/process.h"
#include "core/sim_config.h"

// Wspolny silnik Round Robin; kwant wyznaczany funkcja liczby gotowych procesow N
// (N liczone lacznie z wybieranym procesem, na poczatku kazdej tury).
// Nowe procesy trafiaja do kolejki PRZED procesem wracajacym z tury.
RunResult runRoundRobinGeneric(std::vector<Process> in, const std::function<int(int)>& quantumFor,
                               const SimConfig& cfg = SimConfig());
