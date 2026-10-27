#pragma once
#include "experiments/experiment_config.h"

// Tryb "testowy" wielowatkowosci: to samo obciazenie liczone na 1, 2, ..., maxThreads watkach.
// Wypisuje czas, przyspieszenie, efektywnosc i sprawdza, czy wyniki sa identyczne jak dla 1 watku.
// Zwraca 0, jesli wszystkie wyniki sie zgadzaja, w przeciwnym razie 1.
int runThreadBenchmark(int maxThreads, const ExperimentConfig& cfg);
