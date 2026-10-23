"""Accuracy of F_ADD against the exact results (Scripts/hw/fadd_acc/run.sh).

Usage: python3 check.py <dir with pairs.txt and sums.txt written by tb.v>
Also counts the pairs where a - b should be exact (b between a/2 and 2a, the
documented Sterbenz case) and is not.
"""
import sys
from fractions import Fraction as F
def val(w):
    w = int(w, 16); s = w >> 31; e = (w >> 23) & 0xFF; m = w & 0x7FFFFF
    if e >= 128: e -= 256
    v = F(m) * F(2) ** e
    return -v if s else v
def ulp(x):               # one unit in the last place of a normalized result near x
    a = abs(x)
    if a == 0: return F(0)
    e = a.numerator.bit_length() - a.denominator.bit_length()
    while F(2) ** e > a: e -= 1
    while F(2) ** (e + 1) <= a: e += 1
    return F(2) ** (e - 22)
def trunc(x):
    if x == 0: return F(0)
    u = ulp(x); q = abs(x) // u * u
    return q if x > 0 else -q
d = sys.argv[1]
# pairs
n = bad = 0; hist = {}
nsub = badsub = 0
for line in open(d + '/pairs.txt'):
    a, b, o = line.split(); x = val(a) + val(b); got = val(o)
    sub = (val(a) > 0) != (val(b) > 0)
    n += 1; nsub += sub
    if got != trunc(x):
        bad += 1; badsub += sub
        k = round(abs(got - trunc(x)) / ulp(x)) if x != 0 else 99
        hist[k] = hist.get(k, 0) + 1
print('pairs: %d, not the exact truncation: %d (%.1f %%); of them subtractions: %d of %d' % (n, bad, 100.0 * bad / n, badsub, nsub))
print('  distance from the truncation, in units of the last place:', dict(sorted(hist.items())))
ns = nbad = 0
for line in open(d + '/pairs.txt'):
    a, b, o = line.split(); x, y = val(a), val(b)
    if (x > 0) == (y > 0) or not (abs(x) / 2 <= abs(y) <= 2 * abs(x)): continue
    ns += 1; nbad += val(o) != x + y
print('  a - b with b between a/2 and 2a: %d pairs, %d not exact' % (ns, nbad))
# sums
errs = []
cur = []
for line in open(d + '/sums.txt'):
    if line.startswith('='):
        exact = sum(cur, F(0)); got = val(line.split()[1])
        errs.append(float((got - exact) / ulp(exact)))
        cur = []
    else:
        cur.append(val(line.strip()))
import statistics as st
print('sums of 1000 (300 trials): error of the final result in units of its last place: mean %+.1f, mean |e| %.1f, max |e| %.1f'
      % (st.mean(errs), st.mean(abs(e) for e in errs), max(abs(e) for e in errs)))
