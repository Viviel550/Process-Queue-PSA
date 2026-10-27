#include "experiments/average_runs.h"

#include <stdexcept>
#include <vector>

#include "core/validate.h"
#include "parallel/parallel_for.h"

Metrics averageOverRuns(const Algo& algo, int n, Mix mix, const ExperimentConfig& cfg) {
    std::vector<Metrics> perRun(static_cast<size_t>(cfg.runs));

    parallelFor(perRun.size(), cfg.threads, [&](std::size_t i) {
        const auto input = generateProcesses(n, mix, cfg.baseSeed + static_cast<unsigned>(i));
        const RunResult res = algo.run(input);
        if (!validate(res)) throw std::runtime_error("niepoprawny harmonogram (" + algo.name + ")");
        perRun[i] = computeMetrics(res);
    });

    Metrics acc;
    for (const auto& m : perRun) {  // stala kolejnosc sumowania => wynik niezalezny od liczby watkow
        acc.avgWaiting += m.avgWaiting;
        acc.avgTurnaround += m.avgTurnaround;
        acc.avgResponse += m.avgResponse;
        acc.makespan += m.makespan;
        acc.contextSwitches += m.contextSwitches;
    }
    const double runs = static_cast<double>(cfg.runs);
    acc.avgWaiting /= runs;
    acc.avgTurnaround /= runs;
    acc.avgResponse /= runs;
    acc.makespan /= runs;
    acc.contextSwitches /= runs;
    return acc;
}
