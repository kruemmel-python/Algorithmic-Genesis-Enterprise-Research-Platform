#include "ag/ag_rng.hpp"
#include <cmath>
#include <iomanip>
#include <sstream>

namespace ag {

SplitMix64::SplitMix64(uint64_t seed) : state_(seed) {}

uint64_t SplitMix64::next_u64() {
    uint64_t z = (state_ += 0x9E3779B97F4A7C15ull);
    z = (z ^ (z >> 30u)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27u)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31u);
}

uint32_t SplitMix64::next_u32() {
    return static_cast<uint32_t>(next_u64() >> 32u);
}

double SplitMix64::uniform01() {
    return (next_u64() >> 11u) * (1.0 / 9007199254740992.0);
}

float SplitMix64::uniform01f() {
    return static_cast<float>(uniform01());
}

int SplitMix64::uniform_int(int lo, int hi) {
    if (hi <= lo) return lo;
    const uint32_t span = static_cast<uint32_t>(hi - lo + 1);
    return lo + static_cast<int>(next_u32() % span);
}

bool SplitMix64::bernoulli(double p) {
    return uniform01() < p;
}

double SplitMix64::normal01() {
    if (has_spare_) {
        has_spare_ = false;
        return spare_;
    }
    const double u = std::max(1e-12, uniform01());
    const double v = uniform01();
    const double r = std::sqrt(-2.0 * std::log(u));
    const double theta = 6.283185307179586 * v;
    spare_ = r * std::sin(theta);
    has_spare_ = true;
    return r * std::cos(theta);
}

std::vector<float> random_floats_cpu(uint64_t seed, std::size_t count) {
    SplitMix64 rng(seed);
    std::vector<float> out(count);
    for (auto& x : out) x = rng.uniform01f();
    return out;
}

std::string hex64(uint64_t value) {
    std::ostringstream os;
    os << std::hex << std::setw(16) << std::setfill('0') << value;
    return os.str();
}

} // namespace ag
