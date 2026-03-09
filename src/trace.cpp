/// @file trace.cpp

#include "wcet/trace.hpp"
#include <cstring>

namespace wcet {

std::size_t write_trace(const char *path, const std::vector<ProbeSample> &samples,
                        std::uint64_t tsc_hz) noexcept {
    FILE *f = std::fopen(path, "wb");
    if (!f) return 0;

    TraceHeader hdr{};
    std::memcpy(hdr.magic, "WCETPROB", 8);
    hdr.version = 1;
    hdr.sample_size = sizeof(ProbeSample);
    hdr.tsc_hz = tsc_hz;
    hdr.num_samples = samples.size();

    std::fwrite(&hdr, sizeof(hdr), 1, f);
    std::fwrite(samples.data(), sizeof(ProbeSample), samples.size(), f);
    std::fclose(f);

    return samples.size();
}

bool read_trace(const char *path, std::vector<ProbeSample> &samples,
                std::uint64_t &tsc_hz) noexcept {
    FILE *f = std::fopen(path, "rb");
    if (!f) return false;

    TraceHeader hdr{};
    if (std::fread(&hdr, sizeof(hdr), 1, f) != 1) {
        std::fclose(f);
        return false;
    }

    if (std::memcmp(hdr.magic, "WCETPROB", 8) != 0 || hdr.version != 1) {
        std::fclose(f);
        return false;
    }

    tsc_hz = hdr.tsc_hz;
    samples.resize(hdr.num_samples);
    auto read = std::fread(samples.data(), sizeof(ProbeSample), hdr.num_samples, f);
    std::fclose(f);

    samples.resize(read);
    return read == hdr.num_samples;
}

} // namespace wcet
