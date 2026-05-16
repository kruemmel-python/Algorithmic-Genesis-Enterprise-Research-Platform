#include "ag/ag_json.hpp"
#include "ag/ag_expr.hpp"
#include "ag/ag_substrate.hpp"
#include <sstream>

namespace ag {

std::string json_escape(const std::string& s) {
    std::ostringstream os;
    for (char c : s) {
        switch (c) {
            case '"': os << "\\\""; break;
            case '\\': os << "\\\\"; break;
            case '\n': os << "\\n"; break;
            case '\r': os << "\\r"; break;
            case '\t': os << "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) os << "\\u00" << std::hex << static_cast<int>(c);
                else os << c;
        }
    }
    return os.str();
}

std::string genome_to_json(const Genome& g) {
    std::ostringstream os;
    os << "{";
    os << "\"id\":" << g.id << ",";
    os << "\"name\":\"" << json_escape(g.name) << "\",";
    os << "\"fitness\":" << g.fitness << ",";
    os << "\"novelty\":" << g.novelty << ",";
    os << "\"accuracy\":" << g.accuracy << ",";
    os << "\"stability\":" << g.stability << ",";
    os << "\"complexity\":" << g.complexity << ",";
    os << "\"parent_a\":" << g.parent_a << ",";
    os << "\"parent_b\":" << g.parent_b << ",";
    os << "\"birth_generation\":" << g.birth_generation << ",";
    os << "\"origin\":\"" << json_escape(g.origin) << "\",";
    os << "\"mutation_trace\":\"" << json_escape(g.mutation_trace) << "\",";
    os << "\"ast_fingerprint\":\"" << json_escape(g.ast_fingerprint) << "\",";
    os << "\"behavior_fingerprint\":\"" << json_escape(g.behavior_fingerprint) << "\",";
    os << "\"derivative_fingerprint\":\"" << json_escape(g.derivative_fingerprint) << "\",";
    os << "\"stability_fingerprint\":\"" << json_escape(g.stability_fingerprint) << "\",";
    os << "\"complexity_fingerprint\":\"" << json_escape(g.complexity_fingerprint) << "\",";
    os << "\"expression\":\"" << json_escape(genome_to_expression(g)) << "\"";
    os << "}";
    return os.str();
}

std::string evolution_report_to_json(const EvolutionReport& r) {
    std::ostringstream os;
    os << "{";
    os << "\"backend\":\"" << json_escape(r.backend) << "\",";
    os << "\"generator\":\"" << json_escape(r.generator) << "\",";
    os << "\"archive\":{";
    os << "\"before_entries\":" << r.archive_before.entries << ",";
    os << "\"after_entries\":" << r.archive_after.entries << ",";
    os << "\"unique_ast\":" << r.archive_after.unique_ast << ",";
    os << "\"unique_behavior\":" << r.archive_after.unique_behavior << "},";
    os << "\"best\":" << genome_to_json(r.best) << ",";
    os << "\"vm_differential\":" << vm_differential_report_to_json(r.vm_report) << ",";
    os << "\"best_fitness_by_generation\":[";
    for (std::size_t i = 0; i < r.best_fitness_by_generation.size(); ++i) {
        if (i) os << ",";
        os << r.best_fitness_by_generation[i];
    }
    os << "],\"mean_fitness_by_generation\":[";
    for (std::size_t i = 0; i < r.mean_fitness_by_generation.size(); ++i) {
        if (i) os << ",";
        os << r.mean_fitness_by_generation[i];
    }
    os << "]}";
    return os.str();
}

std::string snn_report_to_json(const SnnReport& r) {
    std::ostringstream os;
    os << "{\"backend\":\"" << json_escape(r.backend) << "\",";
    os << "\"opencl\":" << opencl_probe_to_json(r.opencl) << ",";
    os << "\"backend_error\":\"" << json_escape(r.backend_error) << "\",";
    os << "\"backend_model\":\"" << json_escape(r.backend_model) << "\",";
    os << "\"synapse_count\":" << r.synapse_count << ",";
    os << "\"total_spikes\":" << r.total_spikes
       << ",\"mean_voltage\":" << r.mean_voltage
       << ",\"mean_weight\":" << r.mean_weight << ",";
    os << "\"profile\":{";
    os << "\"wall_ms\":" << r.profile.wall_ms << ",";
    os << "\"setup_ms\":" << r.profile.setup_ms << ",";
    os << "\"kernel_ms\":" << r.profile.kernel_ms << ",";
    os << "\"readback_ms\":" << r.profile.readback_ms << ",";
    os << "\"teardown_ms\":" << r.profile.teardown_ms << ",";
    os << "\"local_size\":" << r.profile.local_size << ",";
    os << "\"global_neurons\":" << r.profile.global_neurons << ",";
    os << "\"transfer_bytes\":" << r.profile.transfer_bytes << ",";
    os << "\"neuron_steps_per_second\":" << r.profile.neuron_steps_per_second << ",";
    os << "\"kernel_launches\":" << r.profile.kernel_launches << ",";
    os << "\"differential_checks\":" << r.profile.differential_checks << ",";
    os << "\"max_probe_error\":" << r.profile.max_probe_error;
    os << "},";
    os << "\"somnia_cycles\":" << r.somnia_cycles << ",";
    os << "\"quantized_weights\":" << (r.quantized_weights ? "true" : "false") << ",";
    os << "\"estimated_weight_memory_saving\":" << r.estimated_weight_memory_saving << ",";
    os << "\"growth_events\":" << r.growth_events << ",";
    os << "\"cannibalized_synapses\":" << r.cannibalized_synapses << ",";
    os << "\"arena_capacity\":" << r.arena_capacity << ",";
    os << "\"differential_checks\":" << r.differential_checks << ",";
    os << "\"max_probe_error\":" << r.max_probe_error << ",";
    os << "\"spikes_by_step\":[";
    for (std::size_t i = 0; i < r.spikes_by_step.size(); ++i) {
        if (i) os << ",";
        os << r.spikes_by_step[i];
    }
    os << "]}";
    return os.str();
}

std::string opencl_probe_to_json(const OpenClProbe& p) {
    std::ostringstream os;
    os << "{\"available\":" << (p.available ? "true" : "false")
       << ",\"platform_name\":\"" << json_escape(p.platform_name)
       << "\",\"device_name\":\"" << json_escape(p.device_name)
       << "\",\"error\":\"" << json_escape(p.error) << "\"}";
    return os.str();
}

} // namespace ag
