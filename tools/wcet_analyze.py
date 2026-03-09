#!/usr/bin/env python3
"""
wcet_analyze.py — Parse WCET trace files and compute statistics.

Usage:
    python3 wcet_analyze.py trace.wcet [--histogram] [--report report.html]
"""

import argparse
import struct
import sys
from pathlib import Path

HEADER_FMT = "<8sII QQ 16s"  # magic, version, sample_size, tsc_hz, num_samples, reserved
HEADER_SIZE = 48
SAMPLE_FMT = "<QQQ II"  # start_tsc, end_tsc, elapsed_tsc, probe_id, pad
SAMPLE_SIZE = 32


def read_trace(path: str) -> tuple[list[dict], int]:
    """Read a .wcet trace file. Returns (samples, tsc_hz)."""
    with open(path, "rb") as f:
        hdr = f.read(HEADER_SIZE)
        if len(hdr) < HEADER_SIZE:
            print(f"Error: file too small", file=sys.stderr)
            sys.exit(1)

        magic = hdr[:8]
        if magic != b"WCETPROB":
            print(f"Error: not a WCET trace file", file=sys.stderr)
            sys.exit(1)

        version = struct.unpack_from("<I", hdr, 8)[0]
        tsc_hz = struct.unpack_from("<Q", hdr, 16)[0]
        num_samples = struct.unpack_from("<Q", hdr, 24)[0]

        samples = []
        for _ in range(num_samples):
            data = f.read(SAMPLE_SIZE)
            if len(data) < SAMPLE_SIZE:
                break
            start, end, elapsed, probe_id, _ = struct.unpack(SAMPLE_FMT, data)
            samples.append({
                "start_tsc": start,
                "end_tsc": end,
                "elapsed_tsc": elapsed,
                "probe_id": probe_id,
                "elapsed_ns": (elapsed * 1_000_000_000) // tsc_hz if tsc_hz else 0,
            })

    return samples, tsc_hz


def percentile(sorted_vals: list, p: float) -> float:
    idx = int(len(sorted_vals) * p)
    idx = min(idx, len(sorted_vals) - 1)
    return sorted_vals[idx]


def analyze(samples: list[dict], tsc_hz: int):
    """Print statistical analysis."""
    if not samples:
        print("No samples to analyze.")
        return

    elapsed = sorted([s["elapsed_ns"] for s in samples])
    n = len(elapsed)

    mean = sum(elapsed) / n
    variance = sum((x - mean) ** 2 for x in elapsed) / n
    stddev = variance ** 0.5

    print(f"WCET Analysis Report")
    print(f"{'=' * 50}")
    print(f"  Samples:     {n:,}")
    print(f"  TSC freq:    {tsc_hz / 1e9:.2f} GHz")
    print(f"{'─' * 50}")
    print(f"  Min:         {elapsed[0]:>10,} ns")
    print(f"  Mean:        {mean:>10,.0f} ns  (σ={stddev:,.0f} ns)")
    print(f"  Median:      {percentile(elapsed, 0.50):>10,} ns")
    print(f"  p90:         {percentile(elapsed, 0.90):>10,} ns")
    print(f"  p95:         {percentile(elapsed, 0.95):>10,} ns")
    print(f"  p99:         {percentile(elapsed, 0.99):>10,} ns")
    print(f"  p99.9:       {percentile(elapsed, 0.999):>10,} ns")
    print(f"  p99.99:      {percentile(elapsed, 0.9999):>10,} ns")
    print(f"  Max (OWCET): {elapsed[-1]:>10,} ns")
    print(f"{'=' * 50}")


def histogram(samples: list[dict], bins: int = 40):
    """Print ASCII histogram."""
    elapsed = [s["elapsed_ns"] for s in samples]
    if not elapsed:
        return

    lo, hi = min(elapsed), max(elapsed)
    if lo == hi:
        print(f"All samples: {lo} ns")
        return

    step = (hi - lo) / bins
    counts = [0] * bins
    for v in elapsed:
        idx = min(int((v - lo) / step), bins - 1)
        counts[idx] += 1

    max_count = max(counts)
    bar_width = 50

    print(f"\nHistogram ({len(elapsed):,} samples):")
    print(f"{'─' * 70}")
    for i, c in enumerate(counts):
        lo_ns = lo + i * step
        bar_len = int(c / max_count * bar_width) if max_count else 0
        bar = "█" * bar_len
        print(f"  {lo_ns:>10,.0f} ns │{bar} ({c:,})")
    print(f"{'─' * 70}")


def main():
    parser = argparse.ArgumentParser(description="WCET trace analyzer")
    parser.add_argument("trace", help="Path to .wcet trace file")
    parser.add_argument("--histogram", action="store_true", help="Show histogram")
    args = parser.parse_args()

    samples, tsc_hz = read_trace(args.trace)
    analyze(samples, tsc_hz)
    if args.histogram:
        histogram(samples)


if __name__ == "__main__":
    main()
