#include <iostream>

#include "experiments/experiments.h"

void runAllExperiments(const ExperimentConfig& cfg) {
    std::cout << "\n=== Eksperymenty (" << cfg.runs << " losowych zestawow na konfiguracje, watki: " << cfg.threads
              << ", kara przelaczenia: " << cfg.sim.contextSwitchCost << ") ===\n";
    runExperimentA(cfg);
    runExperimentB(cfg);
    runExperimentC(cfg);
    runExperimentD(cfg);
    runExperimentE(cfg);
}
