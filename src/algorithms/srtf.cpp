#include "algorithms/srtf.h"

#include <algorithm>
#include <limits>
#include <utility>

#include "core/context_switch.h"
#include "core/process_utils.h"

namespace {

constexpr int kNone = std::numeric_limits<int>::max();

// Indeks gotowego procesu o najmniejszym remainingTime (remis: pierwszy w kolejnosci przyjscia).
int pickShortest(const std::vector<Process>& ps, int time) {
    int best = -1;
    for (size_t i = 0; i < ps.size(); ++i) {
        if (ps[i].remainingTime == 0 || ps[i].arrivalTime > time) continue;
        if (best < 0 || ps[i].remainingTime < ps[best].remainingTime) best = static_cast<int>(i);
    }
    return best;
}

// Najblizszy czas przyjscia scisle po `time` (kNone, jesli nikt juz nie przyjdzie).
int nextArrivalAfter(const std::vector<Process>& ps, int time) {
    int next = kNone;
    for (const auto& p : ps)
        if (p.arrivalTime > time) next = std::min(next, p.arrivalTime);
    return next;
}

}  // namespace

RunResult runSRTF(std::vector<Process> in, const SimConfig& cfg) {
    auto ps = prepare(std::move(in));
    ContextSwitchTracker sw(cfg.contextSwitchCost);
    const size_t n = ps.size();
    size_t finished = 0;
    int time = 0;
    while (finished < n) {
        const int best = pickShortest(ps, time);
        if (best < 0) {  // procesor bezczynny do najblizszego przyjscia
            time = nextArrivalAfter(ps, time);
            continue;
        }
        Process& p = ps[best];
        time = sw.onRun(p.id, time);
        if (pickShortest(ps, time) != best) continue;  // w trakcie przelaczenia przyszedl lepszy proces

        if (p.startTime < 0) p.startTime = time;
        int run = p.remainingTime;
        const int nextArr = nextArrivalAfter(ps, time);
        if (nextArr != kNone) run = std::min(run, nextArr - time);
        time += run;
        p.remainingTime -= run;
        if (p.remainingTime == 0) {
            p.completionTime = time;
            ++finished;
        }
    }
    return sw.finish(std::move(ps));
}
