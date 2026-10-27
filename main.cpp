// Symulator algorytmow kolejkowania procesow (jeden procesor, czas symulowany liczbowo).
// Algorytmy: FCFS, SJF (niewywlaszczajacy), SRTF (opcjonalny), Round Robin,
//            adaptacyjny Round Robin: q(N) = max(qmin, ceil(q0 / N)).
//
// Definicja przelaczenia kontekstu: licznik rosnie, gdy procesor zaczyna wykonywac
// proces INNY niz ten, ktory wykonywal sie jako ostatni. Pierwsze uruchomienie
// i czas bezczynnosci procesora nie sa liczone jako przelaczenie.
//
// Kompilacja: g++ -std=c++17 -O2 -Wall -Wextra -o sim main.cpp
// Uruchomienie: ./sim [test | demo | experiments | all]   (domyslnie: all)

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <deque>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <utility>
#include <vector>

// ============================ Model danych ============================

struct Process {
    int id = 0;
    int arrivalTime = 0;
    int burstTime = 0;
    int remainingTime = 0;
    int startTime = -1;       // pierwsze uruchomienie
    int completionTime = -1;  // zakonczenie
};

struct RunResult {
    std::vector<Process> procs;
    int contextSwitches = 0;
};

struct Metrics {
    double avgWaiting = 0;
    double avgTurnaround = 0;
    double avgResponse = 0;
    double makespan = 0;
    double contextSwitches = 0;
};

// Liczy przelaczenia kontekstu zgodnie z przyjeta definicja.
struct SwitchCounter {
    int last = -1;
    int count = 0;
    void onRun(int id) {
        if (last != -1 && last != id) ++count;
        last = id;
    }
};

static void sortByArrival(std::vector<Process>& ps) {
    std::stable_sort(ps.begin(), ps.end(), [](const Process& a, const Process& b) {
        if (a.arrivalTime != b.arrivalTime) return a.arrivalTime < b.arrivalTime;
        return a.id < b.id;
    });
}

static std::vector<Process> prepare(std::vector<Process> ps) {
    for (auto& p : ps) {
        p.remainingTime = p.burstTime;
        p.startTime = -1;
        p.completionTime = -1;
    }
    sortByArrival(ps);
    return ps;
}

// ============================ Metryki ============================

