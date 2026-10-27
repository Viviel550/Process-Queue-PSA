#include "algorithms/round_robin.h"

#include <utility>

#include "algorithms/round_robin_engine.h"

RunResult runRR(std::vector<Process> in, int q, const SimConfig& cfg) {
    return runRoundRobinGeneric(std::move(in), [q](int) { return q; }, cfg);
}
