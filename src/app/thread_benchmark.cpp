#include "app/thread_benchmark.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

#include "algorithms/algo_registry.h"
#include "core/metrics.h"
#include "experiments/average_runs.h"
#include "generator/generator.h"

namespace {

// Obciazenie: srednie po cfg.runs zestawach (n = 400, mixed) dla kazdego algorytmu.
std::vector<Metrics> workload(const std::vector<Algo>& algos, const ExperimentConfig& cfg) {
    std::vector<Metrics> out;
    for (const auto& a : algos) out.push_back(averageOverRuns(a, 400, Mix::Mixed, cfg));
    return out;
}

}  // namespace

int runThreadBenchmark(int maxThreads, const ExperimentConfig& cfg) {
    const unsigned hw = std::thread::hardware_concurrency();
    const auto algos = defaultAlgos(cfg.sim);

    std::cout << "\n=== Test wielowatkowosci: 1.." << maxThreads << " watkow ===\n"
              << "Obciazenie: " << algos.size() << " algorytmow x " << cfg.runs
              << " zestawow (n=400, mixed), kara przelaczenia " << cfg.sim.contextSwitchCost
              << ". Rdzenie sprzetowe: " << (hw ? std::to_string(hw) : "nieznane") << "\n";
    if (hw && static_cast<unsigned>(maxThreads) > hw)
        std::cout << "Uwaga: wiecej watkow niz rdzeni - przyspieszenie nie bedzie rosnac.\n";

    std::cout << std::right << std::setw(8) << "watki" << std::setw(12) << "czas [ms]" << std::setw(16)
              << "przyspieszenie" << std::setw(13) << "efektywnosc" << std::setw(12) << "wyniki" << "\n";

    std::vector<Metrics> baseline;
    double baseMs = 0;
    int mismatches = 0;
    for (int t = 1; t <= maxThreads; ++t) {
        ExperimentConfig c = cfg;
        c.threads = t;
        const auto start = std::chrono::steady_clock::now();
        const auto res = workload(algos, c);
        const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();

        bool same = true;
        if (t == 1) {
            baseline = res;
            baseMs = ms;
        } else {
            for (size_t i = 0; i < res.size(); ++i) same = same && sameMetrics(res[i], baseline[i]);
            if (!same) ++mismatches;
        }
        const double speedup = ms > 0 ? baseMs / ms : 0;
        std::cout << std::setw(8) << t << std::setw(12) << std::fixed << std::setprecision(1) << ms << std::setw(15)
                  << std::setprecision(2) << speedup << "x" << std::setw(12) << std::setprecision(0)
                  << 100.0 * speedup / t << "%" << std::setw(12) << (t == 1 ? "(baza)" : same ? "zgodne" : "ROZNE")
                  << "\n";
    }
    std::cout << (mismatches ? "\nBLAD: wyniki zalezne od liczby watkow!\n"
                             : "\nWyniki identyczne niezaleznie od liczby watkow.\n");
    return mismatches ? 1 : 0;
}