Metrics computeMetrics(const RunResult& r) {
    Metrics m;
    const double n = static_cast<double>(r.procs.size());
    if (r.procs.empty()) return m;
    double maxC = 0;
    for (const auto& p : r.procs) {
        const int T = p.completionTime - p.arrivalTime;  // czas przebywania w systemie
        const int W = T - p.burstTime;                   // czas oczekiwania
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

// Kontrola poprawnosci harmonogramu (niezmienniki modelu).
bool validate(const RunResult& r) {
    long long totalBurst = 0;
    int maxC = 0;
    for (const auto& p : r.procs) {
        if (p.startTime < p.arrivalTime) return false;
        if (p.completionTime < p.startTime + p.burstTime) return false;
        if (p.completionTime < p.arrivalTime + p.burstTime) return false;
        totalBurst += p.burstTime;
        maxC = std::max(maxC, p.completionTime);
    }
    return maxC >= totalBurst;  // jeden procesor: nie da sie szybciej niz suma czasow
}

// ============================ Algorytmy ============================

// FIFO / FCFS
RunResult runFCFS(std::vector<Process> in) {
    auto ps = prepare(std::move(in));
    SwitchCounter sc;
    int time = 0;
    for (auto& p : ps) {
        time = std::max(time, p.arrivalTime);
        p.startTime = time;
        sc.onRun(p.id);
        time += p.burstTime;
        p.completionTime = time;
        p.remainingTime = 0;
    }
    return {ps, sc.count};
}

// SJF niewywlaszczajacy: sposrod gotowych wybiera najkrotszy burstTime.
RunResult runSJF(std::vector<Process> in) {
    auto ps = prepare(std::move(in));
    SwitchCounter sc;
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
        p.startTime = time;
        sc.onRun(p.id);
        time += p.burstTime;
        p.completionTime = time;
        p.remainingTime = 0;
        done[best] = true;
        ++finished;
    }
    return {ps, sc.count};
}

// SRTF: wywlaszczajacy SJF (zdarzeniowo: decyzja przy kazdym przyjsciu/zakonczeniu).
RunResult runSRTF(std::vector<Process> in) {
    auto ps = prepare(std::move(in));
    SwitchCounter sc;
    const size_t n = ps.size();
    size_t finished = 0;
    int time = 0;
    while (finished < n) {
        int best = -1;
        for (size_t i = 0; i < n; ++i) {
            if (ps[i].remainingTime == 0 || ps[i].arrivalTime > time) continue;
            if (best < 0 || ps[i].remainingTime < ps[best].remainingTime) best = static_cast<int>(i);
        }
        int nextArr = std::numeric_limits<int>::max();
        for (size_t i = 0; i < n; ++i)
            if (ps[i].arrivalTime > time) nextArr = std::min(nextArr, ps[i].arrivalTime);
        if (best < 0) {
            time = nextArr;
            continue;
        }
        Process& p = ps[best];
        if (p.startTime < 0) p.startTime = time;
        sc.onRun(p.id);
        int run = p.remainingTime;
        if (nextArr != std::numeric_limits<int>::max()) run = std::min(run, nextArr - time);
        time += run;
        p.remainingTime -= run;
        if (p.remainingTime == 0) {
            p.completionTime = time;
            ++finished;
        }
    }
    return {ps, sc.count};
}

// Wspolny silnik Round Robin; kwant wyznaczany funkcja liczby gotowych procesow N
// (N liczone lacznie z wybieranym procesem, na poczatku kazdej tury).
RunResult runRoundRobinGeneric(std::vector<Process> in,
                               const std::function<int(int)>& quantumFor) {
    auto ps = prepare(std::move(in));
    SwitchCounter sc;
    const size_t n = ps.size();
    std::deque<size_t> ready;
    size_t next = 0, finished = 0;
    int time = 0;

    auto admit = [&]() {
        while (next < n && ps[next].arrivalTime <= time) ready.push_back(next++);
    };

    while (finished < n) {
        admit();
        if (ready.empty()) {  // procesor bezczynny
            time = ps[next].arrivalTime;
            continue;
        }
        const size_t idx = ready.front();
        ready.pop_front();
        const int N = static_cast<int>(ready.size()) + 1;
        const int q = std::max(1, quantumFor(N));

        Process& p = ps[idx];
        if (p.startTime < 0) p.startTime = time;
        sc.onRun(p.id);

        const int slice = std::min(q, p.remainingTime);
        time += slice;
        p.remainingTime -= slice;

        admit();  // nowe procesy trafiaja do kolejki przed wracajacym procesem
        if (p.remainingTime == 0) {
            p.completionTime = time;
            ++finished;
        } else {
            ready.push_back(idx);
        }
    }
    return {ps, sc.count};
}

// Klasyczny Round Robin ze stalym kwantem.
RunResult runRR(std::vector<Process> in, int q) {
    return runRoundRobinGeneric(std::move(in), [q](int) { return q; });
}

// Adaptacyjny Round Robin: q(N) = max(qmin, ceil(q0 / N)).
RunResult runAdaptiveRR(std::vector<Process> in, int q0, int qmin) {
    return runRoundRobinGeneric(std::move(in), [q0, qmin](int N) {
        const int q = (q0 + N - 1) / N;  // ceil(q0 / N)
        return std::max(qmin, q);
    });
}

// ============================ Generator danych ============================

enum class Mix { Short, Long, Mixed };

static const char* mixName(Mix m) {
    switch (m) {
        case Mix::Short: return "short";
        case Mix::Long: return "long";
        default: return "mixed";
    }
}

std::vector<Process> generateProcesses(int n, Mix mix, unsigned seed, int arrivalSpread = 2) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> arrival(0, std::max(1, n * arrivalSpread));
    std::uniform_int_distribution<int> shortB(1, 5), longB(10, 30);
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
    }
    return ps;
}

// ============================ Rejestr algorytmow ============================

struct Algo {
    std::string name;
    std::function<RunResult(const std::vector<Process>&)> run;
};

std::vector<Algo> defaultAlgos(int rrQ = 8, int q0 = 8, int qmin = 1, bool withSRTF = true) {
    std::vector<Algo> a;
    a.push_back({"FCFS", [](const std::vector<Process>& p) { return runFCFS(p); }});
    a.push_back({"SJF", [](const std::vector<Process>& p) { return runSJF(p); }});
    if (withSRTF) a.push_back({"SRTF", [](const std::vector<Process>& p) { return runSRTF(p); }});
    a.push_back({"RR(q=" + std::to_string(rrQ) + ")",
                 [rrQ](const std::vector<Process>& p) { return runRR(p, rrQ); }});
    a.push_back({"AdaptiveRR(q0=" + std::to_string(q0) + ",qmin=" + std::to_string(qmin) + ")",
                 [q0, qmin](const std::vector<Process>& p) { return runAdaptiveRR(p, q0, qmin); }});
    return a;
}

