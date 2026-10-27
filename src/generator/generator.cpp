#include "generator/generator.h"

#include <algorithm>
#include <random>

const char* mixName(Mix m) {
    switch (m) {
        case Mix::Short: return "short";
        case Mix::Long: return "long";
        default: return "mixed";
    }
}

std::vector<Process> generateProcesses(int n, Mix mix, unsigned seed, int arrivalSpread) {
    std::mt19937 rng(seed);
    std::mt19937 prioRng(seed ^ 0x9E3779B9u);
    std::uniform_int_distribution<int> arrival(0, std::max(1, n * arrivalSpread));
    std::uniform_int_distribution<int> shortB(1, 5), longB(10, 30);
    std::uniform_int_distribution<int> prio(1, kMaxPriority);
    std::uniform_real_distribution<double> coin(0.0, 1.0);

    std::vector<Process> ps(n);
    for (int i = 0; i < n; ++i) {
        ps[i].id = i + 1;
        ps[i].arrivalTime = arrival(rng);
        switch (mix) {
            case Mix::Short: ps[i].burstTime = shortB(rng); break;
            case Mix::Long: ps[i].burstTime = longB(rng); break;
            case Mix::Mixed: ps[i].burstTime = (coin(rng) < 0.7) ? shortB(rng) : longB(rng); break;
        }
        ps[i].remainingTime = ps[i].burstTime;
        ps[i].priority = prio(prioRng);
    }
    return ps;
}
