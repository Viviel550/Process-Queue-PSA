# Symulator algorytmów kolejkowania procesów (C++)

Projekt: **„Analiza i optymalizacja algorytmów kolejkowania procesów z wykorzystaniem adaptacyjnego kwantu czasu”.**

Program symuluje planowanie zadań na jednym procesorze i porównuje klasyczne algorytmy szeregowania z autorskim, adaptacyjnym algorytmem Round Robin. Czas jest symulowany liczbowo (bez `sleep()`), a wyniki eksperymentów są eksportowane do plików CSV.

## Zawartość

| Plik | Opis |
|---|---|
| `main.cpp` | Cały kod: model, algorytmy, metryki, generator, testy, demo, eksperymenty |
| `results_*.csv` | Wyniki eksperymentów A–D (tworzone po uruchomieniu `experiments`) |

## Kompilacja i uruchomienie

Wymagany jest kompilator z obsługą C++17 (np. g++).

```bash
g++ -std=c++17 -O2 -Wall -Wextra -o sim main.cpp

./sim test         # testy poprawności
./sim demo         # przykład na 10 procesach z wydrukiem wyników
./sim experiments  # eksperymenty A–D -> pliki CSV
./sim all          # wszystko po kolei (domyślnie)
```

Program zwraca kod 0, jeśli testy przeszły, i niezerowy w przeciwnym razie.

## Model symulacji

- Jeden procesor i jedna kolejka procesów gotowych.
- Czas jest całkowitoliczbowy i symulowany matematycznie: proces o `remainingTime = 10`, który dostaje kwant 3, ma po turze `remainingTime = 7`.
- Czasy przyjścia i wykonania są znane z góry i generowane programowo. To uproszczenie względem rzeczywistych systemów, w których czasy wykonania nie są dokładnie znane.
- Round Robin, SRTF i adaptacyjny RR są wywłaszczające. FCFS i SJF nie są.
- Procesor może być bezczynny, gdy żaden proces nie jest gotowy. Zegar przeskakuje wtedy do najbliższego przyjścia.

### Struktura `Process`

| Pole | Znaczenie |
|---|---|
| `id` | identyfikator procesu |
| `arrivalTime` | czas przyjścia do kolejki |
| `burstTime` | początkowy wymagany czas wykonania |
| `remainingTime` | pozostały czas wykonania |
| `startTime` | pierwsze uruchomienie (−1, dopóki proces nie wystartował) |
| `completionTime` | czas zakończenia |

## Algorytmy

Każda funkcja przyjmuje wektor procesów **przez kopię** (oryginalne dane wejściowe nie są modyfikowane, więc ten sam zestaw można uruchomić na wszystkich algorytmach) i zwraca `RunResult` (procesy z uzupełnionymi czasami oraz liczba przełączeń kontekstu).

1. **FCFS / FIFO** (`runFCFS`): procesy w kolejności przybycia, bez wywłaszczania.
2. **SJF** (`runSJF`): niewywłaszczający. Po zwolnieniu procesora wybierany jest gotowy proces o najmniejszym `burstTime`. Remis rozstrzyga kolejność przybycia, a potem `id`.
3. **SRTF** (`runSRTF`): wywłaszczająca wersja SJF (piąty, opcjonalny algorytm). Decyzja zapada przy każdym przyjściu i zakończeniu procesu, wybierany jest proces o najmniejszym `remainingTime`.
4. **Round Robin** (`runRR(procs, q)`): stały kwant `q`. Proces, który się nie zakończył, wraca na koniec kolejki.
5. **Adaptacyjny Round Robin** (`runAdaptiveRR(procs, q0, qmin)`): kwant zależy od liczby procesów gotowych.

### Adaptacyjny kwant

```
q(N) = max(qmin, ceil(q0 / N))
```

`N` to liczba procesów gotowych, **łącznie z aktualnie wybieranym**, liczona na początku każdej tury. Dla `q0 = 8` i `qmin = 1`:

| N | 1 | 2 | 3 | 4 | 8 |
|---|---|---|---|---|---|
| q | 8 | 4 | 3 | 2 | 1 |

Round Robin i jego wersja adaptacyjna korzystają z jednego silnika (`runRoundRobinGeneric`), który dostaje funkcję `N -> q`. Dzięki temu oba algorytmy różnią się wyłącznie regułą doboru kwantu.

Szczegół implementacyjny: gdy w trakcie tury (lub w jej momencie zakończenia) przychodzą nowe procesy, trafiają do kolejki **przed** procesem wracającym z tej tury. Jest to standardowa konwencja dla RR.

## Wskaźniki jakości

Dla każdego procesu (`C` – zakończenie, `A` – przyjście, `B` – czas wykonania, `S` – pierwsze uruchomienie):

