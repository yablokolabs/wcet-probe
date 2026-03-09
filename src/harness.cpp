/// @file harness.cpp

#include "wcet/harness.hpp"
#include "wcet/clock.hpp"
#include "wcet/trace.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <numeric>

namespace wcet {

static ProbeContext g_context;

ProbeContext& global_context() noexcept {
    return g_context;
}

// Thread-local context
static thread_local ProbeContext tl_context;

ProbeContext& thread_context() noexcept {
    return tl_context;
}

HarnessResults measure(const HarnessConfig& config,
                       const std::function<void()>& workload) noexcept {
    HarnessResults results{};

    // Calibrate TSC
    auto tsc_hz = calibrate_tsc_hz();
    results.tsc_hz = tsc_hz;

    // Pin CPU if requested
    if (config.cpu_pin >= 0) {
        pin_to_cpu(config.cpu_pin);
    }

    // Warmup
    for (std::uint64_t i = 0; i < config.warmup; ++i) {
        workload();
    }

    // Collect samples
    std::vector<std::uint64_t> samples;
    samples.reserve(config.iterations);

    for (std::uint64_t i = 0; i < config.iterations; ++i) {
        if (config.flush_cache) flush_data_cache();
        if (config.flush_tlb) flush_tlb();

        auto start = rdtsc();
        workload();
        auto end = rdtscp();

        samples.push_back(end - start);
    }

    results.num_samples = samples.size();
    if (samples.empty()) return results;

    // Sort for percentile computation
    std::sort(samples.begin(), samples.end());

    results.min_ticks = samples.front();
    results.max_ticks = samples.back();

    // Convert to nanoseconds
    std::vector<std::uint64_t> ns_samples;
    ns_samples.reserve(samples.size());
    for (auto t : samples) {
        ns_samples.push_back(ticks_to_ns(t, tsc_hz));
    }

    std::sort(ns_samples.begin(), ns_samples.end());

    auto n = ns_samples.size();
    results.min_ns    = ns_samples.front();
    results.max_ns    = ns_samples.back();
    results.median_ns = ns_samples[n / 2];
    results.p90_ns    = ns_samples[static_cast<std::size_t>(n * 0.90)];
    results.p95_ns    = ns_samples[static_cast<std::size_t>(n * 0.95)];
    results.p99_ns    = ns_samples[static_cast<std::size_t>(n * 0.99)];
    results.p999_ns   = ns_samples[std::min(static_cast<std::size_t>(n * 0.999), n - 1)];
    results.p9999_ns  = ns_samples[std::min(static_cast<std::size_t>(n * 0.9999), n - 1)];

    // Mean
    double sum = 0;
    for (auto v : ns_samples) sum += static_cast<double>(v);
    double mean = sum / static_cast<double>(n);
    results.mean_ns = static_cast<std::uint64_t>(mean);

    // Stddev
    double var_sum = 0;
    for (auto v : ns_samples) {
        double d = static_cast<double>(v) - mean;
        var_sum += d * d;
    }
    results.stddev_ns = static_cast<std::uint64_t>(std::sqrt(var_sum / static_cast<double>(n)));

    // Write trace if requested
    if (config.trace_path) {
        std::vector<ProbeSample> trace_samples;
        trace_samples.reserve(samples.size());
        for (std::size_t i = 0; i < samples.size(); ++i) {
            ProbeSample s{};
            s.elapsed_tsc = samples[i]; // unsorted original order not preserved; use sorted
            s.probe_id = 0;
            trace_samples.push_back(s);
        }
        write_trace(config.trace_path, trace_samples, tsc_hz);
    }

    return results;
}

void print_results(const HarnessResults& results, const char* name) noexcept {
    std::fprintf(stderr,
        "┌──────────────────────────────────────────────┐\n"
        "│  WCET Analysis: %-28s │\n"
        "├──────────────────────────────────────────────┤\n"
        "│  Samples:    %-10lu  TSC: %.2f GHz     │\n"
        "├──────────────────────────────────────────────┤\n"
        "│  Min:        %-10lu ns                  │\n"
        "│  Mean:       %-10lu ns (σ=%lu ns)       │\n"
        "│  Median:     %-10lu ns                  │\n"
        "│  p90:        %-10lu ns                  │\n"
        "│  p95:        %-10lu ns                  │\n"
        "│  p99:        %-10lu ns                  │\n"
        "│  p99.9:      %-10lu ns                  │\n"
        "│  p99.99:     %-10lu ns                  │\n"
        "│  Max (OWCET):%-10lu ns                  │\n"
        "└──────────────────────────────────────────────┘\n",
        name,
        results.num_samples,
        static_cast<double>(results.tsc_hz) / 1e9,
        results.min_ns,
        results.mean_ns, results.stddev_ns,
        results.median_ns,
        results.p90_ns,
        results.p95_ns,
        results.p99_ns,
        results.p999_ns,
        results.p9999_ns,
        results.max_ns
    );
}

} // namespace wcet
