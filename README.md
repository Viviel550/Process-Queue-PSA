# Symulator algorytmów kolejkowania procesów (C++17)

Projekt: **„Analiza i optymalizacja algorytmów kolejkowania procesów z wykorzystaniem adaptacyjnego kwantu czasu”.**

Program symuluje planowanie zadań na jednym procesorze i porównuje klasyczne algorytmy szeregowania z autorskim, adaptacyjnym algorytmem Round Robin. Czas jest symulowany liczbowo (bez `sleep()`), a wyniki eksperymentów są eksportowane do plików CSV.

## Co nowego względem wersji z jednym plikiem

| Zmiana | Gdzie |
|---|---|
| Algorytm priorytetowy oparty o kolejkę priorytetową (`std::priority_queue`), wersja wywłaszczająca i niewywłaszczająca | `src/algorithms/priority.*` |
| Kara czasowa za przełączenie kontekstu (`--cs-cost=N`, domyślnie 1) | `src/core/context_switch.*`, `SimConfig` |
| Podział `main.cpp` na moduły, jeden algorytm = jeden plik | `src/` |
| Wielowątkowość: `--threads=N` oraz tryb testowy `threadtest` (1..X wątków) | `src/parallel/`, `src/app/thread_benchmark.*` |
| Nowy eksperyment E (wpływ kary przełączenia) | `src/experiments/experiment_e.cpp` |

## Struktura projektu

```
src/
  main.cpp                        tylko parsowanie argumentów i wybór trybu
  core/
    process.h                     Process, RunResult
    sim_config.h                  SimConfig (kara przełączenia)
    context_switch.*              ContextSwitchTracker: liczenie przełączeń + kara czasowa
    process_utils.*               sortByArrival, prepare
    metrics.*                     Metrics, computeMetrics, sameMetrics
    validate.*                    kontrola niezmienników harmonogramu
  algorithms/
    fcfs.*  sjf.*  srtf.*  priority.*
    round_robin_engine.*          wspólny silnik RR (funkcja N -> q)
    round_robin.*  adaptive_round_robin.*
    algo_registry.*               struct Algo, defaultAlgos, makeRR, makeAdaptiveRR
  generator/generator.*           Mix, generateProcesses
  parallel/parallel_for.*         parallelFor, resolveThreadCount
  experiments/
    experiment_config.h           runs, baseSeed, threads, SimConfig
    average_runs.*                średnia po zestawach (równolegle)
    csv_output.*                  zapis CSV
    experiment_a.cpp … experiment_e.cpp, run_all_experiments.cpp
  app/
    cli.*  self_tests.*  demo.*  thread_benchmark.*
```

## Kompilacja i uruchomienie

Wymagany kompilator z obsługą C++17 i wątków (np. g++).

```bash
make                      # buduje ./sim  (albo: cmake -B build && cmake --build build)
# ręcznie: g++ -std=c++17 -O2 -Wall -Wextra -pthread -Isrc $(find src -name '*.cpp') -o sim

./sim test                # testy poprawności
./sim demo                # przykład na 10 procesach
./sim experiments         # eksperymenty A–E -> pliki CSV
./sim all                 # test + demo + experiments (domyślnie)
./sim threadtest          # test wielowątkowości: 1..X wątków
```

Opcje (format `--nazwa=wartość`):

| Opcja | Znaczenie | Domyślnie |
|---|---|---|
| `--threads=N` | liczba wątków roboczych dla testów i eksperymentów; `0` = wszystkie rdzenie | 1 |
| `--max-threads=X` | górna granica trybu `threadtest` (sprawdza 1, 2, …, X); `0` = liczba rdzeni | liczba rdzeni |
| `--cs-cost=N` | kara czasowa za każde przełączenie kontekstu; `0` = model idealny | 1 |
| `--runs=N` | liczba losowych zestawów na konfigurację | 100 |
| `--seed=N` | ziarno bazowe | 1000 |

Przykłady: `./sim experiments --threads=8 --cs-cost=2`, `./sim threadtest --max-threads=16 --runs=200`.
Program zwraca kod 0, jeśli testy przeszły, i niezerowy w przeciwnym razie (2 przy błędnych argumentach).

## Model symulacji