- czas przebywania w systemie: `T = C − A`
- czas oczekiwania: `W = T − B`
- czas odpowiedzi: `R = S − A`

Program raportuje: średni czas oczekiwania (główne kryterium), średni czas przebywania, średni czas odpowiedzi, czas zakończenia całego zestawu (`makespan` = największe `C`, liczone od t = 0) oraz liczbę przełączeń kontekstu.

### Definicja przełączenia kontekstu

Przełączenie liczymy, gdy procesor zaczyna wykonywać proces **inny** niż ten, który wykonywał się jako ostatni. Nie liczymy pierwszego uruchomienia ani czasu bezczynności. Kolejne tury tego samego procesu pod rząd (np. gdy został sam w kolejce) nie są przełączeniem.

## Generator danych

`generateProcesses(n, mix, seed)` używa `std::mt19937`, więc wyniki są powtarzalne dla danego ziarna.

- Czas przyjścia: losowo (rozkład równomierny) z przedziału `[0, 2n]`.
- Czas wykonania zależy od typu zestawu:
  - `short`: 1–5,
  - `long`: 10–30,
  - `mixed`: 70% procesów krótkich i 30% długich.

W eksperymentach każdy algorytm dostaje **dokładnie te same** zestawy (ziarno = `baseSeed + numer_powtórzenia`), a wyniki są uśredniane po 100 zestawach (`baseSeed = 1000`).

## Eksperymenty

| Eksperyment | Plik | Co zmieniamy |
|---|---|---|
| A – liczba procesów | `results_A_process_count.csv` | `n` ∈ {10, 20, 50, 100, 200, 500}, zestaw `mixed` |
| B – długość procesów | `results_B_burst_length.csv` | `short` / `long` / `mixed`, `n = 50` |
| C – parametry kwantu | `results_C_quantum_params.csv` | RR: `q` ∈ {1, 2, 4, 8, 16, 32}. Adaptacyjny: `q0` ∈ {4, 8, 16, 32, 64} × `qmin` ∈ {1, 2, 4}. `n = 50`, `mixed` |
| D – porównanie | `results_D_comparison.csv` | siatka `mix` × `n` ∈ {10, 50, 200} dla wszystkich algorytmów + zestawienie RR(8) vs adaptacyjny na ekranie |

Domyślne parametry w A, B i D: `RR(q=8)` oraz `AdaptiveRR(q0=8, qmin=1)`.

### Format CSV

```
experiment,scenario,n,mix,algorithm,runs,avg_waiting,avg_turnaround,avg_response,makespan,context_switches
```

Wszystkie wartości metryk są średnimi po `runs` zestawach. Plików można użyć bezpośrednio do wykresów (Excel, Python/pandas, gnuplot).

## Poprawność i testy

`./sim test` sprawdza m.in.:

- FCFS i SJF na klasycznym przykładzie `(A,B) = (0,5), (1,3), (2,8), (3,6)`: średnie oczekiwanie 5,75 (FCFS) i 5,25 (SJF), `makespan` 22,
- SRTF nie ma gorszego oczekiwania niż SJF na tym przykładzie,
- obsługę bezczynności procesora (przerwa między procesami) dla wszystkich algorytmów,
- RR z `q = 4` dla czasów 24, 3, 3 (średnie oczekiwanie 17/3, 3 przełączenia kontekstu),
- tabelę adaptacyjnych kwantów 8, 4, 3, 2, 1,
- 200 losowych zestawów × wszystkie algorytmy pod kątem niezmienników (`validate`): start nie wcześniej niż przyjście, zakończenie nie wcześniej niż `start + burst`, `makespan` nie mniejszy niż suma czasów wykonania.

Ta sama kontrola `validate` działa w trakcie eksperymentów. Niepoprawny harmonogram przerywa program z komunikatem błędu.

## Przykładowe wyniki (domyślne uruchomienie)

Zmiana średniego czasu oczekiwania `AdaptiveRR(8,1)` względem `RR(8)` (wartość ujemna = adaptacyjny lepszy):

| Zestaw | n = 10 | n = 50 | n = 200 |
|---|---|---|---|
| short | +18,5% | +28,4% | +28,7% |
| long | +2,9% | +4,9% | +5,0% |
| mixed | −2,0% | −7,8% | −7,6% |

Adaptacyjny kwant poprawia średni czas oczekiwania w zestawach mieszanych, ale w zestawach jednorodnych (same krótkie albo same długie procesy) wypada gorzej. Wykonuje też znacznie więcej przełączeń kontekstu (np. dla `mixed`, `n = 50`: ok. 368 wobec 78 dla RR(8)). Wyniki zależą od rozkładu generatora i parametrów, więc wymagają dalszej analizy w eksperymencie C.
