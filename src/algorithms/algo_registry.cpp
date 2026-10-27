#include "algorithms/algo_registry.h"

#include "algorithms/adaptive_round_robin.h"
#include "algorithms/fcfs.h"
#include "algorithms/priority.h"
#include "algorithms/round_robin.h"
#include "algorithms/sjf.h"
#include "algorithms/srtf.h"

Algo makeRR(int q, const SimConfig& sim) {
    return {"RR(q=" + std::to_string(q) + ")",
            [q, sim](const std::vector<Process>& p) { return runRR(p, q, sim); }};
}

Algo makeAdaptiveRR(int q0, int qmin, const SimConfig& sim) {
    return {"AdaptiveRR(q0=" + std::to_string(q0) + ",qmin=" + std::to_string(qmin) + ")",
            [q0, qmin, sim](const std::vector<Process>& p) { return runAdaptiveRR(p, q0, qmin, sim); }};
}

std::vector<Algo> defaultAlgos(const SimConfig& sim, int rrQ, int q0, int qmin, bool withSRTF,
                               bool withPriority) {
    std::vector<Algo> a;
    a.push_back({"FCFS", [sim](const std::vector<Process>& p) { return runFCFS(p, sim); }});
    a.push_back({"SJF", [sim](const std::vector<Process>& p) { return runSJF(p, sim); }});
    if (withSRTF) a.push_back({"SRTF", [sim](const std::vector<Process>& p) { return runSRTF(p, sim); }});
    if (withPriority) {
        a.push_back({"Priority(np)", [sim](const std::vector<Process>& p) { return runPriority(p, false, sim); }});
        a.push_back({"Priority(p)", [sim](const std::vector<Process>& p) { return runPriority(p, true, sim); }});
    }
    a.push_back(makeRR(rrQ, sim));
    a.push_back(makeAdaptiveRR(q0, qmin, sim));
    return a;
}
