"""Reference model of Software/sapho_all.cmm: the 4-bit value its out(0, ...) prints
every turn (the 32-bit sum s folded by XOR of its eight nibbles).

Written from the operator definitions in HDL/ula.v, NOT by running YANC:
  - int: NUBITS-bit two's complement, wrapping; / and % truncate toward zero
    (Verilog signed, as C); >> is logical, >>> arithmetic, << logical;
    && || ! give 0/1; sign(a, b) is b with the sign of a (by negation);
    norm(v) = v / NUGAIN, truncated; pset(v) = 0 if v < 0 else v.
  - float: value m * 2^e, m an explicit NBMANT-bit mantissa (top bit set);
    at #FROUND 2 every F_ADD / F_SU / F_MLT / F_DIV / I2F result is the exact
    value rounded to nearest, ties to even, to NBMANT significant bits;
    F2I truncates toward zero (saturating); F_SGN / F_ABS / F_NEG / F_PST
    act on the sign only.
  - sqrt(4.0), exp(0.0) and log(1.0) are taken as exact (2, 1, 0): the program
    calls them only to instantiate F_ROT / F_SCL / XPO, with arguments whose
    result the library macros produce exactly.
  - ia[y) is the FFT bit-reversed index: FFTSIZ bits of y, reversed.
Usage: python3 model.py <input value> [--terms]
"""
import sys
from fractions import Fraction as Fr

NUBITS, NBMANT, NUGAIN, FFTSIZ = 32, 23, 128, 3
M = 1 << NUBITS

def wrap(v):                       # to a signed NUBITS-bit word
    v %= M
    return v - M if v >= M // 2 else v

def tdiv(a, b): q = abs(a) // abs(b); return wrap(q if (a < 0) == (b < 0) else -q)
def tmod(a, b): return wrap(a - tdiv(a, b) * b)
def shr(a, n):  return wrap((a % M) >> n)
def srs(a, n):  return wrap(a >> n)
def shl(a, n):  return wrap(a << n)
def sgn(a, b):  return wrap(b if (a < 0) == (b < 0) else -b)
def nrm(a):     return tdiv(a, NUGAIN)
def pst(a):     return 0 if a < 0 else a
def inv(a):     return wrap(~a)
def lan(a, b):  return int(a != 0 and b != 0)
def lor(a, b):  return int(a != 0 or b != 0)
def lin(a):     return int(a == 0)
def rev(k):     return int(format(k % (1 << FFTSIZ), f'0{FFTSIZ}b')[::-1], 2)

def fl(q):                         # round to nearest even, NBMANT significant bits
    q = Fr(q)
    if q == 0: return Fr(0)
    s = -1 if q < 0 else 1; a = abs(q)
    e = 0
    while a >= 2 ** NBMANT: a /= 2; e += 1
    while a <  2 ** (NBMANT - 1): a *= 2; e -= 1
    m = a.numerator // a.denominator; r = a - m
    if r > Fr(1, 2) or (r == Fr(1, 2) and m % 2): m += 1
    return s * Fr(m) * Fr(2) ** e
def f2i(q):  return wrap(int(q))   # int() truncates toward zero
def fsgn(a, b): return abs(b) if a >= 0 else -abs(b)
def fpst(a): return Fr(0) if a < 0 else a

