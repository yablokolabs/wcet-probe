#pragma once
/// @file probe.hpp
/// Zero-overhead instrumentation macros for WCET measurement.
/// Usage:
///   WCET_PROBE_START(my_function);
///   // ... code under test ...
///   WCET_PROBE_END(my_function);

#include "wcet/clock.hpp"
#include "wcet/ring_buffer.hpp"

namespace wcet {

/// Probe measurement record.
struct ProbeSample {
    std::uint64_t start_tsc;    ///< TSC at probe start
    std::uint64_t end_tsc;      ///< TSC at probe end
    std::uint64_t elapsed_tsc;  ///< end - start
    std::uint32_t probe_id;     ///< Identifies the probe point
    std::uint32_t _pad;         ///< Padding to 32 bytes
};

static_assert(sizeof(ProbeSample) == 32);

/// Global probe storage. Thread-local for zero contention.
/// Each thread gets its own ring buffer.
class ProbeContext {
public:
    static constexpr std::size_t BUFFER_CAPACITY = 1 << 20; // 1M samples = 32MB

    ProbeContext() noexcept : buffer_(BUFFER_CAPACITY), count_(0) {}

    /// Record a completed measurement.
    void record(std::uint32_t probe_id, std::uint64_t start, std::uint64_t end) noexcept {
        ProbeSample sample;
        sample.start_tsc   = start;
        sample.end_tsc     = end;
        sample.elapsed_tsc = end - start;
        sample.probe_id    = probe_id;
        sample._pad        = 0;
        buffer_.push(sample);
        ++count_;
    }

    /// Get buffer for reading.
    [[nodiscard]] const RingBuffer<ProbeSample>& buffer() const noexcept { return buffer_; }
    [[nodiscard]] RingBuffer<ProbeSample>& buffer() noexcept { return buffer_; }

    /// Total samples recorded.
    [[nodiscard]] std::size_t count() const noexcept { return count_; }

private:
    RingBuffer<ProbeSample> buffer_;
    std::size_t count_;
};

/// Get the thread-local probe context.
ProbeContext& thread_context() noexcept;

/// Get/set the global default probe context (for single-threaded use).
ProbeContext& global_context() noexcept;

/// RAII probe: measures elapsed time in constructor/destructor.
class ScopedProbe {
public:
    explicit ScopedProbe(std::uint32_t probe_id, ProbeContext& ctx) noexcept
        : probe_id_(probe_id), ctx_(ctx), start_(rdtsc()) {}

    ~ScopedProbe() noexcept {
        auto end = rdtscp();
        ctx_.record(probe_id_, start_, end);
    }

    ScopedProbe(const ScopedProbe&) = delete;
    ScopedProbe& operator=(const ScopedProbe&) = delete;

private:
    std::uint32_t probe_id_;
    ProbeContext& ctx_;
    std::uint64_t start_;
};

} // namespace wcet

/// Compile-time probe ID from string hash.
consteval std::uint32_t wcet_probe_id(const char* s) {
    std::uint32_t h = 2166136261u;
    while (*s) {
        h ^= static_cast<std::uint32_t>(*s++);
        h *= 16777619u;
    }
    return h;
}

/// Start a measurement probe (manual start/end).
#define WCET_PROBE_START(name) \
    auto _wcet_start_##name = ::wcet::rdtsc()

/// End a measurement probe and record the sample.
#define WCET_PROBE_END(name) \
    do { \
        auto _wcet_end_##name = ::wcet::rdtscp(); \
        ::wcet::global_context().record( \
            wcet_probe_id(#name), _wcet_start_##name, _wcet_end_##name); \
    } while(0)

/// Scoped probe: measures from construction to end of scope.
#define WCET_PROBE_SCOPE(name) \
    ::wcet::ScopedProbe _wcet_scoped_##name(wcet_probe_id(#name), ::wcet::global_context())
