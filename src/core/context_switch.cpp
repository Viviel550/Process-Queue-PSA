#include "core/context_switch.h"

#include <utility>

int ContextSwitchTracker::onRun(int id, int time) {
    if (last_ != -1 && last_ != id) {
        ++count_;
        time += cost_;
    }
    last_ = id;
    return time;
}

RunResult ContextSwitchTracker::finish(std::vector<Process> procs) const {
    RunResult r;
    r.procs = std::move(procs);
    r.contextSwitches = count_;
    r.overheadTime = count_ * cost_;
    return r;
}
