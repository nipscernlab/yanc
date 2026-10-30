#!/usr/bin/env python3
"""proc_rls against an RLS computed apart from YANC.

Compilers/CMMComp/Tests/proc_rls runs 20 turns of an RLS filter of order 4
written in the Dirac notation and writes trunc(1000 w) each turn. This
computes the same filter in Python, in double precision, from the same
input files, and compares it with the fixture's recorded output. The SAPHO
float has a 23-bit mantissa, so the two could differ in the last bits and a
value near an integer truncate to the neighbouring one: the check allows a
difference of TOL in a word (measured on 2026-10-30: the largest is 0).

What it can see (measured on the same day, by changing the filter here): no
update of P gives a largest difference of 4348, P started at 10 instead of
1000 gives 653. What it cannot: the filter reaches its exact weights within
a few turns, so a forgetting factor of 1 instead of 1/0.99 gives 0, and so
does a transposed outer product, since K Px' is symmetric here (K = g Px).
Those are the job of dirac_shapes, whose values are not symmetric.

The recorded output is the regression's sim golden, so this checks that the
golden is the filter's, and the regression checks that the simulation still
gives the golden.

usage: python3 Scripts/rls_ref.py [repo-root]    (exit 1 on a difference)
"""
import math
import os
import sys

N, TURNS, TOL = 4, 20, 2


def f2i(v):
    return int(math.trunc(v))


def rls(xs, ds, transposed=False):
    x = [0.0] * N
    w = [0.0] * N
    P = [[1000.0 if r == c else 0.0 for c in range(N)] for r in range(N)]
    xs, ds = iter(xs), iter(ds)
    out = []
    for _ in range(TURNS):
        for _ in range(N):                                  # x # fin(0)*0.001 -> |x>
            x = [next(xs) * 0.001] + x[:-1]
        d = next(ds) * 0.001
        e = d - sum(w[k] * x[k] for k in range(N))
        Px = [sum(P[r][k] * x[k] for k in range(N)) for r in range(N)]
        g = 1.0 / (0.99 + sum(x[k] * Px[k] for k in range(N)))
        K = [g * v for v in Px]
        w = [w[k] + e * K[k] for k in range(N)]
        if transposed:
            P = [[P[r][c] - K[c] * Px[r] for c in range(N)] for r in range(N)]
        else:
            P = [[P[r][c] - K[r] * Px[c] for c in range(N)] for r in range(N)]
        P = [[v * 1.010101 for v in row] for row in P]
        out += [f2i(1000.0 * v) for v in w]
    return out


def main():
    root = sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
    t = os.path.join(root, 'Compilers', 'CMMComp', 'Tests', 'proc_rls')
    xs = [int(v) for v in open(os.path.join(t, 'input_0.txt')).read().split()]
    ds = [int(v) for v in open(os.path.join(t, 'input_1.txt')).read().split()]
    got = [int(v) for v in open(os.path.join(t, 'golden_sim', 'output_0.txt')).read().split()]
    ref = rls(xs, ds)
    if len(got) != len(ref):
        sys.exit('rls_ref: proc_rls wrote %d values, the filter %d' % (len(got), len(ref)))
    worst = max(abs(p - q) for p, q in zip(got, ref))
    if worst > TOL:
        k = next(i for i, (p, q) in enumerate(zip(got, ref)) if abs(p - q) > TOL)
        sys.exit('rls_ref: proc_rls is not the RLS filter: value %d (turn %d) is %d, the filter gives %d'
                 % (k, k // N + 1, got[k], ref[k]))
    print('rls_ref: proc_rls is the RLS filter (%d values, largest difference %d)' % (len(ref), worst))


if __name__ == '__main__':
    main()
