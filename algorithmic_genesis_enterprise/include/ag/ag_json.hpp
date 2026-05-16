#pragma once
#include "ag_ea.hpp"
#include "ag_snn.hpp"
#include "ag_opencl.hpp"
#include <string>

namespace ag {

std::string json_escape(const std::string& s);
std::string genome_to_json(const Genome& genome);
std::string evolution_report_to_json(const EvolutionReport& report);
std::string snn_report_to_json(const SnnReport& report);
std::string opencl_probe_to_json(const OpenClProbe& probe);

} // namespace ag
