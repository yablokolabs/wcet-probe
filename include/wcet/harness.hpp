#pragma once
/// @file harness.hpp
/// Measurement harness: runs a function many times under controlled conditions
/// and collects timing samples.

#include "wcet/probe.hpp"
#include "wcet/cpu_control.hpp"
#include <cstdint>
#include <functional>
#include <vector>

namespace wcet {

/// Harness configuration.
struct HarnessConfig {
    std::uint64_t iterations   = 10000;   ///< Number of measurement iterations
    std::uint64_t warmup       = 100;     ///< Warmup iterations (discarded)
    int           cpu_pin      = -1;      ///< CPU to pin to (-1 = no pin)
    bool          flush_cache  = false;   ///< Flush caches before each iteration
    bool          flush_tlb    = false;   ///< Flush TLB before each iteration
    const char*   trace_path   = nullptr; ///< Output trace file (null = no file)
};

/// Measurement results (computed from samples).
struct HarnessResults {
    std::uint64_t num_samples;
    std::uint64_t tsc_hz;        ///< TSC frequency used for conversion

    // In nanoseconds
    std::uint64_t min_ns;
    std::uint64_t max_ns;
    std::uint64_t mean_ns;
    std::uint64_t median_ns;
    std::uint64_t p90_ns;
    std::uint64_t p95_ns;
    std::uint64_t p99_ns;
    std::uint64_t p999_ns;
    std::uint64_t p9999_ns;
    std::uint64_t stddev_ns;

    // Raw TSC ticks
    std::uint64_t min_ticks;
    std::uint64_t max_ticks;
};

/// Run a measurement harness on a callable.
/// The callable takes no arguments and returns void.
HarnessResults measure(const HarnessConfig& config,
                       const std::function<void()>& workload) noexcept;

/// Print results to stderr.
void print_results(const HarnessResults& results, const char* name) noexcept;

} // namespace wcet
