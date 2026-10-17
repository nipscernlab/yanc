#!/usr/bin/env python3
"""compare.py: lines up the host g++ answer (ref.txt) with cppcomp's
(out_fcond.txt) and cmmcomp's (out_fcond_l0/1/2.txt) for the cases of
cases.txt, and prints a table of the cases that differ. Exit 1 if any does."""
import os
import sys

d = sys.argv[1] if len(sys.argv) > 1 else '.'
cases = [l.split() for l in open(os.path.join(d, 'cases.txt'))]
ref = open(os.path.join(d, 'ref.txt')).read().split()


def load(name):
    p = os.path.join(d, name)
    return open(p).read().split() if os.path.exists(p) else None


runs = [('C++', load('out_fcond.txt'), False)] + \
       [('C+- L%d' % l, load('out_fcond_l%d.txt' % l), True) for l in (0, 1, 2)]

bad = 0
for label, got, cmm_only in runs:
    if got is None:
        print('%-8s no output' % label); bad += 1; continue
    k = 0          # index into this run's own output
    wrong = []
    for i, name, tag, in_cmm in cases:
        if cmm_only and in_cmm == '0':
            continue
        r = ref[int(i)]
        g = got[k] if k < len(got) else '?'
        if g != r:
            wrong.append((name, tag, r, g))
        k += 1
    n = k
    print('%-8s %3d cases, %3d differ from g++' % (label, n, len(wrong)))
    for name, tag, r, g in wrong:
        print('           %-6s %-6s  C: %s  here: %s' % (name, tag, r, g))
    bad += len(wrong)
sys.exit(1 if bad else 0)