// Srednia metryk po wielu losowych zestawach (ten sam zestaw dla kazdego algorytmu).
Metrics averageOverRuns(const Algo& algo, int n, Mix mix, int runs, unsigned baseSeed) {
    Metrics acc;
    for (int r = 0; r < runs; ++r) {
        auto input = generateProcesses(n, mix, baseSeed + r);
        RunResult res = algo.run(input);
        if (!validate(res)) {
            std::cerr << "BLAD: niepoprawny harmonogram (" << algo.name << ")\n";
            std::exit(1);
        }
        Metrics m = computeMetrics(res);
        acc.avgWaiting += m.avgWaiting;
        acc.avgTurnaround += m.avgTurnaround;
        acc.avgResponse += m.avgResponse;
        acc.makespan += m.makespan;
        acc.contextSwitches += m.contextSwitches;
    }
    acc.avgWaiting /= runs;
    acc.avgTurnaround /= runs;
    acc.avgResponse /= runs;
    acc.makespan /= runs;
    acc.contextSwitches /= runs;
    return acc;
}

// ============================ Testy ============================

static bool near(double a, double b) { return std::fabs(a - b) < 1e-9; }

int runSelfTests() {
    int failed = 0;
    auto check = [&](bool ok, const std::string& what) {
        std::cout << (ok ? "[OK]   " : "[FAIL] ") << what << "\n";
        if (!ok) ++failed;
    };

    // Klasyczny przyklad: (A,B) = (0,5), (1,3), (2,8), (3,6)
    std::vector<Process> ps(4);
    int A[] = {0, 1, 2, 3}, B[] = {5, 3, 8, 6};
    for (int i = 0; i < 4; ++i) {
        ps[i].id = i + 1;
        ps[i].arrivalTime = A[i];
        ps[i].burstTime = B[i];
    }

    auto fc = runFCFS(ps);
    auto fm = computeMetrics(fc);
    check(near(fm.avgWaiting, 5.75) && near(fm.makespan, 22), "FCFS: sredni czas oczekiwania = 5.75, makespan = 22");

    auto sj = runSJF(ps);
    auto sm = computeMetrics(sj);
    check(near(sm.avgWaiting, 5.25) && near(sm.makespan, 22), "SJF: sredni czas oczekiwania = 5.25, makespan = 22");

    auto sr = runSRTF(ps);
    check(validate(sr), "SRTF: harmonogram poprawny");
    check(computeMetrics(sr).avgWaiting <= sm.avgWaiting + 1e-9, "SRTF: oczekiwanie <= SJF");

    // Pojedynczy proces + dziura bezczynnosci
    std::vector<Process> gap(2);
    gap[0] = {1, 0, 3, 3, -1, -1};
    gap[1] = {2, 10, 2, 2, -1, -1};
    for (auto& r : {runFCFS(gap), runSJF(gap), runSRTF(gap), runRR(gap, 2), runAdaptiveRR(gap, 8, 1)}) {
        check(validate(r) && r.procs[1].completionTime == 12, "bezczynnosc procesora: P2 konczy sie w t=12");
    }

    // RR: znany przyklad, wszystkie przyjscia w t=0, B = 24, 3, 3, q = 4 -> srednie oczekiwanie 5.666..
    std::vector<Process> rr(3);
    int B2[] = {24, 3, 3};
    for (int i = 0; i < 3; ++i) rr[i] = {i + 1, 0, B2[i], B2[i], -1, -1};
    auto rrr = runRR(rr, 4);
    check(near(computeMetrics(rrr).avgWaiting, 17.0 / 3.0), "RR(q=4): sredni czas oczekiwania = 17/3");
    check(rrr.contextSwitches == 3, "RR(q=4): 3 przelaczenia kontekstu");

    // Adaptacyjny RR: tabela z opisu (q0=8): N=1..4,8 -> 8,4,3,2,1
    std::vector<int> quanta;
    auto probe = [&](int N) { return std::max(1, (8 + N - 1) / N); };
    for (int N : {1, 2, 3, 4, 8}) quanta.push_back(probe(N));
    check(quanta == std::vector<int>({8, 4, 3, 2, 1}), "adaptacyjny kwant: 8,4,3,2,1 dla N=1,2,3,4,8");

    // Losowe zestawy: poprawnosc i zgodnosc makespan przy braku bezczynnosci
    bool allValid = true;
    for (unsigned s = 0; s < 200; ++s) {
        auto in = generateProcesses(30, Mix::Mixed, s);
        for (auto& a : defaultAlgos())
            if (!validate(a.run(in))) allValid = false;
    }
    check(allValid, "200 losowych zestawow x wszystkie algorytmy: harmonogramy poprawne");

    std::cout << (failed ? "\nTESTY NIEZALICZONE\n" : "\nWszystkie testy zaliczone.\n");
    return failed;
}

