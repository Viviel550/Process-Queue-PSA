#include "experiments/csv_output.h"

#include <iomanip>

static const char* kCsvHeader =
    "experiment,scenario,n,mix,algorithm,runs,avg_waiting,avg_turnaround,avg_response,makespan,context_switches\n";

std::ofstream openCsv(const std::string& path) {
    std::ofstream f(path);
    f << std::fixed << std::setprecision(4) << kCsvHeader;
    return f;
}

void writeRow(std::ofstream& f, const std::string& exp, const std::string& scenario, int n, Mix mix,
              const std::string& algo, int runs, const Metrics& m) {
    f << exp << ',' << scenario << ',' << n << ',' << mixName(mix) << ",\"" << algo << "\"," << runs << ','
      << m.avgWaiting << ',' << m.avgTurnaround << ',' << m.avgResponse << ',' << m.makespan << ','
      << m.contextSwitches << '\n';
}
