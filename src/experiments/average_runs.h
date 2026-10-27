#pragma once
#include "algorithms/algo_registry.h"
#include "core/metrics.h"
#include "experiments/experiment_config.h"
#include "generator/generator.h"

// Srednia metryk po cfg.runs losowych zestawach (ten sam zestaw dla kazdego algorytmu).
// Zestawy sa liczone rownolegle na cfg.threads watkach; wynik NIE zalezy od liczby watkow
// (kazdy zestaw ma wlasny slot, a sumowanie odbywa sie w stalej kolejnosci).
// Rzuca std::runtime_error, jesli ktorys harmonogram jest niepoprawny.
Metrics averageOverRuns(const Algo& algo, int n, Mix mix, const ExperimentConfig& cfg);