def run(inp):
    terms = []
    s = 0
    def add(t):
        nonlocal s
        terms.append(t); s = wrap(s + t)
    def addf(fw):                  # t = fw * 4194304.0 (F_MLT, then F2I); s = s + t
        add(f2i(fl(fw * 4194304)))
    x = inp; fx = fl(inp); w = 0
    y = wrap(x - 4); z = wrap(0 - x)
    fy = fl(fx * Fr(1, 2)); fz = fl(fx - 10)

    # int with a memory operand
    for t in (wrap(x + y), wrap(x * y), tdiv(z, y), tmod(z, y), x & y, x | y, x ^ y,
              lan(x, y), lor(x, w), shl(x, y), shr(z, y), srs(z, y), sgn(y, z)):
        add(t)
    if x < y: s = wrap(s + 1)
    if x > y: s = wrap(s + 2)
    if x == y: s = wrap(s + 4)
    # int, both operands computed
    a, b = wrap(x + 1), wrap(y + 2)
    for t in (wrap(a + wrap(y * 2)), wrap(a * b), tdiv(wrap(z - 1), wrap(y + 1)),
              tmod(wrap(z - 1), wrap(y + 1)), a & b, a | b, a ^ b,
              lan(a, wrap(y - 3)), lor(wrap(x - 7), b), shl(a, wrap(y - 1)),
              shr(wrap(z - 1), wrap(y - 1)), srs(wrap(z - 1), wrap(y - 1)),
              sgn(wrap(y + 1), wrap(z - 1))):
        add(t)
    if a < b: s = wrap(s + 8)
    if a > b: s = wrap(s + 16)
    if a == wrap(y + 5): s = wrap(s + 32)
    # int unary
    y1 = wrap(y + 1)
    for t in (wrap(-wrap(x + y)), wrap(-x), wrap(y1 + x), abs(wrap(z - 1)), abs(z),
              wrap(y1 - abs(z)), pst(wrap(z - 1)), pst(x), wrap(y1 - pst(z)),
              nrm(wrap(x * 1000)), nrm(z), wrap(y1 - nrm(x)),
              inv(wrap(x + y)), inv(x), wrap(y1 - inv(x)),
              lin(wrap(x - 7)), lin(x), wrap(y1 - lin(x))):
        add(t)
    # float
    for fw in (fl(fx + fy), fl(fx * fy), fl(fx / fy), fl(fx - fy), fsgn(fy, fz),
               fl(fl(fx + 1) + fl(fy * 2)), fl(fl(fx + 1) * fl(fy + 2)),
               fl(fl(fx + 1) / fl(fy + 2)), fl(fl(fx + 1) - fl(fy + 2)),
               fl(fy - fl(fx * 2)), fsgn(fl(fy + 1), fl(fz - 1))):
        addf(fw)
    if fx < fy: s = wrap(s + 64)
    if fx > fy: s = wrap(s + 128)
    if fl(fx + 1) < fl(fy + 2): s = wrap(s + 256)
    if fl(fx + 1) > fl(fy + 2): s = wrap(s + 512)
    fy1 = fl(fy + 1)
    for fw in (-fl(fx + fy), -fx, fl(fy1 + fx), abs(fl(fz - 1)), abs(fz),
               fl(fy1 - abs(fz)), fpst(fl(fz - 1)), fpst(fx), fl(fy1 - fpst(fz))):
        addf(fw)
    # conversions
    addf(fl(wrap(x + y))); addf(fl(x)); addf(fl(fy1 - fl(x))); addf(fl(fl(x) - fy1))
    add(f2i(fl(fx + fy))); add(f2i(fy)); add(f2i(fl(fl(y1) - fy))); add(wrap(y1 - lin(f2i(fy))))
    # input with a live accumulator (the port gives the same value every read)
    add(wrap(y1 - inp)); addf(fl(fy1 - fl(inp)))
    # arrays
    ia = [0] * 8; fa = [Fr(0)] * 8
    ia[y] = x; add(ia[y])
    ia[rev(y)] = z; add(ia[rev(y)])
    fa[y] = fx; addf(fa[y])
    # call, second parameter, comp index, library blocks
    add(wrap(x + x)); add(wrap(x - y))
    add(ia[f2i(fl(Fr(2) + 1))])             # real part of (2+1i)+(1+0i), F2I
    addf(Fr(2)); addf(Fr(1)); addf(Fr(0))    # sqrt(4.0), exp(0.0), log(1.0)
    u = s % M                                # the fold works on the raw word (>> is logical)
    u ^= u >> 16; u ^= u >> 8; u ^= u >> 4
    return u & 15, terms

if __name__ == '__main__':
    s, terms = run(int(sys.argv[1]))
    if '--terms' in sys.argv:
        for t in terms: print(t)
    print(s)
