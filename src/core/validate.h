#pragma once
#include "core/process.h"

// Kontrola niezmiennikow harmonogramu:
//  - start nie wczesniej niz przyjscie,
//  - zakonczenie nie wczesniej niz start + burst,
//  - makespan >= suma burstow + czas stracony na przelaczenia (jeden procesor).
bool validate(const RunResult& r);
