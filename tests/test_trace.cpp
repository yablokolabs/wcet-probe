/// @file test_trace.cpp
#include "wcet/trace.hpp"
#include <cstdio>
#include <cstdlib>

void test_write_read_roundtrip() {
    const char* path = "/tmp/wcet_test_trace.bin";

    std::vector<wcet::ProbeSample> samples;
    for (int i = 0; i < 100; ++i) {
        wcet::ProbeSample s{};
        s.start_tsc = static_cast<std::uint64_t>(i * 1000);
        s.end_tsc = static_cast<std::uint64_t>(i * 1000 + 500);
        s.elapsed_tsc = 500;
        s.probe_id = 42;
        samples.push_back(s);
    }

    auto written = wcet::write_trace(path, samples, 3'000'000'000ULL);
    if (written != 100) { std::fprintf(stderr, "  ✗ wrote %zu, expected 100\n", written); return; }

    std::vector<wcet::ProbeSample> loaded;
    std::uint64_t tsc_hz = 0;
    bool ok = wcet::read_trace(path, loaded, tsc_hz);

    if (!ok) { std::fprintf(stderr, "  ✗ read failed\n"); return; }
    if (loaded.size() != 100) { std::fprintf(stderr, "  ✗ loaded %zu\n", loaded.size()); return; }
    if (tsc_hz != 3'000'000'000ULL) { std::fprintf(stderr, "  ✗ tsc_hz mismatch\n"); return; }
    if (loaded[0].probe_id != 42) { std::fprintf(stderr, "  ✗ probe_id mismatch\n"); return; }
    if (loaded[50].elapsed_tsc != 500) { std::fprintf(stderr, "  ✗ elapsed mismatch\n"); return; }

    std::remove(path);
    std::fprintf(stderr, "  ✓ write/read roundtrip (100 samples)\n");
}

void test_bad_file() {
    std::vector<wcet::ProbeSample> loaded;
    std::uint64_t tsc_hz = 0;
    bool ok = wcet::read_trace("/tmp/nonexistent_wcet_file.bin", loaded, tsc_hz);
    if (ok) { std::fprintf(stderr, "  ✗ should fail on missing file\n"); return; }
    std::fprintf(stderr, "  ✓ bad file returns false\n");
}

int main() {
    std::fprintf(stderr, "test_trace:\n");
    test_write_read_roundtrip();
    test_bad_file();
    std::fprintf(stderr, "  All trace tests passed.\n");
    return 0;
}
