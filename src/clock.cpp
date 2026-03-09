/// @file clock.cpp

#include "wcet/clock.hpp"
#include <thread>
#include <chrono>

namespace wcet {

std::uint64_t calibrate_tsc_hz() noexcept {
    auto t1 = rdtsc();
    auto wall1 = now_ns();

    // Sleep 50ms for calibration
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    auto t2 = rdtsc();
    auto wall2 = now_ns();

    auto tsc_delta = t2 - t1;
    auto ns_delta = static_cast<std::uint64_t>(wall2 - wall1);

    if (ns_delta == 0) return 1'000'000'000ULL; // fallback: assume 1GHz

    return (tsc_delta * 1'000'000'000ULL) / ns_delta;
}

} // namespace wcet
