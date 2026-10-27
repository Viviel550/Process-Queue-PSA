#pragma once
#include <vector>

#include "core/process.h"

enum class Mix { Short, Long, Mixed };

const char* mixName(Mix m);

constexpr int kMaxPriority = 5;

// Powtarzalny generator zestawow (std::mt19937). Czasy przyjscia i wykonania zalezne tylko od `seed`
// (priorytety losowane z OSOBNEGO generatora, wiec nie zmieniaja pozostalych danych).
std::vector<Process> generateProcesses(int n, Mix mix, unsigned seed, int arrivalSpread = 2);
