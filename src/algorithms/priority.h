#pragma once
#include <vector>

#include "core/process.h"
#include "core/sim_config.h"

// Planowanie priorytetowe oparte o kolejke priorytetowa (std::priority_queue).
// Wybierany jest gotowy proces o najwyzszym priorytecie (MNIEJSZA liczba Process::priority = wyzszy
// priorytet); remis: wczesniejsze przyjscie, potem id.
//   preemptive = false -> proces dziala do konca,
//   preemptive = true  -> przyjscie procesu o wyzszym priorytecie wywlaszcza biezacy.
// Uwaga: algorytm moze zaglodzic procesy o niskim priorytecie (mozliwe rozszerzenie: aging).
RunResult runPriority(std::vector<Process> in, bool preemptive, const SimConfig& cfg = SimConfig());
