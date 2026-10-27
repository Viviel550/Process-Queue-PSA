#include "core/process_utils.h"

#include <algorithm>
#include <utility>

void sortByArrival(std::vector<Process>& ps) {
    std::stable_sort(ps.begin(), ps.end(), [](const Process& a, const Process& b) {
        if (a.arrivalTime != b.arrivalTime) return a.arrivalTime < b.arrivalTime;
        return a.id < b.id;
    });
}

std::vector<Process> prepare(std::vector<Process> ps) {
    for (auto& p : ps) {
        p.remainingTime = p.burstTime;
        p.startTime = -1;
        p.completionTime = -1;
    }
    sortByArrival(ps);
    return ps;
}
