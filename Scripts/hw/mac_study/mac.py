"""Accuracy of the MAC variants of docs/design/array-unit.md, section 4 (run.sh).

Usage:
  python3 mac.py gen    NUBITS NBMANT NBEXPO DIR    writes DIR/vec.txt and DIR/sets.txt
  python3 mac.py report NUBITS NBMANT NBEXPO DIR    reads them and DIR/base_L{0,1,2}.txt (tb.v)

Each vector is an inner product sum(a[k] * b[k]) of SAPHO floats. Against the
exact sum (Python integers, no rounding) it measures:
  today  F_MLT then F_ADD per term, the Dirac loop of today, run on SAPHO/ula.v
         by tb.v at #FROUND 0, 1, 2 (measured, not modelled)
  F      fused multiply-add per term: the exact product added to the
         accumulator and rounded once per term (model)
  A<W>   lazy accumulator, a W-bit mantissa: aligned to the larger exponent,
         added, one right shift on a carry out, never normalised to the left;
         normalised and rounded once at the end (model)
  B<W>   long fixed-point accumulator of W bits, a window placed from the
         data's range: each product shifted to its place (bits below the
         window truncated), integer add, rounded once at the end (model)
  C      full-range long accumulator (Kulisch): the exact sum rounded once
The models round at the end by #FROUND: 0 and 1 truncate the magnitude, 2
rounds to nearest with ties to even. The error is in units of the last place
of the exact result (ULP); a vector whose exact sum is 0 is reported apart.
Deterministic: the data come from a fixed seed.
"""
import random
import sys
from fractions import Fraction

# ---- the SAPHO float word (docs/floating-point.md) -------------------------
# [sign 1][exponent E, two's complement, no bias][mantissa M, top bit written]
# value = (-1)^s * m * 2^e; zero = the most negative exponent with m = 0

def fmt(M, E):
    return {'M': M, 'E': E, 'emin': -(1 << (E - 1)), 'emax': (1 << (E - 1)) - 1}

def dec(f, w):
    M, E = f['M'], f['E']
    s = (w >> (M + E)) & 1
    e = (w >> M) & ((1 << E) - 1)
    if e >= 1 << (E - 1):
        e -= 1 << E
    m = w & ((1 << M) - 1)
    return s, e, m

def enc(f, s, e, m):
    M, E = f['M'], f['E']
    return (((s << E) | (e & ((1 << E) - 1))) << M) | m

def zero(f):
    return enc(f, 0, f['emin'], 0)

def word_val(f, w):
    """the word as (V, L): value = V * 2^L, V a signed integer"""
    s, e, m = dec(f, w)
    return (-m if s else m), e

def tshift(v, k):
    """v * 2^-k truncated toward zero (sign and magnitude hardware)"""
    if k <= 0:
        return v << -k
    return -((-v) >> k) if v < 0 else v >> k

def round_to(f, V, L, nearest):
    """round V * 2^L to the format; returns (V, L) of the result and a range flag"""
    if V == 0:
        return 0, 0, False
    M = f['M']
    neg = V < 0
    a = -V if neg else V
    sh = a.bit_length() - M
    if sh > 0:
        q = a >> sh
        r = a - (q << sh)
        if nearest:
            half = 1 << (sh - 1)
            if r > half or (r == half and q & 1):
                q += 1
                if q == 1 << M:
                    q >>= 1
                    sh += 1
    else:
        q = a << -sh
    e = L + sh
    bad = e < f['emin'] or e > f['emax']
    return (-q if neg else q), e, bad

def err_ulp(f, got, exact):
    """(got - exact) in units of the last place of exact; both (V, L)"""
    Vg, Lg = got
    Vx, Lx = exact
    if Vx == 0:
        return None
    L = min(Lg, Lx)
    d = (Vg << (Lg - L)) - (Vx << (Lx - L))
    ulp_e = Lx + abs(Vx).bit_length() - f['M']
    return float(Fraction(d) * Fraction(2) ** (L - ulp_e))

# ---- the data ---------------------------------------------------------------

def rnd_word(f, rng, lo, hi, signed):
    """a normalized float with magnitude in [2^lo, 2^hi)"""
    M = f['M']
    m = (1 << (M - 1)) | rng.getrandbits(M - 1)
    e = rng.randrange(lo, hi) - (M - 1)
    s = rng.getrandbits(1) if signed else 0
    return enc(f, s, e, m)

def neg_word(f, w):
    return w ^ (1 << (f['M'] + f['E']))

