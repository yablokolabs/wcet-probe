/// @file main.cpp
/// wcet-probe: measure execution time of built-in example workloads.
///
/// Usage:
///   wcet-probe [--iterations N] [--warmup N] [--cpu N] [--flush-cache] [--trace FILE]

#include "wcet/harness.hpp"
#include "wcet/clock.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

// ── Example workloads ───────────────────────────────────────

/// 64x64 matrix multiply (double precision).
static void matrix_multiply_64() {
    static double A[64][64], B[64][64], C[64][64];
    for (int i = 0; i < 64; ++i)
        for (int j = 0; j < 64; ++j) {
            double sum = 0;
            for (int k = 0; k < 64; ++k)
                sum += A[i][k] * B[k][j];
            C[i][j] = sum;
        }
    // Prevent compiler from optimizing away C
    asm volatile("" :: "m"(C));
}

/// Simulated sensor fusion: weighted average of 128 sensor readings.
static void sensor_fusion_128() {
    static double sensors[128], weights[128], result[4];
    (void)result;
    double sum = 0, wx = 0, wy = 0, wz = 0;
    for (int i = 0; i < 128; ++i) {
        double w = weights[i];
        sum += w;
        wx += sensors[i] * w;
        if (i < 64) wy += sensors[i] * std::sin(static_cast<double>(i));
        else wz += sensors[i] * std::cos(static_cast<double>(i));
    }
    if (sum > 0) { result[0] = wx/sum; result[1] = wy/sum; result[2] = wz/sum; }
    result[3] = sum;
    asm volatile("" :: "m"(result));
}

/// Bubble sort 256 integers (deliberately worst-case-ish).
static void sort_256() {
    static int arr[256];
    // Fill with reverse order each time for consistent worst case
    for (int i = 0; i < 256; ++i) arr[i] = 256 - i;
    for (int i = 0; i < 256; ++i)
        for (int j = 0; j < 255 - i; ++j)
            if (arr[j] > arr[j+1]) {
                int tmp = arr[j]; arr[j] = arr[j+1]; arr[j+1] = tmp;
            }
}

// ── Main ────────────────────────────────────────────────────

int main(int argc, char* argv[]) {
    wcet::HarnessConfig config;
    config.iterations = 100000;
    config.warmup = 1000;

    // Parse arguments
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--iterations") == 0 && i+1 < argc)
            config.iterations = static_cast<std::uint64_t>(std::atol(argv[++i]));
        else if (std::strcmp(argv[i], "--warmup") == 0 && i+1 < argc)
            config.warmup = static_cast<std::uint64_t>(std::atol(argv[++i]));
        else if (std::strcmp(argv[i], "--cpu") == 0 && i+1 < argc)
            config.cpu_pin = std::atoi(argv[++i]);
        else if (std::strcmp(argv[i], "--flush-cache") == 0)
            config.flush_cache = true;
        else if (std::strcmp(argv[i], "--trace") == 0 && i+1 < argc)
            config.trace_path = argv[++i];
    }

    std::fprintf(stderr,
        "╔══════════════════════════════════════════════╗\n"
        "║            wcet-probe v0.1.0                 ║\n"
        "║  Measurement-based WCET analysis toolkit     ║\n"
        "╚══════════════════════════════════════════════╝\n\n"
        "Config:\n"
        "  Iterations:   %lu\n"
        "  Warmup:       %lu\n"
        "  CPU pin:      %s\n"
        "  Cache flush:  %s\n"
        "  Trace:        %s\n\n",
        config.iterations,
        config.warmup,
        config.cpu_pin >= 0 ? "yes" : "no",
        config.flush_cache ? "yes" : "no",
        config.trace_path ? config.trace_path : "none"
    );

    // Run workloads
    std::fprintf(stderr, "Measuring matrix_multiply_64...\n");
    auto r1 = wcet::measure(config, matrix_multiply_64);
    wcet::print_results(r1, "matrix_multiply_64");

    std::fprintf(stderr, "\nMeasuring sensor_fusion_128...\n");
    auto r2 = wcet::measure(config, sensor_fusion_128);
    wcet::print_results(r2, "sensor_fusion_128");

    std::fprintf(stderr, "\nMeasuring sort_256...\n");
    auto r3 = wcet::measure(config, sort_256);
    wcet::print_results(r3, "sort_256");

    return 0;
}
