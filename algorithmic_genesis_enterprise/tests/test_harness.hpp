#pragma once
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using TestFn = void(*)();

struct TestRegistry {
    std::vector<std::pair<std::string, TestFn>> tests;
    static TestRegistry& instance() { static TestRegistry r; return r; }
    void add(std::string name, TestFn fn) { tests.emplace_back(std::move(name), fn); }
};

struct TestAdder {
    TestAdder(const std::string& name, TestFn fn) { TestRegistry::instance().add(name, fn); }
};

#define TEST_CASE(name) \
    static void name(); \
    static TestAdder adder_##name(#name, &name); \
    static void name()

#define CHECK_TRUE(x) do { if (!(x)) throw std::runtime_error(std::string("CHECK_TRUE failed: ") + #x); } while(false)
#define CHECK_NEAR(a,b,eps) do { if (std::fabs((a)-(b)) > (eps)) throw std::runtime_error("CHECK_NEAR failed"); } while(false)
#define CHECK_EQ(a,b) do { if (!((a)==(b))) throw std::runtime_error(std::string("CHECK_EQ failed: ") + #a + " == " + #b); } while(false)
