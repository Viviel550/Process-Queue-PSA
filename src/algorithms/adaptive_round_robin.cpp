#include "algorithms/adaptive_round_robin.h"

#include <algorithm>
#include <utility>

#include "algorithms/round_robin_engine.h"

int adaptiveQuantum(int q0, int qmin, int N) {
    if (N < 1) N = 1;
    const int q = (q0 + N - 1) / N;  // ceil(q0 / N)
    return std::max(qmin, q);
}

RunResult runAdaptiveRR(std::vector<Process> in, int q0, int qmin, const SimConfig& cfg) {
    return runRoundRobinGeneric(std::move(in), [q0, qmin](int N) { return adaptiveQuantum(q0, qmin, N); }, cfg);
}
