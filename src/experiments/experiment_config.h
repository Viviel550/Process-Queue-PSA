#pragma once
#include "core/sim_config.h"

// Wspolne ustawienia uruchomien (eksperymenty, testy, demo, benchmark).
struct ExperimentConfig {
    int runs = 100;            // liczba losowych zestawow na konfiguracje
    unsigned baseSeed = 1000;  // ziarno zestawu r = baseSeed + r
    int threads = 1;           // liczba watkow roboczych
    SimConfig sim;             // model: m.in. kara za przelaczenie kontekstu
};
