#include "test_harness.hpp"

int main() {
    int failed = 0;
    for (const auto& [name, fn] : TestRegistry::instance().tests) {
        try {
            fn();
            std::cout << "[PASS] " << name << "\n";
        } catch (const std::exception& e) {
            std::cerr << "[FAIL] " << name << ": " << e.what() << "\n";
            ++failed;
        }
    }
    return failed == 0 ? 0 : 1;
}
