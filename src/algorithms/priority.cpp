#include "algorithms/priority.h"

#include <algorithm>
#include <queue>
#include <utility>

#include "core/context_switch.h"
#include "core/process_utils.h"

namespace {

// Porzadek kolejki: top() to proces o najwyzszym priorytecie.
// operator() zwraca true, gdy `a` ma NIZSZY priorytet niz `b`.
struct LowerPrecedence {
    const std::vector<Process>* ps;
    bool operator()(size_t a, size_t b) const {
        const Process& x = (*ps)[a];
        const Process& y = (*ps)[b];
        if (x.priority != y.priority) return x.priority > y.priority;
        if (x.arrivalTime != y.arrivalTime) return x.arrivalTime > y.arrivalTime;
        return x.id > y.id;
    }
};

}  // namespace

RunResult runPriority(std::vector<Process> in, bool preemptive, const SimConfig& cfg) {
    auto ps = prepare(std::move(in));
    ContextSwitchTracker sw(cfg.contextSwitchCost);
    const size_t n = ps.size();
    std::priority_queue<size_t, std::vector<size_t>, LowerPrecedence> ready{LowerPrecedence{&ps}};
    size_t next = 0, finished = 0;
    int time = 0;

    auto admit = [&]() {
        while (next < n && ps[next].arrivalTime <= time) ready.push(next++);
    };

    while (finished < n) {
        admit();
        if (ready.empty()) {  // procesor bezczynny
            time = ps[next].arrivalTime;
            continue;
        }
        const size_t idx = ready.top();
        if (!preemptive) ready.pop();  // niewywlaszczany: dziala do konca, wiec od razu opuszcza kolejke

        Process& p = ps[idx];
        time = sw.onRun(p.id, time);
        admit();  // przyjscia w trakcie przelaczenia kontekstu
        if (preemptive && ready.top() != idx) continue;  // w trakcie przelaczenia przyszedl wazniejszy proces

        if (p.startTime < 0) p.startTime = time;
        int run = p.remainingTime;
        if (preemptive && next < n) run = std::min(run, ps[next].arrivalTime - time);
        time += run;
        p.remainingTime -= run;
        if (p.remainingTime == 0) {
            p.completionTime = time;
            if (preemptive) ready.pop();
            ++finished;
        }
    }
    return sw.finish(std::move(ps));
}
