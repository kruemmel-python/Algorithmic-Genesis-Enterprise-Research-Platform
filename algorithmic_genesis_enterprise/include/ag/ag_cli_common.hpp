#pragma once
#include <string>
#include <vector>

namespace ag {

struct CliArgs {
    std::vector<std::string> args;
    explicit CliArgs(int argc, char** argv);
    bool has(const std::string& key) const;
    std::string get(const std::string& key, const std::string& default_value) const;
    int get_int(const std::string& key, int default_value) const;
    unsigned long long get_u64(const std::string& key, unsigned long long default_value) const;
    double get_double(const std::string& key, double default_value) const;
};

void write_file(const std::string& path, const std::string& content);
void print_usage();

} // namespace ag
