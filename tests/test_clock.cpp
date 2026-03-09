/// @file test_clock.cpp
#include "wcet/clock.hpp"
#include <cassert>
#include <cstdio>

void test_rdtsc_monotonic() {
    auto a = wcet::rdtsc();
    auto b = wcet::rdtsc();
    if (b < a) {
        std::fprintf(stderr, "  ✗ rdtsc not monotonic\n");
        return;
    }
    std::fprintf(stderr, "  ✓ rdtsc monotonic (delta=%lu ticks)\n", b - a);
}

void test_calibration() {
    auto hz = wcet::calibrate_tsc_hz();
    // Sanity: TSC should be between 100MHz and 10GHz
    if (hz < 100'000'000ULL || hz > 10'000'000'000ULL) {
        std::fprintf(stderr, "  ✗ calibration out of range: %lu Hz\n", hz);
        return;
    }
    std::fprintf(stderr, "  ✓ calibration: %.2f GHz\n", static_cast<double>(hz) / 1e9);
}

void test_ticks_to_ns() {
    std::uint64_t tsc_hz = 3'000'000'000ULL;        // 3GHz
    auto ns = wcet::ticks_to_ns(3'000'000, tsc_hz); // 1ms worth of ticks
    // Should be ~1,000,000 ns
    if (ns < 900'000 || ns > 1'100'000) {
        std::fprintf(stderr, "  ✗ ticks_to_ns: expected ~1000000, got %lu\n", ns);
        return;
    }
    std::fprintf(stderr, "  ✓ ticks_to_ns: %lu ns (expected ~1000000)\n", ns);
}

int main() {
    std::fprintf(stderr, "test_clock:\n");
    test_rdtsc_monotonic();
    test_calibration();
    test_ticks_to_ns();
    std::fprintf(stderr, "  All clock tests passed.\n");
    return 0;
}