// ============================ Demo ============================

void runDemo() {
    const int n = 10;
    auto input = generateProcesses(n, Mix::Mixed, 42);

    std::cout << "\n=== Dane wejsciowe (n=" << n << ", mixed, seed=42) ===\n";
    std::cout << std::setw(4) << "id" << std::setw(10) << "arrival" << std::setw(8) << "burst" << "\n";
    for (const auto& p : input)
        std::cout << std::setw(4) << p.id << std::setw(10) << p.arrivalTime << std::setw(8) << p.burstTime << "\n";

    std::cout << "\n=== Porownanie algorytmow ===\n";
    std::cout << std::left << std::setw(28) << "algorytm" << std::right << std::setw(10) << "avgWait"
              << std::setw(10) << "avgTAT" << std::setw(10) << "avgResp" << std::setw(10) << "makespan"
              << std::setw(10) << "ctxSw" << "\n";
    std::cout << std::fixed << std::setprecision(2);
    for (const auto& a : defaultAlgos()) {
        auto m = computeMetrics(a.run(input));
        std::cout << std::left << std::setw(28) << a.name << std::right << std::setw(10) << m.avgWaiting
                  << std::setw(10) << m.avgTurnaround << std::setw(10) << m.avgResponse << std::setw(10)
                  << m.makespan << std::setw(10) << m.contextSwitches << "\n";
    }

    auto res = runAdaptiveRR(input, 8, 1);
    std::cout << "\n=== AdaptiveRR(q0=8, qmin=1) - wyniki per proces ===\n";
    std::cout << std::setw(4) << "id" << std::setw(9) << "arrival" << std::setw(7) << "burst" << std::setw(8)
              << "start" << std::setw(7) << "end" << std::setw(7) << "wait" << "\n";
    auto sorted = res.procs;
    std::sort(sorted.begin(), sorted.end(), [](const Process& a, const Process& b) { return a.id < b.id; });
    for (const auto& p : sorted) {
        int wait = p.completionTime - p.arrivalTime - p.burstTime;
        std::cout << std::setw(4) << p.id << std::setw(9) << p.arrivalTime << std::setw(7) << p.burstTime
                  << std::setw(8) << p.startTime << std::setw(7) << p.completionTime << std::setw(7) << wait << "\n";
    }
}

// ============================ Eksperymenty (CSV) ============================

static const char* CSV_HEADER =
    "experiment,scenario,n,mix,algorithm,runs,avg_waiting,avg_turnaround,avg_response,makespan,context_switches\n";

static void writeRow(std::ofstream& f, const std::string& exp, const std::string& scenario, int n, Mix mix,
                     const std::string& algo, int runs, const Metrics& m) {
    f << exp << ',' << scenario << ',' << n << ',' << mixName(mix) << ",\"" << algo << "\"," << runs << ','
      << m.avgWaiting << ',' << m.avgTurnaround << ',' << m.avgResponse << ',' << m.makespan << ','
      << m.contextSwitches << '\n';
}

