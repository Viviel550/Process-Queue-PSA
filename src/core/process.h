#pragma once
#include <vector>

// Pojedynczy proces w symulacji.
struct Process {
    int id = 0;
    int arrivalTime = 0;
    int burstTime = 0;
    int remainingTime = 0;
    int startTime = -1;       // pierwsze faktyczne uruchomienie na procesorze (po ew. karze przelaczenia)
    int completionTime = -1;  // zakonczenie
    int priority = 0;         // mniejsza liczba = wyzszy priorytet (uzywane przez algorytm priorytetowy)
};

// Wynik jednego przebiegu algorytmu.
struct RunResult {
    std::vector<Process> procs;
    int contextSwitches = 0;  // liczba przelaczen kontekstu
    int overheadTime = 0;     // laczny czas stracony na przelaczenia (= contextSwitches * koszt)
};
