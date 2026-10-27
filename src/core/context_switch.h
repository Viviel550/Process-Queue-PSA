#pragma once
#include <vector>

#include "core/process.h"

// Liczy przelaczenia kontekstu i nalicza za nie kare czasowa.
//
// Przelaczenie = procesor zaczyna wykonywac proces INNY niz ostatnio wykonywany.
// Pierwsze uruchomienie i czas bezczynnosci nie sa przelaczeniem.
class ContextSwitchTracker {
public:
    explicit ContextSwitchTracker(int cost = 0) : cost_(cost < 0 ? 0 : cost) {}

    // Rejestruje chec uruchomienia procesu `id` w chwili `time`.
    // Zwraca chwile, w ktorej proces faktycznie zaczyna sie wykonywac
    // (time + kara, jesli nastapilo przelaczenie; w przeciwnym razie time).
    int onRun(int id, int time);

    int count() const { return count_; }

    // Sklada wynik przebiegu z procesow i statystyk przelaczen.
    RunResult finish(std::vector<Process> procs) const;

private:
    int cost_;
    int last_ = -1;
    int count_ = 0;
};
