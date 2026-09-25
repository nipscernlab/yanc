"""Check Compilers/common/yanc_num.c against an exact model (TODO.md item 5).

Usage: python3 Scripts/check_yanc_num.py <yanc_num_test executable> [n_random]

The model reads the decimal text as a Fraction and rounds it to the SAPHO
format -- value = m * 2^e, m an explicit NBMANT-bit mantissa with its top bit
set, e a two's-complement NBEXPO-bit exponent, word {s, e, m}, zero =
{0, 100..0, 0..0} -- to nearest, ties to even, with the exponent held at its
minimum below the smallest normal (flushed to zero at #FROUND >= 1). Every
case must give the same word and the same flags.
"""
import random, subprocess, sys
from fractions import Fraction as Fr

FORMATS = [(23, 8), (25, 6), (10, 5), (4, 3), (2, 2)]    # (nbmant, nbexpo): 32/23/8, 32/25/6, 16/10/5, ...

def model(text, nbmant, nbexpo, fround):
    v = Fr(text)                           # Fraction reads 1.5e-3, .5, -2 exactly
    emin, emax = -(1 << (nbexpo - 1)), (1 << (nbexpo - 1)) - 1
    zero = '0' + '1' + '0' * (nbexpo - 1) + '0' * nbmant
    if v == 0:
        return zero, '-'
    s = 1 if v < 0 else 0
    a = abs(v)
    e = a.numerator.bit_length() - a.denominator.bit_length() - nbmant
    while a / Fr(2) ** e >= (1 << nbmant): e += 1
    while a / Fr(2) ** e < (1 << (nbmant - 1)): e -= 1
    if e < emin: e = emin
    x = a / Fr(2) ** e
    q = x.numerator // x.denominator
    r = x - q
    inexact = r != 0
    if r > Fr(1, 2) or (r == Fr(1, 2) and q % 2): q += 1
    if q == (1 << nbmant): q >>= 1; e += 1
    flags = ''
    if e > emax: return zero, 'o' + ('i' if inexact else '')
    if q == 0: return zero, 'u' + ('i' if inexact else '')
    if q < (1 << (nbmant - 1)):
        if fround >= 1: return zero, 'uf' + ('i' if inexact else '')
        flags += 'd'
    if inexact: flags += 'i'
    bits = str(s) + format(e & ((1 << nbexpo) - 1), '0%db' % nbexpo) + format(q, '0%db' % nbmant)
    return bits, flags or '-'

def cases(nrand):
    fixed = ['0', '0.0', '-0.0', '1', '1.0', '-1.0', '0.5', '2', '3', '0.1', '0.001', '1e-8', '2e-8',
             '1.570796327', '4194304.0', '8388608', '16777217', '0.000000000001', '1e-30', '1e30',
             '1E+2', '.5', '5.', '123456789012345678901234567890', '1e-45', '3.4e38', '1e300', '1e-300',
             '-2.5e-3', '0.3', '0.7', '2.718281828459045', '65504', '65505', '7.25']
    out = []
    for nm, ne in FORMATS:
        for fr in (0, 2):
            for t in fixed: out.append((nm, ne, fr, t))
            emin, emax = -(1 << (ne - 1)), (1 << (ne - 1)) - 1
            # exact ties: odd mantissa of nbmant+1 bits -> halfway between two words
            for k in range(20):
                m = random.randrange(1 << nm, 1 << (nm + 1)) | 1
                e = random.randint(emin, emax)
                t = Fr(m) * Fr(2) ** (e - 1)
                out.append((nm, ne, fr, dec(t)))
            # values around the smallest normal / denormals / overflow edge
            for k in range(20):
                e = random.choice([emin - 3, emin - 1, emin, emin + 1, emax, emax + 1])
                m = random.randrange(1, 1 << (nm + 2))
                out.append((nm, ne, fr, dec(Fr(m) * Fr(2) ** (e - 1))))
    for k in range(nrand):
        nm, ne = random.choice(FORMATS)
        mant = str(random.randrange(1, 10 ** random.randint(1, 25)))
        dot = random.randint(0, len(mant))
        t = mant[:dot] + '.' + mant[dot:] + (('e%d' % random.randint(-40, 40)) if random.random() < 0.5 else '')
        if random.random() < 0.3: t = '-' + t
        out.append((nm, ne, random.choice((0, 1, 2)), t))
    return out

def dec(f):                               # a Fraction with a power-of-two denominator, as exact decimal text
    n, d = f.numerator, f.denominator
    k = 0
    while d > 1: d //= 2; n *= 5; k += 1
    s = str(n)
    if k == 0: return s
    s = s.rjust(k + 1, '0')
    return s[:-k] + '.' + s[-k:]

def main():
    exe = sys.argv[1]
    nrand = int(sys.argv[2]) if len(sys.argv) > 2 else 3000
    random.seed(20260925)
    cs = [c for c in cases(nrand) if len(c[3]) < 390]
    inp = ''.join('%d %d %d %s\n' % c for c in cs)
    out = subprocess.run([exe], input=inp, capture_output=True, text=True).stdout.split('\n')
    bad = 0
    for c, got in zip(cs, out):
        want = ' '.join(model(c[3], c[0], c[1], c[2]))
        if got.strip() != want:
            bad += 1
            if bad <= 10: print('MISMATCH %d/%d fround %d %s: got %s want %s' % (c[0] + c[1] + 1, c[0], c[2], c[3], got.strip(), want))
    print('check_yanc_num: %d cases, %d mismatches' % (len(cs), bad))
    sys.exit(1 if bad else 0)

main()
