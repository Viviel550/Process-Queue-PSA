#include "algorithms/round_robin_engine.h"

#include <algorithm>
#include <deque>
#include <utility>

#include "core/context_switch.h"
#include "core/process_utils.h"

RunResult runRoundRobinGeneric(std::vector<Process> in, const std::function<int(int)>& quantumFor,
                               const SimConfig& cfg) {
    auto ps = prepare(std::move(in));
    ContextSwitchTracker sw(cfg.contextSwitchCost);
    const size_t n = ps.size();
    std::deque<size_t> ready;
    size_t next = 0, finished = 0;
    int time = 0;

    auto admit = [&]() {
        while (next < n && ps[next].arrivalTime <= time) ready.push_back(next++);
    };

    while (finished < n) {
        admit();
        if (ready.empty()) {  // procesor bezczynny
            time = ps[next].arrivalTime;
            continue;
        }
        const size_t idx = ready.front();
        ready.pop_front();
        const int N = static_cast<int>(ready.size()) + 1;
        const int q = std::max(1, quantumFor(N));

        Process& p = ps[idx];
        time = sw.onRun(p.id, time);  // ewentualna kara za przelaczenie
        if (p.startTime < 0) p.startTime = time;

        const int slice = std::min(q, p.remainingTime);
        time += slice;
        p.remainingTime -= slice;

        admit();  // nowe procesy (takze te z czasu kary) trafiaja do kolejki przed wracajacym procesem
        if (p.remainingTime == 0) {
            p.completionTime = time;
            ++finished;
        } else {
            ready.push_back(idx);
        }
    }
    return sw.finish(std::move(ps));
}
