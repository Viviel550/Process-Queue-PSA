// Symulator algorytmow kolejkowania procesow (jeden procesor, czas symulowany liczbowo).
// Algorytmy: FCFS, SJF, SRTF, priorytetowy (kolejka priorytetowa), Round Robin, adaptacyjny Round Robin.
// Szczegoly: README.md.

#include <iostream>
#include <string>

#include "app/cli.h"
#include "app/demo.h"
#include "app/self_tests.h"
#include "app/thread_benchmark.h"
#include "experiments/experiments.h"

int main(int argc, char** argv) {
    CliOptions opt;
    std::string error;
    if (!parseArgs(argc, argv, opt, error)) {
        std::cerr << error << "\n\n";
        printUsage(argv[0]);
        return 2;
    }
    if (opt.help) {
        printUsage(argv[0]);
        return 0;
    }

    std::cout << "Watki: " << opt.exp.threads << ", kara przelaczenia kontekstu: " << opt.exp.sim.contextSwitchCost
              << ", zestawow na konfiguracje: " << opt.exp.runs << "\n";

    int rc = 0;
    try {
        const bool all = opt.mode == "all";
        if (all || opt.mode == "test") rc = runSelfTests(opt.exp);
        if (all || opt.mode == "demo") runDemo(opt.exp);
        if (all || opt.mode == "experiments") runAllExperiments(opt.exp);
        if (opt.mode == "threadtest") rc = runThreadBenchmark(opt.maxThreads, opt.exp);
    } catch (const std::exception& e) {
        std::cerr << "BLAD: " << e.what() << "\n";
        return 1;
    }
    return rc;
}
