#pragma once
#include "experiments/experiment_config.h"

void runExperimentA(const ExperimentConfig& cfg);  // wplyw liczby procesow
void runExperimentB(const ExperimentConfig& cfg);  // wplyw dlugosci procesow
void runExperimentC(const ExperimentConfig& cfg);  // parametry kwantu (RR vs adaptacyjny)
void runExperimentD(const ExperimentConfig& cfg);  // pelne porownanie algorytmow + zestawienie na ekranie
void runExperimentE(const ExperimentConfig& cfg);  // wplyw kary za przelaczenie kontekstu

// A-E po kolei.
void runAllExperiments(const ExperimentConfig& cfg);
