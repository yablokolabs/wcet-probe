#pragma once
/// @file cpu_control.hpp
/// CPU control utilities for worst-case provocation.
/// Pin to core, set governor, flush caches.

#include <cstdint>

namespace wcet {

/// Pin the current thread to a specific CPU core.
/// Returns true on success.
bool pin_to_cpu(int cpu) noexcept;

/// Set CPU frequency governor (requires root).
/// governor: "performance", "powersave", "userspace", etc.
/// Returns true on success.
bool set_governor(int cpu, const char *governor) noexcept;

/// Flush L1/L2/L3 data caches by reading a large array.
/// This forces subsequent code to fetch from memory, provoking worst-case latency.
void flush_data_cache() noexcept;

/// Flush instruction cache (x86: not directly possible, use large code traversal).
/// On ARM: explicit icache flush.
void flush_instruction_cache() noexcept;

/// Flush TLB by touching many pages.
void flush_tlb() noexcept;

/// Combined: flush all caches + TLB.
void flush_all() noexcept;

/// Disable hardware prefetchers via MSR (requires root + msr module).
/// Returns true on success.
bool disable_prefetchers() noexcept;

/// Re-enable hardware prefetchers.
bool enable_prefetchers() noexcept;

} // namespace wcet
