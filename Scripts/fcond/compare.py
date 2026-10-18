#!/usr/bin/env python3
"""compare.py: lines up the host answers with every simulated run and prints
the cases that differ. Exit 1 if any does. With -s, one line per run.
  float:   ref.txt (host g++) against out_fcond_<format>.txt (cppcomp) and
           out_fcond_<format>_l<n>.txt (cmmcomp), cases in cases.txt
  complex: ref_comp.txt (host gcc, C99) against out_fcomp_<format>_l<n>.txt
           (cmmcomp), cases in ccases.txt"""
import glob
import os
import re
import sys

args = [a for a in sys.argv[1:] if a != '-s']
short = '-s' in sys.argv[1:]
d = args[0] if args else '.'


def min_normal(nm, ne):
    # the smallest normal float: the integer mantissa's top bit set, times
    # 2 to the lowest exponent (the very lowest code is the zero's)
    return 2.0 ** (nm - 1) * 2.0 ** (-(2 ** (ne - 1)) + 1)


FAMILIES = [('fcond', 'cases.txt', 'ref.txt'), ('fcomp', 'ccases.txt', 'ref_comp.txt')]
bad = 0
for fam, cfile, rfile in FAMILIES:
    if not os.path.exists(os.path.join(d, cfile)) or not os.path.exists(os.path.join(d, rfile)):
        continue
    cases = [l.split() for l in open(os.path.join(d, cfile))]
    ref = open(os.path.join(d, rfile)).read().split()
    for p in sorted(glob.glob(os.path.join(d, 'out_%s_*.txt' % fam))):
        m = re.match(r'out_%s_(\d+)_(\d+)_(\d+)(?:_l(\d))?\.txt$' % fam, os.path.basename(p))
        if not m:
            continue
        nm, ne = int(m.group(2)), int(m.group(3))
        cmm = m.group(4) is not None
        lang = ('C+- ' if cmm else 'C++ ') + ('cplx ' if fam == 'fcomp' else '')
        label = '%s%s/%s/%s%s' % (lang, m.group(1), nm, ne, ' L' + m.group(4) if cmm else '')
        got = open(p).read().split()
        small = min_normal(nm, ne)
        k, wrong, skipped = 0, [], set()
        for i, name, tag, in_cmm, mag in cases:
            if cmm and in_cmm == '0':
                continue
            g = got[k] if k < len(got) else '?'
            k += 1
            if 0 < float(mag) < small:
                skipped.add(name)                  # not a number of this format
                continue
            if g != ref[int(i)]:
                wrong.append((name, tag, ref[int(i)], g))
        note = ' (left out, below %.3g: %s)' % (small, ', '.join(sorted(skipped))) if skipped else ''
        print('%-24s %3d cases, %3d differ from the host%s' % (label, k, len(wrong), note))
        if not short:
            for name, tag, r, g in wrong:
                print('                         %-6s %-6s  C: %s  here: %s' % (name, tag, r, g))
        bad += len(wrong)
sys.exit(1 if bad else 0)
