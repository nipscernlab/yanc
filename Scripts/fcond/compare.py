#!/usr/bin/env python3
"""compare.py: lines up the host g++ answer (ref.txt) with every simulated
run (out_fcond_<format>.txt from cppcomp, out_fcond_<format>_l<n>.txt from
cmmcomp) for the cases of cases.txt, and prints the cases that differ.
Exit 1 if any does. With -s, one line per run (no case list)."""
import glob
import os
import re
import sys

args = [a for a in sys.argv[1:] if a != '-s']
short = '-s' in sys.argv[1:]
d = args[0] if args else '.'
cases = [l.split() for l in open(os.path.join(d, 'cases.txt'))]
ref = open(os.path.join(d, 'ref.txt')).read().split()

runs = []
for p in sorted(glob.glob(os.path.join(d, 'out_fcond_*.txt'))):
    m = re.match(r'out_fcond_(\d+)_(\d+)_(\d+)(?:_l(\d))?\.txt$', os.path.basename(p))
    if not m:
        continue
    fmt = '%s/%s/%s' % m.group(1, 2, 3)
    if m.group(4) is None:
        runs.append(('C++ %s' % fmt, p, False))
    else:
        runs.append(('C+- %s L%s' % (fmt, m.group(4)), p, True))

def min_normal(nm, ne):
    # the smallest normal float: the integer mantissa's top bit set, times
    # 2 to the lowest exponent (the very lowest code is the zero's)
    return 2.0 ** (nm - 1) * 2.0 ** (-(2 ** (ne - 1)) + 1)


bad = 0
for label, p, cmm in runs:
    got = open(p).read().split()
    nm, ne = [int(x) for x in label.split()[1].split('/')[1:3]]
    small = min_normal(nm, ne)
    k, wrong, skipped = 0, [], set()
    for i, name, tag, in_cmm, mag in cases:
        if cmm and in_cmm == '0':
            continue
        g = got[k] if k < len(got) else '?'
        k += 1
        if 0 < float(mag) < small:
            skipped.add(name)                      # not a number of this format
            continue
        r = ref[int(i)]
        if g != r:
            wrong.append((name, tag, r, g))
    note = ' (left out, below %.3g: %s)' % (small, ', '.join(sorted(skipped))) if skipped else ''
    print('%-18s %3d cases, %3d differ from g++%s' % (label, k, len(wrong), note))
    if not short:
        for name, tag, r, g in wrong:
            print('                     %-6s %-6s  C: %s  here: %s' % (name, tag, r, g))
    bad += len(wrong)
sys.exit(1 if bad else 0)
