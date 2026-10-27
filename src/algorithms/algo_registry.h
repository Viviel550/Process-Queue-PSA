#pragma once
#include <functional>
#include <string>
#include <vector>

#include "core/process.h"
#include "core/sim_config.h"

// Algorytm z nazwa i ustalonymi parametrami (w tym kara przelaczenia).
// run() jest czysta funkcja (bez stanu wspoldzielonego), wiec mozna ja wolac z wielu watkow naraz.
struct Algo {
    std::string name;
    std::function<RunResult(const std::vector<Process>&)> run;
};

Algo makeRR(int q, const SimConfig& sim);
Algo makeAdaptiveRR(int q0, int qmin, const SimConfig& sim);

std::vector<Algo> defaultAlgos(const SimConfig& sim, int rrQ = 8, int q0 = 8, int qmin = 1,
                               bool withSRTF = true, bool withPriority = true);
