/// @file matrix_multiply.cpp
/// Example: WCET analysis of a 64x64 matrix multiply.
/// Demonstrates instrumentation with WCET_PROBE macros.

#include "wcet/probe.hpp"
#include "wcet/harness.hpp"
#include <cstdio>

static double A[64][64], B[64][64], C[64][64];

static void init_matrices() {
    for (int i = 0; i < 64; ++i)
        for (int j = 0; j < 64; ++j) {
            A[i][j] = static_cast<double>(i + j) * 0.01;
            B[i][j] = static_cast<double>(i - j) * 0.01;
            C[i][j] = 0;
        }
}

static void matrix_multiply() {
    for (int i = 0; i < 64; ++i)
        for (int j = 0; j < 64; ++j) {
            double sum = 0;
            for (int k = 0; k < 64; ++k)
                sum += A[i][k] * B[k][j];
            C[i][j] = sum;
        }
}

int main() {
    init_matrices();

    wcet::HarnessConfig config;
    config.iterations = 50000;
    config.warmup = 500;
    config.trace_path = "matrix_multiply.wcet";

    std::fprintf(stderr, "Measuring 64x64 matrix multiply (%lu iterations)...\n\n",
                config.iterations);

    auto results = wcet::measure(config, matrix_multiply);
    wcet::print_results(results, "matrix_multiply_64x64");

    std::fprintf(stderr, "\nTrace written to: matrix_multiply.wcet\n");
    std::fprintf(stderr, "Analyze with: python3 tools/wcet_analyze.py matrix_multiply.wcet\n");

    return 0;
}
