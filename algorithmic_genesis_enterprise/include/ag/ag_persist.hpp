#pragma once
#include "ag_expr.hpp"
#include <string>
#include <vector>

namespace ag {

class AlgorithmStore final {
public:
    explicit AlgorithmStore(std::string path);
    ~AlgorithmStore();

    AlgorithmStore(const AlgorithmStore&) = delete;
    AlgorithmStore& operator=(const AlgorithmStore&) = delete;

    void open();
    void save_genome(const Genome& genome, const std::string& expression, const std::string& notes);
    std::vector<Genome> load_top(int limit);
    bool enabled() const;

private:
    std::string path_;
    void* db_{nullptr};
};

} // namespace ag
