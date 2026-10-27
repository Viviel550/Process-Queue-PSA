#pragma once
#include <cstddef>
#include <functional>

// 0 -> liczba rdzeni sprzetowych (min. 1), wartosc ujemna -> 1, inaczej bez zmian.
int resolveThreadCount(int requested);

// Wykonuje body(0..count-1) na `threads` watkach (dynamiczny podzial pracy przez wspolny licznik).
// Watek wywolujacy tez pracuje. Wyjatek z dowolnego zadania jest przekazywany do wolajacego
// (po zakonczeniu pozostalych watkow). body musi pisac tylko do rozlacznych danych (np. wlasny indeks).
void parallelFor(std::size_t count, int threads, const std::function<void(std::size_t)>& body);
