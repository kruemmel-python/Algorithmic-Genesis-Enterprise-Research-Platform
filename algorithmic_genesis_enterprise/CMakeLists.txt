cmake_minimum_required(VERSION 3.20)
project(algorithmic_genesis VERSION 0.1.0 LANGUAGES CXX)

option(AG_ENABLE_OPENCL "Enable OpenCL runtime support" ON)
option(AG_ENABLE_SQLITE "Enable SQLite persistence" ON)
option(AG_BUILD_TESTS "Build tests" ON)
option(AG_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)

add_library(ag_core
    src/ag_rng.cpp
    src/ag_expr.cpp
    src/ag_fitness.cpp
    src/ag_ea.cpp
    src/ag_genesis.cpp
    src/ag_substrate.cpp
    src/ag_algorithm.cpp
    src/ag_formula_parts.cpp
    src/ag_snn.cpp
    src/ag_opencl.cpp
    src/ag_persist.cpp
    src/ag_json.cpp
    src/ag_cli_common.cpp
)

target_include_directories(ag_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_definitions(ag_core PUBLIC
    AG_VERSION="${PROJECT_VERSION}"
)

if (MSVC)
    target_compile_options(ag_core PRIVATE /W4)
    if (AG_WARNINGS_AS_ERRORS)
        target_compile_options(ag_core PRIVATE /WX)
    endif()
else()
    target_compile_options(ag_core PRIVATE -Wall -Wextra -Wpedantic)
    if (AG_WARNINGS_AS_ERRORS)
        target_compile_options(ag_core PRIVATE -Werror)
    endif()
endif()

if (AG_ENABLE_OPENCL)
    find_package(OpenCL)
    if (OpenCL_FOUND)
        target_link_libraries(ag_core PUBLIC OpenCL::OpenCL)
        target_compile_definitions(ag_core PUBLIC AG_ENABLE_OPENCL=1)
    else()
        message(WARNING "OpenCL requested but not found. Building CPU fallback.")
        target_compile_definitions(ag_core PUBLIC AG_ENABLE_OPENCL=0)
    endif()
else()
    target_compile_definitions(ag_core PUBLIC AG_ENABLE_OPENCL=0)
endif()

if (AG_ENABLE_SQLITE)
    find_package(SQLite3)
    if (SQLite3_FOUND)
        target_link_libraries(ag_core PUBLIC SQLite::SQLite3)
        target_compile_definitions(ag_core PUBLIC AG_ENABLE_SQLITE=1)
    else()
        message(WARNING "SQLite requested but not found. Building no-op persistence fallback.")
        target_compile_definitions(ag_core PUBLIC AG_ENABLE_SQLITE=0)
    endif()
else()
    target_compile_definitions(ag_core PUBLIC AG_ENABLE_SQLITE=0)
endif()

add_executable(ag_cli src/main.cpp)
target_link_libraries(ag_cli PRIVATE ag_core)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kernels/random.cl
               ${CMAKE_CURRENT_BINARY_DIR}/kernels/random.cl COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kernels/snn_lif.cl
               ${CMAKE_CURRENT_BINARY_DIR}/kernels/snn_lif.cl COPYONLY)

if (AG_BUILD_TESTS)
    enable_testing()
    add_executable(ag_tests
        tests/test_main.cpp
        tests/test_rng.cpp
        tests/test_expr.cpp
        tests/test_snn.cpp
        tests/test_ea.cpp
        tests/test_persist.cpp
        tests/test_genesis.cpp
        tests/test_substrate.cpp
        tests/test_algorithm.cpp
        tests/test_formula_parts.cpp
    )
    target_link_libraries(ag_tests PRIVATE ag_core)
    add_test(NAME ag_tests COMMAND ag_tests)
endif()

install(TARGETS ag_core ag_cli
    EXPORT agTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY kernels/ DESTINATION ${CMAKE_INSTALL_DATADIR}/algorithmic_genesis/kernels)