- Jeden procesor i jedna kolejka procesów gotowych. Czas jest całkowitoliczbowy.
- Czasy przyjścia i wykonania są znane z góry (uproszczenie względem realnych systemów).
- RR, SRTF, adaptacyjny RR i priorytetowy z wywłaszczaniem są wywłaszczające. FCFS, SJF i priorytetowy bez wywłaszczania nie są.
- Procesor może być bezczynny; zegar przeskakuje wtedy do najbliższego przyjścia.

### Przełączenie kontekstu i kara czasowa

Przełączenie liczymy, gdy procesor zaczyna wykonywać proces **inny** niż ostatnio wykonywany. Nie liczymy pierwszego uruchomienia ani bezczynności.

Każde przełączenie **kosztuje `contextSwitchCost` jednostek czasu**, w których procesor nie wykonuje żadnego procesu. Konsekwencje:

- `startTime` to chwila faktycznego ruszenia procesu, czyli już po karze (czas odpowiedzi ją zawiera).
- Kara wchodzi do czasu oczekiwania (`W = C − A − B`), bo nie jest częścią `B`.
- `makespan ≥ suma burstów + liczba_przełączeń · koszt`. Ten niezmiennik sprawdza `validate`.
- Gdy w trakcie przełączenia przyjdzie lepszy proces (SRTF, priorytetowy z wywłaszczaniem), dispatcher przełącza się ponownie, z kolejną karą.
- W RR liczba gotowych procesów `N` (dla kwantu adaptacyjnego) jest liczona na początku tury, przed karą.

### Planowanie priorytetowe

`runPriority(procs, preemptive)`: gotowe procesy trzymane są w `std::priority_queue`. Mniejsza liczba `priority` oznacza wyższy priorytet. Remis rozstrzyga kolejność przybycia, potem `id`. Generator losuje priorytety 1–5 z **osobnego** generatora liczb losowych, więc nie zmienia pozostałych danych (wyniki innych algorytmów z poprzedniej wersji są odtwarzalne). Algorytm może zagłodzić procesy o niskim priorytecie. Możliwe rozszerzenie: *aging*.

## Algorytmy

1. **FCFS** (`runFCFS`)
2. **SJF** niewywłaszczający (`runSJF`)
3. **SRTF** (`runSRTF`)
4. **Priorytetowy** (`runPriority`, wariant `np` i `p`)
5. **Round Robin** (`runRR(procs, q)`)
6. **Adaptacyjny Round Robin** (`runAdaptiveRR(procs, q0, qmin)`): `q(N) = max(qmin, ceil(q0 / N))`, gdzie `N` to liczba procesów gotowych łącznie z wybieranym.

Każda funkcja przyjmuje wektor procesów przez kopię oraz `SimConfig` i zwraca `RunResult`.

## Wielowątkowość

Równolegle liczone są **niezależne zestawy losowe** (`averageOverRuns`, testy losowe). Algorytmy są czystymi funkcjami, a każdy zestaw zapisuje wynik do własnego slotu. Sumowanie odbywa się potem w stałej kolejności, więc **wyniki są bitowo identyczne niezależnie od liczby wątków**. Weryfikują to test `./sim test` i tryb `threadtest`. Wyjątki z wątków (np. niepoprawny harmonogram) są przekazywane do wątku głównego.

`./sim threadtest [--max-threads=X]` liczy to samo obciążenie (7 algorytmów × `--runs` zestawów, n = 400) na 1, 2, …, X wątkach. Wypisuje czas, przyspieszenie, efektywność i zgodność wyników z przebiegiem jednowątkowym:

```
   watki   czas [ms]  przyspieszenie  efektywnosc      wyniki
       1        79.2           1.00x         100%      (baza)
       2        ...
```

> Uwaga: środowisko, w którym sprawdzałem kod, miało 1 rdzeń, więc zmierzone przyspieszenie wynosiło ≈1,0x (przy pełnej zgodności wyników). Realne przyspieszenie zobaczysz na swoim komputerze.

## Wskaźniki jakości

`T = C − A`, `W = T − B`, `R = S − A`. Raportowane: średni czas oczekiwania (główne kryterium), przebywania, odpowiedzi, `makespan` i liczba przełączeń.

## Generator danych