def make_sets(NB, f):
    """(name, size, a-words, b-words) for every vector"""
    rng = random.Random(20261024)
    if NB == 32:
        sizes, wide, small = [4, 16, 64, 256, 1024], (-20, 20), -14
    else:
        sizes, wide, small = [4, 16, 64, 256], (-3, 6), -5
    per = 30
    out = []
    for n in sizes:
        for _ in range(per):
            a = [rnd_word(f, rng, -2, 2, False) for _ in range(n)]
            b = [rnd_word(f, rng, -2, 2, False) for _ in range(n)]
            out.append(('pos', n, a, b))
        for _ in range(per):
            a = [rnd_word(f, rng, -2, 2, True) for _ in range(n)]
            b = [rnd_word(f, rng, -2, 2, True) for _ in range(n)]
            out.append(('mixed', n, a, b))
        for _ in range(per):
            a = [rnd_word(f, rng, wide[0], wide[1], True) for _ in range(n)]
            b = [rnd_word(f, rng, wide[0], wide[1], True) for _ in range(n)]
            out.append(('wide', n, a, b))
        for _ in range(per):
            # pairs that cancel exactly, and two small terms that survive
            h = n // 2 - 1
            x = [rnd_word(f, rng, -2, 2, True) for _ in range(h)]
            y = [rnd_word(f, rng, -2, 2, True) for _ in range(h)]
            pairs = list(zip(x, y)) + [(neg_word(f, p), q) for p, q in zip(x, y)]
            pairs += [(rnd_word(f, rng, small, small + 2, True), rnd_word(f, rng, -1, 1, False)) for _ in range(2)]
            rng.shuffle(pairs)
            out.append(('cancel', n, [p for p, _ in pairs], [q for _, q in pairs]))
        for _ in range(per):
            a = [rnd_word(f, rng, -2, 2, True) if rng.random() > 0.3 else zero(f) for _ in range(n)]
            b = [rnd_word(f, rng, -2, 2, True) for _ in range(n)]
            out.append(('sparse', n, a, b))
    return out

# ---- the variants -----------------------------------------------------------

def products(f, a, b):
    """exact products as (P, lp): P * 2^lp; a zero mantissa gives P = 0"""
    out = []
    for x, y in zip(a, b):
        V1, e1 = word_val(f, x)
        V2, e2 = word_val(f, y)
        out.append((V1 * V2, e1 + e2))
    return out

def exact_sum(ps):
    nz = [lp for P, lp in ps if P != 0]
    if not nz:
        return 0, 0
    L0 = min(nz)
    return sum(P << (lp - L0) for P, lp in ps if P != 0), L0

def var_fused(f, ps, nearest):
    V, L, bad = 0, 0, False
    for P, lp in ps:
        if P == 0:
            continue
        if V == 0:
            S, Ls = P, lp
        else:
            Ls = min(L, lp)
            S = (V << (L - Ls)) + (P << (lp - Ls))
        V, L, b = round_to(f, S, Ls, nearest)
        bad |= b
    return (V, L), bad

def var_lazy(f, ps, W, nearest):
    """as mac_a in mac.v: two's complement in [-2^W, 2^W), arithmetic shifts
    (toward minus infinity, Python's >>), a zero product skipped"""
    M = f['M']
    V, e = 0, 0
    for P, lp in ps:
        if P == 0:
            continue
        Pp, ep = P << (W - 2 * M), lp - (W - 2 * M)  # the product at the top of W bits
        if V == 0:
            V, e = Pp, ep
            continue
        if ep >= e:
            V, e = V >> (ep - e), ep
        else:
            Pp = Pp >> (e - ep)
        V += Pp
        if not -(1 << W) <= V < (1 << W):             # out of range: one shift right
            V, e = V >> 1, e + 1
    r = round_to(f, V, e, nearest)
    return (r[0], r[1]), r[2]

def var_window(f, ps, W, top, nearest):
    """|acc| < 2^top; its LSB weighs 2^(top - W + 1) (one bit of sign)"""
    L = top - W + 1
    acc, low, over = 0, False, False
    for P, lp in ps:
        if P == 0:
            continue
        if lp < L:
            c = tshift(P, L - lp)
            low |= (c << (L - lp)) != P
        else:
            c = P << (lp - L)
        acc += c
        if abs(acc) >= 1 << (W - 1):
            over = True
    r = round_to(f, acc, L, nearest)
    return (r[0], r[1]), r[2] or over, low

def top_of(ps):
    """the weight bound of the largest partial sum and product of a vector"""
    t = -10 ** 9
    run = 0
    L0 = min([lp for P, lp in ps if P != 0], default=0)
    for P, lp in ps:
        if P == 0:
            continue
        run += P << (lp - L0)
        for v in (abs(run), abs(P) << (lp - L0)):
            if v:
                t = max(t, v.bit_length() + L0)
    return t

# ---- main ---------------------------------------------------------------------

def stats(es):
    if not es:
        return '-'
    m = sum(abs(x) for x in es) / len(es)
    mx = max(abs(x) for x in es)
    return '%.3g/%.3g' % (m, mx)

