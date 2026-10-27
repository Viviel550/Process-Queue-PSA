#pragma once
#include "core/process.h"

struct Metrics {
    double avgWaiting = 0;
    double avgTurnaround = 0;
    double avgResponse = 0;
    double makespan = 0;
    double contextSwitches = 0;
};

// T = C - A, W = T - B, R = S - A; makespan = najwieksze C.
Metrics computeMetrics(const RunResult& r);

// Dokladne porownanie (wyniki sa deterministyczne, wiec bez tolerancji).
bool sameMetrics(const Metrics& a, const Metrics& b);
