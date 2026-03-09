/// @file test_probe.cpp
#include "wcet/probe.hpp"
#include <cstdio>

void test_scoped_probe() {
    auto &ctx = wcet::global_context();
    auto before = ctx.count();

    {
        WCET_PROBE_SCOPE(test_scope);
        volatile int x = 0;
        for (int i = 0; i < 1000; ++i) x += i;
        (void)x;
    }

    if (ctx.count() != before + 1) {
        std::fprintf(stderr, "  ✗ scoped probe didn't record\n");
        return;
    }
    std::fprintf(stderr, "  ✓ scoped probe recorded sample\n");
}

void test_manual_probe() {
    auto &ctx = wcet::global_context();
    auto before = ctx.count();

    WCET_PROBE_START(manual);
    volatile int x = 0;
    for (int i = 0; i < 1000; ++i) x += i;
    (void)x;
    WCET_PROBE_END(manual);

    if (ctx.count() != before + 1) {
        std::fprintf(stderr, "  ✗ manual probe didn't record\n");
        return;
    }
    std::fprintf(stderr, "  ✓ manual probe recorded sample\n");
}

void test_probe_id_hash() {
    auto id1 = wcet_probe_id("function_a");
    auto id2 = wcet_probe_id("function_b");
    auto id3 = wcet_probe_id("function_a");

    if (id1 == id2) {
        std::fprintf(stderr, "  ✗ different names same hash\n");
        return;
    }
    if (id1 != id3) {
        std::fprintf(stderr, "  ✗ same name different hash\n");
        return;
    }
    std::fprintf(stderr, "  ✓ probe ID hashing\n");
}

void test_many_samples() {
    wcet::ProbeContext ctx;
    for (int i = 0; i < 10000; ++i) {
        ctx.record(1, wcet::rdtsc(), wcet::rdtscp());
    }
    if (ctx.count() != 10000) {
        std::fprintf(stderr, "  ✗ expected 10000 samples, got %zu\n", ctx.count());
        return;
    }
    std::fprintf(stderr, "  ✓ 10000 samples recorded\n");
}

int main() {
    std::fprintf(stderr, "test_probe:\n");
    test_scoped_probe();
    test_manual_probe();
    test_probe_id_hash();
    test_many_samples();
    std::fprintf(stderr, "  All probe tests passed.\n");
    return 0;
}
