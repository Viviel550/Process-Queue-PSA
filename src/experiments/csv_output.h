#pragma once
#include <fstream>
#include <string>

#include "core/metrics.h"
#include "generator/generator.h"

// Otwiera plik CSV, ustawia format liczb i zapisuje naglowek.
std::ofstream openCsv(const std::string& path);

void writeRow(std::ofstream& f, const std::string& exp, const std::string& scenario, int n, Mix mix,
              const std::string& algo, int runs, const Metrics& m);
