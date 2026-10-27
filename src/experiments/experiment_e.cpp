#include <iostream>
#include <string>

#include "algorithms/algo_registry.h"
#include "experiments/average_runs.h"
#include "experiments/csv_output.h"
#include "experiments/experiments.h"

// E: wplyw kary za przelaczenie kontekstu (n = 50, mixed), kara z zestawu {0, 1, 2, 4, 8}
void runExperimentE(const ExperimentConfig& cfg) {
    auto f = openCsv("results_E_switch_cost.csv");
    for (int cost : {0, 1, 2, 4, 8}) {
        SimConfig sim;
        sim.contextSwitchCost = cost;
        for (const auto& a : defaultAlgos(sim))
            writeRow(f, "E", "cs_cost=" + std::to_string(cost), 50, Mix::Mixed, a.name, cfg.runs,
                     averageOverRuns(a, 50, Mix::Mixed, cfg));
    }
    std::cout << "  results_E_switch_cost.csv\n";
}
