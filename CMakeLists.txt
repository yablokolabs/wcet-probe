cmake_minimum_required(VERSION 3.25)
project(wcet-probe
    VERSION 0.1.0
    LANGUAGES CXX
    DESCRIPTION "Measurement-based WCET analysis toolkit for C++ real-time code"
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(
    -Wall -Wextra -Wpedantic -Werror
    -Wno-unused-parameter
)

# ── Library ──────────────────────────────────────────────────

add_library(wcet_core STATIC
    src/clock.cpp
    src/ring_buffer.cpp
    src/trace.cpp
    src/cpu_control.cpp
    src/harness.cpp
)

target_include_directories(wcet_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(wcet_core PUBLIC pthread rt)

# ── Main executable ──────────────────────────────────────────

add_executable(wcet-probe src/main.cpp)
target_link_libraries(wcet-probe PRIVATE wcet_core)

# ── Examples ─────────────────────────────────────────────────

add_executable(example_matrix examples/matrix_multiply.cpp)
target_link_libraries(example_matrix PRIVATE wcet_core)

add_executable(example_sensor examples/sensor_fusion_stub.cpp)
target_link_libraries(example_sensor PRIVATE wcet_core)

# ── Tests ────────────────────────────────────────────────────

option(WCET_BUILD_TESTS "Build tests" ON)

if(WCET_BUILD_TESTS)
    enable_testing()

    add_executable(test_clock tests/test_clock.cpp)
    target_link_libraries(test_clock PRIVATE wcet_core)
    add_test(NAME test_clock COMMAND test_clock)

    add_executable(test_ring_buffer tests/test_ring_buffer.cpp)
    target_link_libraries(test_ring_buffer PRIVATE wcet_core)
    add_test(NAME test_ring_buffer COMMAND test_ring_buffer)

    add_executable(test_probe tests/test_probe.cpp)
    target_link_libraries(test_probe PRIVATE wcet_core)
    add_test(NAME test_probe COMMAND test_probe)

    add_executable(test_trace tests/test_trace.cpp)
    target_link_libraries(test_trace PRIVATE wcet_core)
    add_test(NAME test_trace COMMAND test_trace)
endif()

# ── Benchmarks ───────────────────────────────────────────────

option(WCET_BUILD_BENCH "Build benchmarks" OFF)

if(WCET_BUILD_BENCH)
    add_executable(bench_overhead bench/bench_overhead.cpp)
    target_link_libraries(bench_overhead PRIVATE wcet_core)
endif()
