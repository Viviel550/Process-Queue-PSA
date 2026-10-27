#include <iostream>

#include "algorithms/algo_registry.h"
#include "experiments/average_runs.h"
#include "experiments/csv_output.h"
#include "experiments/experiments.h"

// B: dlugosc procesow (n = 50)
void runExperimentB(const ExperimentConfig& cfg) {
    const auto algos = defaultAlgos(cfg.sim);
    auto f = openCsv("results_B_burst_length.csv");
    for (Mix mix : {Mix::Short, Mix::Long, Mix::Mixed})
        for (const auto& a : algos)
            writeRow(f, "B", mixName(mix), 50, mix, a.name, cfg.runs, averageOverRuns(a, 50, mix, cfg));
    std::cout << "  results_B_burst_length.csv\n";
}
