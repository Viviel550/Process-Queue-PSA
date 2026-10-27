#include <iostream>
#include <string>

#include "algorithms/algo_registry.h"
#include "experiments/average_runs.h"
#include "experiments/csv_output.h"
#include "experiments/experiments.h"

// A: wplyw liczby procesow (mixed)
void runExperimentA(const ExperimentConfig& cfg) {
    const auto algos = defaultAlgos(cfg.sim);
    auto f = openCsv("results_A_process_count.csv");
    for (int n : {10, 20, 50, 100, 200, 500})
        for (const auto& a : algos)
            writeRow(f, "A", "n=" + std::to_string(n), n, Mix::Mixed, a.name, cfg.runs,
                     averageOverRuns(a, n, Mix::Mixed, cfg));
    std::cout << "  results_A_process_count.csv\n";
}
