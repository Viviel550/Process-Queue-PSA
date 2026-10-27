#include "algorithms/sjf.h"

#include <algorithm>
#include <limits>
#include <utility>

#include "core/context_switch.h"
#include "core/process_utils.h"

RunResult runSJF(std::vector<Process> in, const SimConfig& cfg) {
    auto ps = prepare(std::move(in));
    ContextSwitchTracker sw(cfg.contextSwitchCost);
    const size_t n = ps.size();
    std::vector<bool> done(n, false);
    size_t finished = 0;
    int time = 0;
    while (finished < n) {
        int best = -1;
        for (size_t i = 0; i < n; ++i) {
            if (done[i] || ps[i].arrivalTime > time) continue;
            if (best < 0 || ps[i].burstTime < ps[best].burstTime) best = static_cast<int>(i);
            // remis: ps jest posortowane wg (arrival, id), wiec pierwszy wygrywa
        }
        if (best < 0) {  // brak gotowych -> procesor bezczynny do najblizszego przyjscia
            int nextArr = std::numeric_limits<int>::max();
            for (size_t i = 0; i < n; ++i)
                if (!done[i]) nextArr = std::min(nextArr, ps[i].arrivalTime);
            time = nextArr;
            continue;
        }
        Process& p = ps[best];
        time = sw.onRun(p.id, time);
        p.startTime = time;
        time += p.burstTime;
        p.completionTime = time;
        p.remainingTime = 0;
        done[best] = true;
        ++finished;
    }
    return sw.finish(std::move(ps));
}
