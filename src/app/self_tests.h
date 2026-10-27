#pragma once
#include "experiments/experiment_config.h"

// Zwraca liczbe niezaliczonych testow (0 = wszystko OK). Losowe testy uzywaja cfg.threads watkow.
int runSelfTests(const ExperimentConfig& cfg);
