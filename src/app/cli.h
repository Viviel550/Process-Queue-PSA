#pragma once
#include <string>

#include "experiments/experiment_config.h"

struct CliOptions {
    std::string mode = "all";  // test | demo | experiments | threadtest | all
    ExperimentConfig exp;      // runs, seed, threads, kara przelaczenia
    int maxThreads = 0;        // gorna granica dla trybu threadtest (0 = liczba rdzeni)
    bool help = false;
};

// Zwraca false i wypelnia `error` przy blednych argumentach.
bool parseArgs(int argc, char** argv, CliOptions& out, std::string& error);

void printUsage(const char* prog);
