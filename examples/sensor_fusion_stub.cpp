/// @file sensor_fusion_stub.cpp
/// Example: WCET analysis of a simulated sensor fusion pipeline.

#include "wcet/harness.hpp"
#include "wcet/probe.hpp"
#include <cmath>
#include <cstdio>

static double sensors[256];
static double weights[256];
static double output[4];

static void init_data() {
    for (int i = 0; i < 256; ++i) {
        sensors[i] = std::sin(static_cast<double>(i) * 0.1) * 100.0;
        weights[i] = 1.0 / (1.0 + static_cast<double>(i) * 0.01);
    }
}

static void sensor_fusion() {
    double wsum = 0, ax = 0, ay = 0, az = 0;

    for (int i = 0; i < 256; ++i) {
        double w = weights[i];
        double s = sensors[i];
        wsum += w;
        ax += s * w * std::cos(static_cast<double>(i) * 0.05);
        ay += s * w * std::sin(static_cast<double>(i) * 0.05);
        az += s * w * 0.5;
    }

    if (wsum > 0) {
        output[0] = ax / wsum;
        output[1] = ay / wsum;
        output[2] = az / wsum;
    }
    output[3] = wsum;
}

int main() {
    init_data();

    wcet::HarnessConfig config;
    config.iterations = 100000;
    config.warmup = 1000;
    config.trace_path = "sensor_fusion.wcet";

    std::fprintf(stderr, "Measuring sensor fusion (256 sensors, %lu iterations)...\n\n",
                 config.iterations);

    auto results = wcet::measure(config, sensor_fusion);
    wcet::print_results(results, "sensor_fusion_256");

    std::fprintf(stderr, "\nTrace written to: sensor_fusion.wcet\n");
    return 0;
}
