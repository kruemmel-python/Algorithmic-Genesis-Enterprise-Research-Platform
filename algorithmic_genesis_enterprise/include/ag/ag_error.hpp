#pragma once
#include <stdexcept>
#include <string>

namespace ag {

class Error final : public std::runtime_error {
public:
    explicit Error(const std::string& message) : std::runtime_error(message) {}
};

#define AG_REQUIRE(condition, message) \
    do { if (!(condition)) { throw ::ag::Error(message); } } while(false)

} // namespace ag