def main():
    cmd, NB, M, E, d = sys.argv[1], int(sys.argv[2]), int(sys.argv[3]), int(sys.argv[4]), sys.argv[5]
    f = fmt(M, E)
    sets = make_sets(NB, f)
    if cmd == 'gen':
        with open(d + '/vec.txt', 'w') as o:
            o.write('%d\n' % len(sets))
            for name, n, a, b in sets:
                o.write('%d\n' % n)
                for x, y in zip(a, b):
                    o.write('%0*x %0*x\n' % (NB // 4, x, NB // 4, y))
        return
    if cmd == 'expect':
        # expect NB M E DIR VARIANT FR: what mac.v must give, one line per
        # vector: the window's LSB exponent (B; 0 otherwise), the word, 1 if
        # the model's result is in range and the window did not overflow
        var, lv = sys.argv[6], int(sys.argv[7])
        nearest = lv == 2
        pss = [products(f, a, b) for _, _, a, b in sets]
        top = {}
        for (name, n, _, _), ps in zip(sets, pss):
            top[name] = max(top.get(name, -10 ** 9), top_of(ps) + 1)
        with open('%s/exp_%s_L%d.txt' % (d, var, lv), 'w') as o:
            for (name, n, _, _), ps in zip(sets, pss):
                L = 0
                if var[0] == 'A':
                    (V, Le), bad = var_lazy(f, ps, int(var[1:]), nearest)
                elif var[0] == 'B':
                    W = int(var[1:])
                    L = top[name] - W + 1
                    (V, Le), bad, _ = var_window(f, ps, W, top[name], nearest)
                else:
                    S, L0 = exact_sum(ps)
                    V, Le, bad = round_to(f, S, L0, nearest)
                w = zero(f) if V == 0 else enc(f, int(V < 0), Le, abs(V))
                o.write('%d %0*x %d\n' % (L, NB // 4, w, 0 if bad else 1))
        return
    base = {}
    for lv in (0, 1, 2):
        base[lv] = [int(l, 16) for l in open('%s/base_L%d.txt' % (d, lv)) if l.strip()]
        assert len(base[lv]) == len(sets), 'tb.v wrote %d results for %d vectors' % (len(base[lv]), len(sets))
    WA = [2 * M + 2, 2 * M + 8, 2 * M + 16]
    WB = [NB * 3 // 2, NB * 2, NB * 3, NB * 4]
    nmax = max(n for _, n, _, _ in sets)
    full = (1 << (E + 1)) + 2 * M + (nmax - 1).bit_length() - 1
    print('format %d/%d/%d; full-range accumulator (C): about %d bits for up to %d terms' % (NB, M, E, full, nmax))
    # the window: one per data set, placed from the largest top over all its vectors
    pss = [products(f, a, b) for _, _, a, b in sets]
    top = {}
    for (name, n, _, _), ps in zip(sets, pss):
        top[name] = max(top.get(name, -10 ** 9), top_of(ps) + 1)
    for lv in (0, 1, 2):
        nearest = lv == 2
        print('\n== FROUND %d  (error in ULP of the exact result: mean |e| / max |e|)' % lv)
        cols = ['today', 'F'] + ['A%d' % w for w in WA] + ['B%d' % w for w in WB] + ['C']
        print('%-7s %5s  ' % ('set', 'n') + '  '.join('%-15s' % c for c in cols))
        groups = {}
        flags = {}
        zeros = {}
        for i, ((name, n, a, b), ps) in enumerate(zip(sets, pss)):
            ex = exact_sum(ps)
            key = (name, n)
            g = groups.setdefault(key, {c: [] for c in cols})
            fl = flags.setdefault(key, {c: [0, 0] for c in cols})
            if ex[0] == 0:
                zeros[key] = zeros.get(key, 0) + 1
                continue
            res = {'today': (word_val(f, base[lv][i]), False, False)}
            r, bad = var_fused(f, ps, nearest); res['F'] = (r, bad, False)
            for w in WA:
                r, bad = var_lazy(f, ps, w, nearest); res['A%d' % w] = (r, bad, False)
            for w in WB:
                r, bad, low = var_window(f, ps, w, top[name], nearest); res['B%d' % w] = (r, bad, low)
            r = round_to(f, ex[0], ex[1], nearest); res['C'] = ((r[0], r[1]), r[2], False)
            for c in cols:
                got, bad, low = res[c]
                fl[c][0] += bad
                fl[c][1] += low
                if not bad:
                    g[c].append(err_ulp(f, got, ex))
        for key in sorted(groups, key=lambda k: (['pos', 'mixed', 'wide', 'cancel', 'sparse'].index(k[0]), k[1])):
            g = groups[key]
            print('%-7s %5d  ' % key + '  '.join('%-15s' % stats(g[c]) for c in cols))
        print('\nflags, all sizes together (vectors out of range or overflowing / with a product cut below the window):')
        for name in ['pos', 'mixed', 'wide', 'cancel', 'sparse']:
            tot = {c: [0, 0] for c in cols}
            for (nm, n), fl in flags.items():
                if nm == name:
                    for c in cols:
                        tot[c][0] += fl[c][0]
                        tot[c][1] += fl[c][1]
            shown = ['%s %d/%d' % (c, tot[c][0], tot[c][1]) for c in cols if tot[c] != [0, 0]]
            print('  %-7s window top 2^%d; %s' % (name, top[name], ', '.join(shown) if shown else 'none'))
        if zeros:
            print('  vectors with an exact sum of 0, left out:', dict(sorted(zeros.items())))

if __name__ == '__main__':
    main()
