#pragma once
/// @file clock.hpp
/// High-resolution timing primitives.
/// Uses rdtsc on x86 for minimum overhead, clock_gettime as fallback.

#include <cstdint>
#include <time.h>

namespace wcet {

/// Read CPU timestamp counter (x86). ~1ns resolution, ~1 cycle overhead.
[[nodiscard]] inline std::uint64_t rdtsc() noexcept {
#if defined(__x86_64__) || defined(_M_X64)
    std::uint32_t lo, hi;
    asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return (static_cast<std::uint64_t>(hi) << 32) | lo;
#elif defined(__aarch64__)
    std::uint64_t val;
    asm volatile("mrs %0, cntvct_el0" : "=r"(val));
    return val;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<std::uint64_t>(ts.tv_sec) * 1'000'000'000ULL + ts.tv_nsec;
#endif
}

/// Serializing rdtsc (rdtscp on x86) — ensures all prior instructions complete.
[[nodiscard]] inline std::uint64_t rdtscp() noexcept {
#if defined(__x86_64__) || defined(_M_X64)
    std::uint32_t lo, hi, aux;
    asm volatile("rdtscp" : "=a"(lo), "=d"(hi), "=c"(aux));
    return (static_cast<std::uint64_t>(hi) << 32) | lo;
#else
    return rdtsc(); // fallback
#endif
}

/// CLOCK_MONOTONIC nanoseconds (portable, higher overhead).
[[nodiscard]] inline std::int64_t now_ns() noexcept {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<std::int64_t>(ts.tv_sec) * 1'000'000'000LL + ts.tv_nsec;
}

/// Estimate TSC frequency in Hz by measuring over a short interval.
/// Call once at startup; cache the result.
std::uint64_t calibrate_tsc_hz() noexcept;

/// Convert TSC ticks to nanoseconds given a frequency.
[[nodiscard]] inline std::uint64_t ticks_to_ns(std::uint64_t ticks, std::uint64_t tsc_hz) noexcept {
    // Use 128-bit multiply to avoid overflow
    return static_cast<std::uint64_t>(
        (static_cast<__uint128_t>(ticks) * 1'000'000'000ULL) / tsc_hz
    );
}

} // namespace wcet
