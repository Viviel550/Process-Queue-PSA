#include <iostream>
#include <string>

#include "algorithms/algo_registry.h"
#include "experiments/average_runs.h"
#include "experiments/csv_output.h"
#include "experiments/experiments.h"

// C: parametry kwantu (n = 50, mixed): RR(q), adaptacyjny (q0, qmin)
void runExperimentC(const ExperimentConfig& cfg) {
    auto f = openCsv("results_C_quantum_params.csv");
    for (int q : {1, 2, 4, 8, 16, 32}) {
        const Algo a = makeRR(q, cfg.sim);
        writeRow(f, "C", "RR q=" + std::to_string(q), 50, Mix::Mixed, a.name, cfg.runs,
                 averageOverRuns(a, 50, Mix::Mixed, cfg));
    }
    for (int q0 : {4, 8, 16, 32, 64})
        for (int qmin : {1, 2, 4}) {
            if (qmin > q0) continue;
            const Algo a = makeAdaptiveRR(q0, qmin, cfg.sim);
            writeRow(f, "C", "q0=" + std::to_string(q0) + " qmin=" + std::to_string(qmin), 50, Mix::Mixed,
                     a.name, cfg.runs, averageOverRuns(a, 50, Mix::Mixed, cfg));
        }
    std::cout << "  results_C_quantum_params.csv\n";
}
