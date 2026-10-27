#include "app/self_tests.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "algorithms/adaptive_round_robin.h"
#include "algorithms/algo_registry.h"
#include "algorithms/fcfs.h"
#include "algorithms/priority.h"
#include "algorithms/round_robin.h"
#include "algorithms/sjf.h"
#include "algorithms/srtf.h"
#include "core/metrics.h"
#include "core/validate.h"
#include "experiments/average_runs.h"
#include "generator/generator.h"
#include "parallel/parallel_for.h"

namespace {

bool near(double a, double b) { return std::fabs(a - b) < 1e-9; }

const Process* findById(const RunResult& r, int id) {
    for (const auto& p : r.procs)
        if (p.id == id) return &p;
    return nullptr;
}

int completionOf(const RunResult& r, int id) {
    const Process* p = findById(r, id);
    return p ? p->completionTime : -1;
}

}  // namespace

int runSelfTests(const ExperimentConfig& cfg) {
    int failed = 0;
    auto check = [&](bool ok, const std::string& what) {
        std::cout << (ok ? "[OK]   " : "[FAIL] ") << what << "\n";
        if (!ok) ++failed;
    };
    std::cout << "\n=== Testy poprawnosci (watki: " << cfg.threads << ") ===\n";

    // ---- Model idealny (kara = 0): klasyczny przyklad (A,B) = (0,5), (1,3), (2,8), (3,6) ----
    std::vector<Process> ps(4);
    const int A[] = {0, 1, 2, 3}, B[] = {5, 3, 8, 6};
    for (int i = 0; i < 4; ++i) {
        ps[i].id = i + 1;
        ps[i].arrivalTime = A[i];
        ps[i].burstTime = B[i];
    }

    const auto fc = runFCFS(ps);
    const auto fm = computeMetrics(fc);
    check(near(fm.avgWaiting, 5.75) && near(fm.makespan, 22), "FCFS: sredni czas oczekiwania = 5.75, makespan = 22");

    const auto sj = runSJF(ps);
    const auto sm = computeMetrics(sj);
    check(near(sm.avgWaiting, 5.25) && near(sm.makespan, 22), "SJF: sredni czas oczekiwania = 5.25, makespan = 22");

    const auto sr = runSRTF(ps);
    check(validate(sr), "SRTF: harmonogram poprawny");
    check(computeMetrics(sr).avgWaiting <= sm.avgWaiting + 1e-9, "SRTF: oczekiwanie <= SJF");

    // Bezczynnosc procesora
    std::vector<Process> gap(2);
    gap[0] = {1, 0, 3, 3, -1, -1, 0};
    gap[1] = {2, 10, 2, 2, -1, -1, 0};
    for (const auto& r : {runFCFS(gap), runSJF(gap), runSRTF(gap), runPriority(gap, false), runPriority(gap, true),
                          runRR(gap, 2), runAdaptiveRR(gap, 8, 1)}) {
        check(validate(r) && r.procs[1].completionTime == 12, "bezczynnosc procesora: P2 konczy sie w t=12");
    }

    // RR: B = 24, 3, 3, q = 4, wszystkie przyjscia w t=0 -> srednie oczekiwanie 17/3
    std::vector<Process> rr(3);
    const int B2[] = {24, 3, 3};
    for (int i = 0; i < 3; ++i) rr[i] = {i + 1, 0, B2[i], B2[i], -1, -1, 0};
    const auto rrr = runRR(rr, 4);
    check(near(computeMetrics(rrr).avgWaiting, 17.0 / 3.0), "RR(q=4): sredni czas oczekiwania = 17/3");
    check(rrr.contextSwitches == 3, "RR(q=4): 3 przelaczenia kontekstu");

    // Adaptacyjny kwant: q0=8 -> N=1,2,3,4,8 daje 8,4,3,2,1
    std::vector<int> quanta;
    for (int N : {1, 2, 3, 4, 8}) quanta.push_back(adaptiveQuantum(8, 1, N));
    check(quanta == std::vector<int>({8, 4, 3, 2, 1}), "adaptacyjny kwant: 8,4,3,2,1 dla N=1,2,3,4,8");

    // ---- Kolejka priorytetowa ----
    std::vector<Process> pr(3);  // wszystkie w t=0; (id, burst, priorytet): (1,5,3) (2,2,1) (3,4,2)
    pr[0] = {1, 0, 5, 5, -1, -1, 3};
    pr[1] = {2, 0, 2, 2, -1, -1, 1};
    pr[2] = {3, 0, 4, 4, -1, -1, 2};
    const auto prr = runPriority(pr, false);
    check(completionOf(prr, 2) == 2 && completionOf(prr, 3) == 6 && completionOf(prr, 1) == 11,
          "Priority(np): kolejnosc wg priorytetu 2 -> 3 -> 1 (zakonczenia 2, 6, 11)");

    std::vector<Process> pre(2);  // P1 (A=0,B=6,prio 2), P2 (A=2,B=2,prio 1)
    pre[0] = {1, 0, 6, 6, -1, -1, 2};
    pre[1] = {2, 2, 2, 2, -1, -1, 1};
    const auto pnp = runPriority(pre, false);
    const auto pp = runPriority(pre, true);
    check(completionOf(pnp, 1) == 6 && completionOf(pnp, 2) == 8, "Priority(np): P2 czeka na koniec P1 (6, 8)");
    check(completionOf(pp, 2) == 4 && completionOf(pp, 1) == 8 && pp.contextSwitches == 2,
          "Priority(p): P2 wywlaszcza P1 (zakonczenia 4, 8; 2 przelaczenia)");
    {
        const auto eq = runPriority(ps, false);  // ps ma wszedzie priorytet 0 -> remis -> jak FCFS
        bool same = true;
        for (const auto& p : fc.procs) same = same && completionOf(eq, p.id) == p.completionTime;
        check(same, "Priority(np): przy rownych priorytetach zachowuje sie jak FCFS");
    }

    // ---- Kara za przelaczenie kontekstu ----
    SimConfig cost1, cost2;
    cost1.contextSwitchCost = 1;
    cost2.contextSwitchCost = 2;

    const auto fcc = runFCFS(ps, cost2);  // 3 przelaczenia * 2 = 6
    check(fcc.contextSwitches == 3 && fcc.overheadTime == 6 && near(computeMetrics(fcc).makespan, 28) &&
              near(computeMetrics(fcc).avgWaiting, 8.75) && validate(fcc),
          "FCFS, kara 2: makespan = 22 + 3*2 = 28, srednie oczekiwanie 8.75");

    const auto rrc = runRR(rr, 4, cost1);  // 3 przelaczenia * 1; P1 konczy w t=33
    check(rrc.contextSwitches == 3 && rrc.overheadTime == 3 && near(computeMetrics(rrc).makespan, 33) &&
              near(computeMetrics(rrc).avgWaiting, 23.0 / 3.0) && validate(rrc),
          "RR(q=4), kara 1: makespan = 30 + 3 = 33, srednie oczekiwanie 23/3");

    const auto ppc = runPriority(pre, true, cost1);  // P1 0-2, P2 3-5, P1 6-10
    check(completionOf(ppc, 2) == 5 && completionOf(ppc, 1) == 10 && validate(ppc),
          "Priority(p), kara 1: P2 konczy w t=5, P1 w t=10");

    {
        bool zeroSame = true;  // kara 0 nie zmienia niczego
        const SimConfig zero;
        zeroSame = zeroSame && sameMetrics(computeMetrics(runAdaptiveRR(ps, 8, 1)),
                                           computeMetrics(runAdaptiveRR(ps, 8, 1, zero)));
        check(zeroSame, "kara 0 == model idealny");
    }

    // ---- Losowe zestawy x wszystkie algorytmy (rownolegle), z kara 0 i 2 ----
    for (int cost : {0, 2}) {
        SimConfig sim;
        sim.contextSwitchCost = cost;
        const auto algos = defaultAlgos(sim);
        std::vector<char> ok(200, 1);
        parallelFor(ok.size(), cfg.threads, [&](std::size_t s) {
            const auto in = generateProcesses(30, Mix::Mixed, static_cast<unsigned>(s));
            for (const auto& a : algos)
                if (!validate(a.run(in))) ok[s] = 0;
        });
        check(std::all_of(ok.begin(), ok.end(), [](char c) { return c == 1; }),
              "200 losowych zestawow x wszystkie algorytmy (kara " + std::to_string(cost) + "): harmonogramy poprawne");
    }

    // ---- Wielowatkowosc: wynik nie zalezy od liczby watkow ----
    {
        ExperimentConfig one = cfg, many = cfg;
        one.runs = many.runs = 40;
        one.threads = 1;
        many.threads = std::max(4, cfg.threads);
        bool same = true;
        for (const auto& a : defaultAlgos(cfg.sim))
            same = same && sameMetrics(averageOverRuns(a, 50, Mix::Mixed, one),
                                       averageOverRuns(a, 50, Mix::Mixed, many));
        check(same, "wyniki identyczne dla 1 i " + std::to_string(many.threads) + " watkow");
    }

    std::cout << (failed ? "\nTESTY NIEZALICZONE\n" : "\nWszystkie testy zaliczone.\n");
    return failed;
}
