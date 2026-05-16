#pragma once
#include <cstdint>
#include <vector>
#include <string>

namespace ag {

class SplitMix64 final {
public:
    explicit SplitMix64(uint64_t seed = 0x9E3779B97F4A7C15ull);
    uint64_t next_u64();
    uint32_t next_u32();
    double uniform01();
    float uniform01f();
    int uniform_int(int lo, int hi);
    bool bernoulli(double p);
    double normal01();

private:
    uint64_t state_;
    bool has_spare_{false};
    double spare_{0.0};
};

std::vector<float> random_floats_cpu(uint64_t seed, std::size_t count);
std::string hex64(uint64_t value);

} // namespace ag
