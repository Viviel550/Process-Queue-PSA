#include <iomanip>
#include <iostream>
#include <string>

#include "algorithms/algo_registry.h"
#include "experiments/average_runs.h"
#include "experiments/csv_output.h"
#include "experiments/experiments.h"

// D: pelna siatka (n x mix) dla algorytmow o domyslnych parametrach + podsumowanie na ekranie
void runExperimentD(const ExperimentConfig& cfg) {
    const auto algos = defaultAlgos(cfg.sim);
    auto f = openCsv("results_D_comparison.csv");
    std::cout << "\nAdaptiveRR(8,1) vs RR(8) - zmiana sredniego czasu oczekiwania ('-' = adaptacyjny lepszy):\n";
    std::cout << std::fixed << std::setprecision(2);
    for (Mix mix : {Mix::Short, Mix::Long, Mix::Mixed})
        for (int n : {10, 50, 200}) {
            Metrics rrM, adM;
            for (const auto& a : algos) {
                const Metrics m = averageOverRuns(a, n, mix, cfg);
                writeRow(f, "D", std::string(mixName(mix)) + " n=" + std::to_string(n), n, mix, a.name, cfg.runs, m);
                if (a.name.rfind("RR(", 0) == 0) rrM = m;
                if (a.name.rfind("AdaptiveRR", 0) == 0) adM = m;
            }
            const double change = rrM.avgWaiting > 0 ? 100.0 * (adM.avgWaiting - rrM.avgWaiting) / rrM.avgWaiting : 0;
            std::cout << "  " << std::setw(6) << mixName(mix) << " n=" << std::setw(3) << n
                      << "  wait RR=" << std::setw(8) << rrM.avgWaiting << "  Adaptive=" << std::setw(8)
                      << adM.avgWaiting << "  zmiana=" << std::setw(7) << change << "%"
                      << "  ctxSw RR=" << rrM.contextSwitches << " Adaptive=" << adM.contextSwitches << "\n";
        }
    std::cout << "  results_D_comparison.csv\n";
}
