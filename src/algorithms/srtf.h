#pragma once
#include <vector>

#include "core/process.h"
#include "core/sim_config.h"

// SRTF: wywlaszczajaca wersja SJF. Decyzja zapada przy kazdym przyjsciu i zakonczeniu procesu.
// Jesli podczas przelaczenia kontekstu przyjdzie lepszy proces, dispatcher przelacza sie ponownie
// (z kolejna kara), tak jak zrobilby to prawdziwy system.
RunResult runSRTF(std::vector<Process> in, const SimConfig& cfg = SimConfig());
