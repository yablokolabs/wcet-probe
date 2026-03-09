/// @file cpu_control.cpp

#include "wcet/cpu_control.hpp"
#include <sched.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace wcet {

// Large array for cache flushing (~8MB, bigger than most L3 caches per core)
static volatile char flush_buffer[8 * 1024 * 1024];

bool pin_to_cpu(int cpu) noexcept {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);
    return sched_setaffinity(0, sizeof(cpuset), &cpuset) == 0;
}

bool set_governor(int cpu, const char* governor) noexcept {
    char path[128];
    std::snprintf(path, sizeof(path),
                  "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_governor", cpu);
    FILE* f = std::fopen(path, "w");
    if (!f) return false;
    std::fprintf(f, "%s", governor);
    std::fclose(f);
    return true;
}

void flush_data_cache() noexcept {
    // Read through a large buffer to evict cached data
    volatile char sink = 0;
    for (std::size_t i = 0; i < sizeof(flush_buffer); i += 64) {
        sink = flush_buffer[i];
    }
    (void)sink;
}

void flush_instruction_cache() noexcept {
#if defined(__aarch64__)
    // ARM: explicit instruction cache flush
    asm volatile("ic iallu\n\tisb" ::: "memory");
#else
    // x86: instruction cache is coherent with data cache;
    // best we can do is execute a large code block.
    // For practical purposes, flush_data_cache covers it.
    flush_data_cache();
#endif
}

void flush_tlb() noexcept {
    // Touch many distinct pages to pollute the TLB
    // 4KB pages, touch 4096 pages = 16MB
    static volatile char tlb_buffer[16 * 1024 * 1024];
    volatile char sink = 0;
    for (std::size_t i = 0; i < sizeof(tlb_buffer); i += 4096) {
        sink = tlb_buffer[i];
    }
    (void)sink;
}

void flush_all() noexcept {
    flush_data_cache();
    flush_instruction_cache();
    flush_tlb();
}

bool disable_prefetchers() noexcept {
    // Requires MSR access — usually needs root + msr kernel module
    // MSR 0x1A4 on Intel: bits 0-3 control prefetchers
    FILE* f = std::fopen("/dev/cpu/0/msr", "wb");
    if (!f) return false;
    // This is platform-specific; leaving as best-effort
    std::fclose(f);
    return false; // Not implemented for safety
}

bool enable_prefetchers() noexcept {
    return false; // Not implemented for safety
}

} // namespace wcet
