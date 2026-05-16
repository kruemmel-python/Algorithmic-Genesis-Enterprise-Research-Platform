#pragma once
#include "ag_expr.hpp"
#include <string>
#include <vector>

namespace ag {

struct FitnessConfig {
    std::string domain{"approximation"};
    int samples{128};
    double x_min{-3.141592653589793};
    double x_max{3.141592653589793};
    double novelty_weight{0.20};
    double accuracy_weight{0.55};
    double stability_weight{0.15};
    double complexity_weight{0.10};
};

struct FitnessDataset {
    std::vector<double> x;
    std::vector<double> y;
};

FitnessDataset make_target_dataset(const FitnessConfig& cfg);
double target_function(double x);
void evaluate_fitness(Genome& genome, const FitnessDataset& dataset, const FitnessConfig& cfg,
                      const std::vector<std::string>& archive_signatures);

} // namespace ag
