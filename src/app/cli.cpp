#include "app/cli.h"

#include <iostream>

#include "parallel/parallel_for.h"

namespace {

bool parseInt(const std::string& s, long long& v) {
    try {
        std::size_t pos = 0;
        v = std::stoll(s, &pos);
        return pos == s.size();
    } catch (...) {
        return false;
    }
}

bool isMode(const std::string& m) {
    return m == "test" || m == "demo" || m == "experiments" || m == "threadtest" || m == "all";
}

}  // namespace

bool parseArgs(int argc, char** argv, CliOptions& out, std::string& error) {
    out = CliOptions();
    out.exp.sim.contextSwitchCost = 1;  // domyslna "mala kara" za przelaczenie
    long long threads = 1, maxThreads = 0;
    bool modeSet = false;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            out.help = true;
            return true;
        }
        if (arg.rfind("--", 0) != 0) {  // argument pozycyjny = tryb
            if (modeSet || !isMode(arg)) {
                error = "Nieznany tryb lub argument: " + arg;
                return false;
            }
            out.mode = arg;
            modeSet = true;
            continue;
        }
        const auto eq = arg.find('=');
        if (eq == std::string::npos) {
            error = "Opcja wymaga wartosci w formacie --nazwa=wartosc: " + arg;
            return false;
        }
        const std::string key = arg.substr(2, eq - 2);
        long long v = 0;
        if (!parseInt(arg.substr(eq + 1), v)) {
            error = "Wartosc musi byc liczba calkowita: " + arg;
            return false;
        }
        if (key == "threads") threads = v;
        else if (key == "max-threads") maxThreads = v;
        else if (key == "cs-cost") out.exp.sim.contextSwitchCost = static_cast<int>(v);
        else if (key == "runs") out.exp.runs = static_cast<int>(v);
        else if (key == "seed") out.exp.baseSeed = static_cast<unsigned>(v);
        else {
            error = "Nieznana opcja: --" + key;
            return false;
        }
    }

    if (threads < 0 || threads > 1024) { error = "--threads musi byc w zakresie 0..1024 (0 = wszystkie rdzenie)"; return false; }
    if (maxThreads < 0 || maxThreads > 1024) { error = "--max-threads musi byc w zakresie 0..1024"; return false; }
    if (out.exp.sim.contextSwitchCost < 0) { error = "--cs-cost nie moze byc ujemne"; return false; }
    if (out.exp.runs < 1) { error = "--runs musi byc >= 1"; return false; }

    out.exp.threads = resolveThreadCount(static_cast<int>(threads));
    out.maxThreads = resolveThreadCount(static_cast<int>(maxThreads));
    return true;
}

void printUsage(const char* prog) {
    std::cerr << "Uzycie: " << prog << " [tryb] [opcje]\n\n"
              << "Tryby:\n"
              << "  test         testy poprawnosci\n"
              << "  demo         przyklad na 10 procesach\n"
              << "  experiments  eksperymenty A-E -> pliki CSV\n"
              << "  threadtest   test skalowania: te same obliczenia na 1..X watkach (czas, przyspieszenie, zgodnosc)\n"
              << "  all          test + demo + experiments (domyslnie)\n\n"
              << "Opcje:\n"
              << "  --threads=N      liczba watkow roboczych (domyslnie 1; 0 = wszystkie rdzenie)\n"
              << "  --max-threads=X  gorna granica dla trybu threadtest (domyslnie liczba rdzeni)\n"
              << "  --cs-cost=N      kara czasowa za przelaczenie kontekstu (domyslnie 1; 0 = brak)\n"
              << "  --runs=N         liczba losowych zestawow na konfiguracje (domyslnie 100)\n"
              << "  --seed=N         ziarno bazowe (domyslnie 1000)\n";
}
