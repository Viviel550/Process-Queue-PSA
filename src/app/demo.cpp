#include "app/demo.h"

#include <algorithm>
#include <iomanip>
#include <iostream>

#include "algorithms/adaptive_round_robin.h"
#include "algorithms/algo_registry.h"
#include "core/metrics.h"
#include "generator/generator.h"

void runDemo(const ExperimentConfig& cfg) {
    const int n = 10;
    const auto input = generateProcesses(n, Mix::Mixed, 42);

    std::cout << "\n=== Dane wejsciowe (n=" << n << ", mixed, seed=42, kara przelaczenia="
              << cfg.sim.contextSwitchCost << ") ===\n";
    std::cout << std::setw(4) << "id" << std::setw(10) << "arrival" << std::setw(8) << "burst" << std::setw(6)
              << "prio" << "\n";
    for (const auto& p : input)
        std::cout << std::setw(4) << p.id << std::setw(10) << p.arrivalTime << std::setw(8) << p.burstTime
                  << std::setw(6) << p.priority << "\n";

    std::cout << "\n=== Porownanie algorytmow ===\n";
    std::cout << std::left << std::setw(28) << "algorytm" << std::right << std::setw(10) << "avgWait"
              << std::setw(10) << "avgTAT" << std::setw(10) << "avgResp" << std::setw(10) << "makespan"
              << std::setw(10) << "ctxSw" << "\n";
    std::cout << std::fixed << std::setprecision(2);
    for (const auto& a : defaultAlgos(cfg.sim)) {
        const auto m = computeMetrics(a.run(input));
        std::cout << std::left << std::setw(28) << a.name << std::right << std::setw(10) << m.avgWaiting
                  << std::setw(10) << m.avgTurnaround << std::setw(10) << m.avgResponse << std::setw(10)
                  << m.makespan << std::setw(10) << m.contextSwitches << "\n";
    }

    const auto res = runAdaptiveRR(input, 8, 1, cfg.sim);
    std::cout << "\n=== AdaptiveRR(q0=8, qmin=1) - wyniki per proces ===\n";
    std::cout << std::setw(4) << "id" << std::setw(9) << "arrival" << std::setw(7) << "burst" << std::setw(8)
              << "start" << std::setw(7) << "end" << std::setw(7) << "wait" << "\n";
    auto sorted = res.procs;
    std::sort(sorted.begin(), sorted.end(), [](const Process& a, const Process& b) { return a.id < b.id; });
    for (const auto& p : sorted) {
        const int wait = p.completionTime - p.arrivalTime - p.burstTime;
        std::cout << std::setw(4) << p.id << std::setw(9) << p.arrivalTime << std::setw(7) << p.burstTime
                  << std::setw(8) << p.startTime << std::setw(7) << p.completionTime << std::setw(7) << wait << "\n";
    }
}
