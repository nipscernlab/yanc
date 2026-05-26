#!/usr/bin/env python3
"""test50 post-step: split the YANC sim output into 6 arrays, descale by
1e6, write them as text files, and compare against the user's hosted
reference outputs (03_reference_outputs/ on the user's tree).

The YANC float format is NOT IEEE-754 bit-compatible (see the
yanc-float-format note), so we relax the comparison tolerance compared
to the default the user's compare_outputs.py uses for two host runs.
"""

from __future__ import annotations

import argparse
import math
import sys
from pathlib import Path


# Output array sizes -- must match the order test50.cpp emits.
# y_input + h_hat + x_hat + y_hat + g + x_tilde
# WINDOW_LEN=100 (test-local reduction; user's source is 1000).
# X_LEN = WINDOW_LEN - M_PULSE + 1 = 100 - 15 + 1 = 86.
ARRAYS = [
    ("y_input",  100),
    ("h_hat",     15),
    ("x_hat",     86),
    ("y_hat",    100),
    ("g",         15),
    ("x_tilde",  100),
    # localizer probe arrays (test50-only): each starts as a 1-element scalar
    # then the 3 inverse-FIR intermediates. delta_pos is an integer; the YANC
    # driver scales it by GAIN_OUT so descaling here gives the integer back.
    ("delta_pos",      1),
    ("solve_ok",       1),
    ("probe_A_row_0", 15),
    ("probe_b_orig",  15),
    ("probe_g_raw",   15),
    # Stage-2 probe: re-do the calibration externally in the test driver,
    # see whether a parallel implementation gets the same wrong g or the
    # correct one. probe_g_calibrated should match host's g.txt up to
    # rounding if the calibration math itself works.
    ("probe_gh",      29),
    ("probe_peak",     1),
    ("probe_pk",       1),
    ("probe_g_calibrated", 15),
]
EXPECTED_TOTAL = sum(n for _, n in ARRAYS)   # 509


def read_int_lines(path: Path) -> list[int]:
    out = []
    with path.open("r", encoding="utf-8") as f:
        for ln, line in enumerate(f, start=1):
            text = line.strip()
            if not text:
                continue
            try:
                out.append(int(text))
            except ValueError as exc:
                raise ValueError(f"{path}:{ln}: not an int: {text}") from exc
    return out


def read_float_lines(path: Path) -> list[float]:
    out = []
    with path.open("r", encoding="utf-8") as f:
        for ln, line in enumerate(f, start=1):
            text = line.strip()
            if not text:
                continue
            try:
                # Reference files may use "1.234e-5" or "1.234e-5, ..."; first column.
                out.append(float(text.split(",")[0]))
            except ValueError as exc:
                raise ValueError(f"{path}:{ln}: not a float: {text}") from exc
    return out


def compare(ref: list[float], cand: list[float]) -> tuple[float, float, float]:
    if len(ref) != len(cand):
        raise ValueError(f"length mismatch: {len(ref)} vs {len(cand)}")
    max_abs = 0.0
    max_rel = 0.0
    sum_sq = 0.0
    for a, b in zip(ref, cand):
        diff = abs(a - b)
        scale = max(abs(a), abs(b), 1.0)
        max_abs = max(max_abs, diff)
        max_rel = max(max_rel, diff / scale)
        sum_sq += diff * diff
    rmse = math.sqrt(sum_sq / len(ref)) if ref else 0.0
    return max_abs, max_rel, rmse


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("yanc_output", type=Path,
                    help="output_0.txt from CPPComp/.work/test50/Simulation/")
    ap.add_argument("reference_dir", type=Path,
                    help="directory with reference {y_input,h_hat,...}.txt")
    ap.add_argument("--gain-out", type=float, default=1e6,
                    help="GAIN_OUT used in test50.cpp (default 1e6)")
    ap.add_argument("--abs-tol", type=float, default=1e-2,
                    help="absolute tolerance (default 1e-2 -- relaxed for YANC float drift)")
    ap.add_argument("--rel-tol", type=float, default=5e-2,
                    help="relative tolerance (default 5e-2)")
    ap.add_argument("--write-dir", type=Path, default=None,
                    help="if given, write descaled YANC arrays as text files here")
    args = ap.parse_args()

    ints = read_int_lines(args.yanc_output)
    if len(ints) != EXPECTED_TOTAL:
        print(f"FAIL: yanc output has {len(ints)} lines, expected {EXPECTED_TOTAL}")
        return 1

    if args.write_dir:
        args.write_dir.mkdir(parents=True, exist_ok=True)

    print(f"{'array':10s} {'n':>6s} {'max_abs':>12s} {'max_rel':>12s} {'rmse':>12s}  status")
    print("-" * 70)
    failed = False
    cursor = 0
    for name, n in ARRAYS:
        chunk_int = ints[cursor:cursor + n]
        cursor += n
        yanc_vals = [v / args.gain_out for v in chunk_int]

        if args.write_dir:
            with (args.write_dir / f"{name}.txt").open("w", encoding="utf-8") as f:
                for v in yanc_vals:
                    f.write(f"{v:.6e}\n")

        ref_path = args.reference_dir / f"{name}.txt"
        try:
            ref_vals = read_float_lines(ref_path)
            max_abs, max_rel, rmse = compare(ref_vals, yanc_vals)
            ok = (max_abs <= args.abs_tol) or (max_rel <= args.rel_tol)
            status = "OK" if ok else "FAIL"
            if not ok:
                failed = True
            print(f"{name:10s} {n:6d} {max_abs:12.5e} {max_rel:12.5e} {rmse:12.5e}  {status}")
        except Exception as exc:
            failed = True
            print(f"{name:10s} {n:6d} {'-':>12s} {'-':>12s} {'-':>12s}  FAIL ({exc})")

    print("-" * 70)
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