`generateProcesses(n, mix, seed)` (`std::mt19937`): przyjście z `[0, 2n]`, czas wykonania `short` 1–5, `long` 10–30, `mixed` 70% krótkich i 30% długich, priorytet 1–5. W eksperymentach każdy algorytm dostaje te same zestawy (ziarno = `baseSeed + nr_powtórzenia`).

## Eksperymenty

| Eksperyment | Plik | Co zmieniamy |
|---|---|---|
| A | `results_A_process_count.csv` | `n` ∈ {10, 20, 50, 100, 200, 500}, `mixed` |
| B | `results_B_burst_length.csv` | `short` / `long` / `mixed`, `n = 50` |
| C | `results_C_quantum_params.csv` | RR: `q` ∈ {1…32}; adaptacyjny: `q0` ∈ {4…64} × `qmin` ∈ {1, 2, 4} |
| D | `results_D_comparison.csv` | `mix` × `n` ∈ {10, 50, 200}, wszystkie algorytmy + zestawienie RR(8) vs adaptacyjny |
| **E** | `results_E_switch_cost.csv` | kara przełączenia ∈ {0, 1, 2, 4, 8}, `n = 50`, `mixed` |

Format CSV bez zmian: `experiment,scenario,n,mix,algorithm,runs,avg_waiting,avg_turnaround,avg_response,makespan,context_switches`.

## Testy

`./sim test` sprawdza m.in.: klasyczny przykład FCFS/SJF, SRTF ≤ SJF, bezczynność, RR(q=4), tabelę kwantów adaptacyjnych, **kolejność i wywłaszczanie w algorytmie priorytetowym** (oraz zgodność z FCFS przy równych priorytetach), **dokładne wartości przy karze przełączenia** (FCFS kara 2 → makespan 28, RR kara 1 → makespan 33, …), 200 losowych zestawów × wszystkie algorytmy dla kary 0 i 2 oraz **identyczność wyników dla 1 i wielu wątków**.

## Wyniki

**Kara 0 (model idealny, jak w poprzedniej wersji; `--cs-cost=0`)**: zmiana średniego oczekiwania `AdaptiveRR(8,1)` względem `RR(8)` (ujemna = adaptacyjny lepszy):

| Zestaw | n = 10 | n = 50 | n = 200 |
|---|---|---|---|
| short | +18,5% | +28,4% | +28,7% |
| long | +2,9% | +4,9% | +5,0% |
| mixed | −2,0% | −7,8% | −7,6% |

**Kara 1 (domyślna)**:

| Zestaw | n = 10 | n = 50 | n = 200 |
|---|---|---|---|
| short | +48,4% | +132,9% | +150,1% |
| long | +79,8% | +90,1% | +89,9% |
| mixed | +38,6% | +69,1% | +71,4% |

**Wniosek:** przy zerowym koszcie przełączeń adaptacyjny kwant poprawia oczekiwanie w zestawach mieszanych (−7,8%). Już kara równa 1 jednostce czasu odwraca ten wynik, bo adaptacyjny RR wykonuje ok. 370 przełączeń kontekstu wobec 78 dla RR(8) (`mixed`, `n = 50`). Przy karze 4 średnie oczekiwanie wynosi 815 wobec 287 dla RR(8). Eksperyment E pokazuje to dla pełnego zakresu kar. Żeby adaptacyjny kwant miał sens w realistycznym modelu, trzeba ograniczyć liczbę przełączeń, np. większe `qmin` (patrz eksperyment C) albo histereza zmiany kwantu.

Algorytm priorytetowy zachowuje się podobnie do FCFS (oczekiwanie ok. 178 wobec 176 dla `mixed`, `n = 50`, kara 1). Priorytety w generatorze są niezależne od długości procesu, więc nie skracają one średniego czasu oczekiwania.

## Ograniczenia i możliwe rozszerzenia

- Kara przełączenia jest stała (nie zależy od procesu). Można ją uzależnić np. od rozmiaru stanu procesu.
- SJF i SRTF mają złożoność O(n²). Można je przepisać na kolejkę priorytetową tak jak algorytm priorytetowy.
- Aging w algorytmie priorytetowym, inne rozkłady przyjść, miary sprawiedliwości.
