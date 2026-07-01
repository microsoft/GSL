#!/usr/bin/env python3
"""
benchmark/check_regression.py

Reads one or more Google Benchmark JSON result files (produced with
--benchmark_report_aggregates_only=true), pairs up gsl::span and std::span
benchmarks by name, computes the ratio gsl_ns / std_ns, and writes a
Markdown report to --output (default: stdout).

Exits with code 1 if any ratio exceeds the threshold so CI fails cleanly.

Usage:
    python3 check_regression.py [--threshold 0.15] [--output report.md] results_*.json
"""

import argparse
import json
import os
import sys
from pathlib import Path


# ─── helpers ──────────────────────────────────────────────────────────────────

def load_json(path: str) -> dict:
    with open(path) as f:
        return json.load(f)


def label_from_filename(path: str) -> str:
    """
    'results_GCC-14-cpp20.json'  →  'GCC-14-cpp20'
    Falls back to the bare filename stem if the prefix isn't there.
    """
    stem = Path(path).stem           # e.g. 'results_GCC-14-cpp20'
    if stem.startswith("results_"):
        return stem[len("results_"):]
    return stem


def parse_benchmarks(data: dict) -> dict[str, dict]:
    """
    Returns a dict keyed by benchmark name.
    For aggregated runs Google Benchmark emits multiple rows per benchmark
    (_mean, _median, _stddev, _cv).  We pull out mean and stddev.

        { "BM_IsSorted_StdSpan": {"mean": 124.3, "stddev": 2.1}, ... }
    """
    result: dict[str, dict] = {}
    for bm in data.get("benchmarks", []):
        name: str = bm["name"]           # e.g. "BM_IsSorted_StdSpan_mean"
        agg  = bm.get("aggregate_name")  # "mean" | "stddev" | "median" | "cv"

        if agg not in ("mean", "stddev"):
            continue

        # Strip the trailing _mean / _stddev to get the canonical name
        suffix = f"_{agg}"
        assert name.endswith(suffix)
        base_name = name[:-len(suffix)]

        entry = result.setdefault(base_name, {"mean": None, "stddev": None})
        entry[agg] = bm.get("real_time") or bm.get("cpu_time", 0.0)

    return result


def pair_benchmarks(bm_dict: dict[str, dict]):
    """
    Match GslSpan variants with their StdSpan counterparts.

    Actual names from span_bench.cpp:
        BM_IsSortedStdSpan              ↔  BM_IsSortedGslSpan
        BM_IsSortedRangesStdSpan        ↔  BM_IsSortedRangesGslSpan
        BM_IsSortedCustomStdSpan        ↔  BM_IsSortedCustomGslSpan
        BM_MinElementAlgorithmStdSpan   ↔  BM_MinElementAlgorithmGslSpan
        BM_MinElementRangeForStdSpan    ↔  BM_MinElementRangeForGslSpan

    All std variants contain 'StdSpan'; swapping it for 'GslSpan' gives
    the paired name. Short display name strips 'BM_' prefix and 'StdSpan'
    infix, leaving e.g. 'IsSortedRanges', 'MinElementAlgorithm'.
    """
    pairs = []
    for name, vals in sorted(bm_dict.items()):
        if "StdSpan" not in name:
            continue
        gsl_name = name.replace("StdSpan", "GslSpan")
        if gsl_name not in bm_dict:
            continue

        # Strip BM_ prefix and StdSpan infix for a clean display name.
        # e.g. "BM_IsSortedRangesStdSpan" → "IsSortedRanges"
        short = name.removeprefix("BM_").replace("StdSpan", "").rstrip("_")

        pairs.append({
            "short":      short,
            "std_name":   name,
            "gsl_name":   gsl_name,
            "std_mean":   bm_dict[name]["mean"]       or 0.0,
            "std_stddev": bm_dict[name]["stddev"]     or 0.0,
            "gsl_mean":   bm_dict[gsl_name]["mean"]   or 0.0,
            "gsl_stddev": bm_dict[gsl_name]["stddev"] or 0.0,
        })
    return pairs


def ratio_and_status(gsl: float, std: float, threshold: float):
    if std == 0:
        return None, "⚠️ div/0"
    r = gsl / std
    if r > 1 + threshold:
        return r, f"🔴 **{r:.2f}×** regression"
    if r < 1 - threshold:
        return r, f"🟢 **{r:.2f}×** faster"
    return r, f"✅ {r:.2f}×"


def fmt(ns: float) -> str:
    """Format nanoseconds nicely."""
    if ns >= 1_000_000:
        return f"{ns/1_000_000:.1f} ms"
    if ns >= 1_000:
        return f"{ns/1_000:.1f} µs"
    return f"{ns:.1f} ns"


