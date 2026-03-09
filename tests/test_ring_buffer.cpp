/// @file test_ring_buffer.cpp
#include "wcet/ring_buffer.hpp"
#include <cstdio>

void test_push_and_read() {
    wcet::RingBuffer<int> rb(4);
    rb.push(10);
    rb.push(20);
    rb.push(30);

    if (rb.size() != 3) { std::fprintf(stderr, "  ✗ size\n"); return; }
    if (rb[0] != 10) { std::fprintf(stderr, "  ✗ rb[0]\n"); return; }
    if (rb[2] != 30) { std::fprintf(stderr, "  ✗ rb[2]\n"); return; }
    std::fprintf(stderr, "  ✓ push and read\n");
}

void test_overwrite() {
    wcet::RingBuffer<int> rb(3);
    rb.push(1); rb.push(2); rb.push(3); rb.push(4); rb.push(5);

    if (rb.size() != 3) { std::fprintf(stderr, "  ✗ size after overwrite\n"); return; }
    if (rb[0] != 3) { std::fprintf(stderr, "  ✗ oldest should be 3, got %d\n", rb[0]); return; }
    if (rb[2] != 5) { std::fprintf(stderr, "  ✗ newest should be 5, got %d\n", rb[2]); return; }
    std::fprintf(stderr, "  ✓ overwrite oldest\n");
}

void test_drain() {
    wcet::RingBuffer<int> rb(4);
    rb.push(10); rb.push(20); rb.push(30);

    std::vector<int> out;
    rb.drain(out);
    if (out.size() != 3) { std::fprintf(stderr, "  ✗ drain size\n"); return; }
    if (out[0] != 10 || out[1] != 20 || out[2] != 30) {
        std::fprintf(stderr, "  ✗ drain order\n"); return;
    }
    std::fprintf(stderr, "  ✓ drain\n");
}

void test_clear() {
    wcet::RingBuffer<int> rb(4);
    rb.push(1); rb.push(2);
    rb.clear();
    if (!rb.empty()) { std::fprintf(stderr, "  ✗ not empty after clear\n"); return; }
    std::fprintf(stderr, "  ✓ clear\n");
}

int main() {
    std::fprintf(stderr, "test_ring_buffer:\n");
    test_push_and_read();
    test_overwrite();
    test_drain();
    test_clear();
    std::fprintf(stderr, "  All ring buffer tests passed.\n");
    return 0;
}
