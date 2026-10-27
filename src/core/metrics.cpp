#include "core/metrics.h"

#include <algorithm>

Metrics computeMetrics(const RunResult& r) {
    Metrics m;
    const double n = static_cast<double>(r.procs.size());
    if (r.procs.empty()) return m;
    double maxC = 0;
    for (const auto& p : r.procs) {
        const int T = p.completionTime - p.arrivalTime;  // czas przebywania w systemie
        const int W = T - p.burstTime;                   // czas oczekiwania (zawiera kary przelaczen)
        const int R = p.startTime - p.arrivalTime;       // czas odpowiedzi
        m.avgTurnaround += T;
        m.avgWaiting += W;
        m.avgResponse += R;
        maxC = std::max<double>(maxC, p.completionTime);
    }
    m.avgTurnaround /= n;
    m.avgWaiting /= n;
    m.avgResponse /= n;
    m.makespan = maxC;
    m.contextSwitches = r.contextSwitches;
    return m;
}

bool sameMetrics(const Metrics& a, const Metrics& b) {
    return a.avgWaiting == b.avgWaiting && a.avgTurnaround == b.avgTurnaround &&
           a.avgResponse == b.avgResponse && a.makespan == b.makespan &&
           a.contextSwitches == b.contextSwitches;
}
