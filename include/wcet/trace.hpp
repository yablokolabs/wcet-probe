#pragma once
/// @file trace.hpp
/// Binary trace file I/O for probe samples.

#include "wcet/probe.hpp"
#include <cstdio>
#include <vector>
#include <cstdint>

namespace wcet {

/// Trace file header.
struct TraceHeader {
    char     magic[8];        ///< "WCETPROB"
    std::uint32_t version;    ///< Format version (1)
    std::uint32_t sample_size;///< sizeof(ProbeSample)
    std::uint64_t tsc_hz;     ///< TSC frequency for time conversion
    std::uint64_t num_samples;///< Number of samples in file
    std::uint64_t _reserved[2];
};

static_assert(sizeof(TraceHeader) == 48);

/// Write probe samples to a binary trace file.
/// Returns number of samples written.
std::size_t write_trace(const char* path,
                        const std::vector<ProbeSample>& samples,
                        std::uint64_t tsc_hz) noexcept;

/// Read probe samples from a binary trace file.
/// Returns true on success.
bool read_trace(const char* path,
                std::vector<ProbeSample>& samples,
                std::uint64_t& tsc_hz) noexcept;

} // namespace wcet
