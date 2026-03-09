/// @file bench_overhead.cpp
/// Measure instrumentation overhead of WCET probes.

#include "wcet/clock.hpp"
#include "wcet/probe.hpp"
#include <cstdio>

int main() {
    constexpr int N = 1'000'000;

    // Measure rdtsc overhead
    {
        auto start = wcet::now_ns();
        for (int i = 0; i < N; ++i) {
            auto t = wcet::rdtsc();
            (void)t;
        }
        auto elapsed = wcet::now_ns() - start;
        std::fprintf(stderr, "rdtsc overhead:     %.1f ns/call\n",
                     static_cast<double>(elapsed) / N);
    }

    // Measure rdtscp overhead
    {
        auto start = wcet::now_ns();
        for (int i = 0; i < N; ++i) {
            auto t = wcet::rdtscp();
            (void)t;
        }
        auto elapsed = wcet::now_ns() - start;
        std::fprintf(stderr, "rdtscp overhead:    %.1f ns/call\n",
                     static_cast<double>(elapsed) / N);
    }

    // Measure probe record overhead
    {
        wcet::ProbeContext ctx;
        auto start = wcet::now_ns();
        for (int i = 0; i < N; ++i) {
            ctx.record(1, 0, 100);
        }
        auto elapsed = wcet::now_ns() - start;
        std::fprintf(stderr, "probe record:       %.1f ns/call\n",
                     static_cast<double>(elapsed) / N);
    }

    // Measure full scoped probe overhead (rdtsc + rdtscp + record)
    {
        auto start = wcet::now_ns();
        for (int i = 0; i < N; ++i) {
            WCET_PROBE_SCOPE(overhead_test);
        }
        auto elapsed = wcet::now_ns() - start;
        std::fprintf(stderr, "scoped probe:       %.1f ns/call\n",
                     static_cast<double>(elapsed) / N);
    }

    return 0;
}
