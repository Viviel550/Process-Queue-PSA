#include "core/validate.h"

#include <algorithm>

bool validate(const RunResult& r) {
    if (r.overheadTime < 0) return false;
    long long totalBurst = 0;
    int maxC = 0;
    for (const auto& p : r.procs) {
        if (p.startTime < p.arrivalTime) return false;
        if (p.completionTime < p.startTime + p.burstTime) return false;
        if (p.completionTime < p.arrivalTime + p.burstTime) return false;
        totalBurst += p.burstTime;
        maxC = std::max(maxC, p.completionTime);
    }
    return maxC >= totalBurst + r.overheadTime;
}
