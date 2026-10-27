#pragma once
#include <vector>

#include "core/process.h"

// Sortuje wg (czas przyjscia, id).
void sortByArrival(std::vector<Process>& ps);

// Kopia wejscia gotowa do symulacji: wyzerowane pola wynikowe, posortowana wg przyjscia.
std::vector<Process> prepare(std::vector<Process> ps);