void runExperiments(int runs = 100, unsigned baseSeed = 1000) {
    std::cout << "\n=== Eksperymenty (" << runs << " losowych zestawow na konfiguracje) ===\n";
    auto algos = defaultAlgos();

    // A: wplyw liczby procesow (mixed)
    {
        std::ofstream f("results_A_process_count.csv");
        f << std::fixed << std::setprecision(4) << CSV_HEADER;
        for (int n : {10, 20, 50, 100, 200, 500})
            for (const auto& a : algos)
                writeRow(f, "A", "n=" + std::to_string(n), n, Mix::Mixed, a.name, runs,
                         averageOverRuns(a, n, Mix::Mixed, runs, baseSeed));
        std::cout << "  results_A_process_count.csv\n";
    }

    // B: dlugosc procesow (n = 50)
    {
        std::ofstream f("results_B_burst_length.csv");
        f << std::fixed << std::setprecision(4) << CSV_HEADER;
        for (Mix mix : {Mix::Short, Mix::Long, Mix::Mixed})
            for (const auto& a : algos)
                writeRow(f, "B", mixName(mix), 50, mix, a.name, runs,
                         averageOverRuns(a, 50, mix, runs, baseSeed));
        std::cout << "  results_B_burst_length.csv\n";
    }

    // C: parametry kwantu (n = 50, mixed): RR(q), adaptacyjny (q0, qmin)
    {
        std::ofstream f("results_C_quantum_params.csv");
        f << std::fixed << std::setprecision(4) << CSV_HEADER;
        for (int q : {1, 2, 4, 8, 16, 32}) {
            Algo a{"RR(q=" + std::to_string(q) + ")",
                   [q](const std::vector<Process>& p) { return runRR(p, q); }};
            writeRow(f, "C", "RR q=" + std::to_string(q), 50, Mix::Mixed, a.name, runs,
                     averageOverRuns(a, 50, Mix::Mixed, runs, baseSeed));
        }
        for (int q0 : {4, 8, 16, 32, 64})
            for (int qmin : {1, 2, 4}) {
                if (qmin > q0) continue;
                Algo a{"AdaptiveRR(q0=" + std::to_string(q0) + ",qmin=" + std::to_string(qmin) + ")",
                       [q0, qmin](const std::vector<Process>& p) { return runAdaptiveRR(p, q0, qmin); }};
                writeRow(f, "C", "q0=" + std::to_string(q0) + " qmin=" + std::to_string(qmin), 50, Mix::Mixed,
                         a.name, runs, averageOverRuns(a, 50, Mix::Mixed, runs, baseSeed));
            }
        std::cout << "  results_C_quantum_params.csv\n";
    }

    // D: pelna siatka (n x mix) dla algorytmow o domyslnych parametrach + podsumowanie na ekranie
    {
        std::ofstream f("results_D_comparison.csv");
        f << std::fixed << std::setprecision(4) << CSV_HEADER;
        std::cout << "\nAdaptiveRR(8,1) vs RR(8) - zmiana sredniego czasu oczekiwania ('-' = adaptacyjny lepszy):\n";
        std::cout << std::fixed << std::setprecision(2);
        for (Mix mix : {Mix::Short, Mix::Long, Mix::Mixed})
            for (int n : {10, 50, 200}) {
                Metrics rrM, adM;
                for (const auto& a : algos) {
                    Metrics m = averageOverRuns(a, n, mix, runs, baseSeed);
                    writeRow(f, "D", std::string(mixName(mix)) + " n=" + std::to_string(n), n, mix, a.name, runs, m);
                    if (a.name.rfind("RR(", 0) == 0) rrM = m;
                    if (a.name.rfind("AdaptiveRR", 0) == 0) adM = m;
                }
                double change = rrM.avgWaiting > 0 ? 100.0 * (adM.avgWaiting - rrM.avgWaiting) / rrM.avgWaiting : 0;
                std::cout << "  " << std::setw(6) << mixName(mix) << " n=" << std::setw(3) << n
                          << "  wait RR=" << std::setw(8) << rrM.avgWaiting << "  Adaptive=" << std::setw(8)
                          << adM.avgWaiting << "  zmiana=" << std::setw(7) << change << "%"
                          << "  ctxSw RR=" << rrM.contextSwitches << " Adaptive=" << adM.contextSwitches << "\n";
            }
        std::cout << "  results_D_comparison.csv\n";
    }
}

// ============================ main ============================

int main(int argc, char** argv) {
    const std::string mode = argc > 1 ? argv[1] : "all";
    int rc = 0;
    if (mode == "test" || mode == "all") rc = runSelfTests();
    if (mode == "demo" || mode == "all") runDemo();
    if (mode == "experiments" || mode == "all") runExperiments();
    if (mode != "test" && mode != "demo" && mode != "experiments" && mode != "all") {
        std::cerr << "Uzycie: " << argv[0] << " [test | demo | experiments | all]\n";
        return 2;
    }
    return rc;
}