def fmt_stddev(stddev: float, mean: float) -> str:
    if mean == 0:
        return "—"
    pct = (stddev / mean) * 100
    return f"±{pct:.1f}%"


# ─── report builder ───────────────────────────────────────────────────────────

def build_report(json_paths: list[str], threshold: float) -> tuple[str, bool]:
    """
    Returns (markdown_text, had_regression).
    """
    lines = []
    had_regression = False

    lines.append("<!-- span-bench-report -->")
    lines.append("## 📊 `gsl::span` vs `std::span` benchmark results")
    lines.append("")
    lines.append(
        f"> Threshold: flag if `gsl_ns / std_ns > {1 + threshold:.2f}` "
        f"(+{threshold*100:.0f}%) or `< {1 - threshold:.2f}` "
        f"(-{threshold*100:.0f}%)  \n"
        f"> Each benchmark ran **10 repetitions**; times shown are the mean "
        f"± stddev."
    )
    lines.append("")

    found_any = False

    for path in sorted(json_paths):
        if not os.path.exists(path):
            lines.append(f"> ⚠️ Result file not found: `{path}`")
            continue

        try:
            data = load_json(path)
        except Exception as e:
            lines.append(f"> ⚠️ Could not parse `{path}`: {e}")
            continue

        config_label = label_from_filename(path)
        bm_dict = parse_benchmarks(data)
        pairs   = pair_benchmarks(bm_dict)

        if not pairs:
            lines.append(f"### `{config_label}`")
            lines.append("> ⚠️ No paired benchmarks found — check naming convention.")
            lines.append("")
            continue

        found_any = True

        # Extract context info from the JSON if present
        ctx = data.get("context", {})
        # cpu_scaling_enabled=True means the OS was allowed to vary frequency
        # — bad for stable benchmarks. Google Benchmark warns about this itself
        # but we surface it in the report too.
        cpu_scaling = ctx.get("cpu_scaling_enabled", None)
        num_cpus    = ctx.get("num_cpus", "?")
        mhz         = ctx.get("mhz_per_cpu", "?")

        lines.append(f"### `{config_label}`")
        lines.append(f"<sup>CPUs: {num_cpus} @ {mhz} MHz</sup>")
        lines.append("")

        if cpu_scaling is True:
            lines.append("> ⚠️ CPU frequency scaling was **enabled** — results may be noisier than usual.")

        lines.append("")
        lines.append(
            "| Benchmark | std mean | std σ | gsl mean | gsl σ | ratio | status |"
        )
        lines.append(
            "|-----------|----------|-------|----------|-------|-------|--------|"
        )

        config_regression = False
        for p in pairs:
            ratio, status = ratio_and_status(p["gsl_mean"], p["std_mean"], threshold)
            ratio_str = "—" if ratio is None else f"{ratio:.2f}×"
            if "regression" in status:
                had_regression = True
                config_regression = True

            lines.append(
                f"| `{p['short']}` "
                f"| {fmt(p['std_mean'])} "
                f"| {fmt_stddev(p['std_stddev'], p['std_mean'])} "
                f"| {fmt(p['gsl_mean'])} "
                f"| {fmt_stddev(p['gsl_stddev'], p['gsl_mean'])} "
                f"| {ratio_str} "
                f"| {status} |"
            )

        if config_regression:
            lines.append("")
            lines.append(
                f"> 🔴 **Regression detected** in `{config_label}` — "
                f"`gsl::span` is more than {threshold*100:.0f}% slower than "
                f"`std::span` on one or more benchmarks."
            )

        lines.append("")

    if not found_any:
        lines.append("> ❌ No benchmark results could be loaded.")

    return "\n".join(lines), had_regression


# ─── entry point ─────────────────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser(
        description="Check gsl::span vs std::span benchmark regression."
    )
    parser.add_argument(
        "json_files",
        nargs="+",
        metavar="results.json",
        help="Google Benchmark JSON output files (one per CI matrix config).",
    )
    parser.add_argument(
        "--threshold",
        type=float,
        default=0.15,
        help="Fractional slowdown threshold before flagging a regression (default: 0.15 = 15%%).",
    )
    parser.add_argument(
        "--output",
        default=None,
        metavar="FILE",
        help="Write Markdown report to FILE instead of stdout.",
    )
    args = parser.parse_args()

    report, had_regression = build_report(args.json_files, args.threshold)

    if args.output:
        Path(args.output).write_text(report, encoding="utf-8")
        print(f"Report written to {args.output}")
    else:
        print(report)

    if had_regression:
        print(
            "\n[check_regression] ❌ Performance regression detected. "
            "See the report above.",
            file=sys.stderr,
        )
        sys.exit(1)

    print("\n[check_regression] ✅ No regressions detected.", file=sys.stderr)
    sys.exit(0)


if __name__ == "__main__":
    main()