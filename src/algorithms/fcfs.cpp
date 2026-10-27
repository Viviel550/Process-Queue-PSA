#include "algorithms/fcfs.h"

#include <algorithm>
#include <utility>

#include "core/context_switch.h"
#include "core/process_utils.h"

RunResult runFCFS(std::vector<Process> in, const SimConfig& cfg) {
    auto ps = prepare(std::move(in));
    ContextSwitchTracker sw(cfg.contextSwitchCost);
    int time = 0;
    for (auto& p : ps) {
        time = std::max(time, p.arrivalTime);
        time = sw.onRun(p.id, time);  // ewentualna kara za przelaczenie
        p.startTime = time;
        time += p.burstTime;
        p.completionTime = time;
        p.remainingTime = 0;
    }
    return sw.finish(std::move(ps));
}
