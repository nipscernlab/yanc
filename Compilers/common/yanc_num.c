// yanc_num -- exact decimal -> SAPHO float encoder. See yanc_num.h.

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "yanc_num.h"

// ---- a minimal unsigned big integer ------------------------------------------
// little-endian 32-bit limbs; 5120 bits cover |decimal exponent| <= 1200 with
// up to ~100 significant digits, far beyond any SAPHO format in use.

#define BN_LIMBS 160
typedef struct { int n; uint32_t d[BN_LIMBS]; } bn;

static void bn_set(bn *a, uint32_t v) { a->n = v ? 1 : 0; a->d[0] = v; }
static void bn_trim(bn *a) { while (a->n > 0 && a->d[a->n - 1] == 0) a->n--; }

static int bn_mul_add(bn *a, uint32_t mul, uint32_t add)      // a = a*mul + add
{
    uint64_t c = add;
    for (int i = 0; i < a->n; i++) { c += (uint64_t)a->d[i] * mul; a->d[i] = (uint32_t)c; c >>= 32; }
    if (c) { if (a->n == BN_LIMBS) return 0; a->d[a->n++] = (uint32_t)c; }
    return 1;
}

static int bn_bitlen(const bn *a)
{
    if (a->n == 0) return 0;
    uint32_t t = a->d[a->n - 1]; int b = 0;
    while (t) { b++; t >>= 1; }
    return (a->n - 1) * 32 + b;
}

static int bn_shl(bn *r, const bn *a, int s)                   // r = a << s
{
    int w = s / 32, b = s % 32;
    if (a->n == 0) { r->n = 0; return 1; }
    if (a->n + w + 1 > BN_LIMBS) return 0;
    uint32_t tmp[BN_LIMBS + 1] = {0};
    for (int i = 0; i < a->n; i++) {
        uint64_t v = (uint64_t)a->d[i] << b;
        tmp[i + w]     |= (uint32_t)v;
        tmp[i + w + 1] |= (uint32_t)(v >> 32);
    }
    r->n = a->n + w + 1;
    memcpy(r->d, tmp, sizeof(uint32_t) * r->n);
    bn_trim(r);
    return 1;
}

static int bn_cmp(const bn *a, const bn *b)
{
    if (a->n != b->n) return a->n < b->n ? -1 : 1;
    for (int i = a->n - 1; i >= 0; i--)
        if (a->d[i] != b->d[i]) return a->d[i] < b->d[i] ? -1 : 1;
    return 0;
}

static void bn_sub(bn *a, const bn *b)                          // a -= b, a >= b
{
    int64_t c = 0;
    for (int i = 0; i < a->n; i++) {
        int64_t v = (int64_t)a->d[i] - (i < b->n ? b->d[i] : 0) + c;
        c = v < 0 ? -1 : 0;
        a->d[i] = (uint32_t)(v + (c ? ((int64_t)1 << 32) : 0));
    }
    bn_trim(a);
}

// q = floor(A / B) for A < B * 2^64, *half = sign of (2*remainder - B),
// *rem0 = remainder is zero. A is consumed.
static uint64_t bn_divq(bn *A, const bn *B, int *half, int *rem0)
{
    uint64_t q = 0;
    int top = bn_bitlen(A) - bn_bitlen(B);
    bn t;
    for (int b = top; b >= 0; b--) {
        if (b > 63) continue;                                   // cannot happen: caller bounds A
        bn_shl(&t, B, b);
        if (bn_cmp(A, &t) >= 0) { bn_sub(A, &t); q |= (uint64_t)1 << b; }
    }
    *rem0 = (A->n == 0);
    bn_shl(&t, A, 1);                                           // 2 * remainder
    *half = bn_cmp(&t, B);
    return q;
}

// quotient of N * 2^k / P (k may be negative), rounding info
static int quot(const bn *N, const bn *P, int k, uint64_t *q, int *half, int *rem0)
{
    bn A, B;
    if (k >= 0) { if (!bn_shl(&A, N, k)) return 0; B = *P; }
    else        { A = *N; if (!bn_shl(&B, P, -k)) return 0; }
    if (bn_bitlen(&A) - bn_bitlen(&B) > 63) return 0;
    *q = bn_divq(&A, &B, half, rem0);
    return 1;
}

// ---- the encoder ---------------------------------------------------------------

static void put_bits(char *out, int *pos, unsigned long long v, int n)
{
    for (int i = n - 1; i >= 0; i--) out[(*pos)++] = ((v >> i) & 1) ? '1' : '0';
}

static void pack(yn_float *r, int nbmant, int nbexpo, int zero)
{
    int p = 0;
    if (zero) {                                                 // {0, 100..0, 0..0}
        r->s = 0; r->m = 0; r->e = -((long long)1 << (nbexpo - 1));
        r->bits[p++] = '0';
        put_bits(r->bits, &p, (unsigned long long)1 << (nbexpo - 1), nbexpo);
        put_bits(r->bits, &p, 0, nbmant);
    } else {
        unsigned long long emask = (nbexpo >= 64) ? ~0ULL : (((unsigned long long)1 << nbexpo) - 1);
        r->bits[p++] = r->s ? '1' : '0';
        put_bits(r->bits, &p, (unsigned long long)r->e & emask, nbexpo);
        put_bits(r->bits, &p, r->m, nbmant);
    }
    r->bits[p] = 0;
}

static double approx(unsigned long long m, long long e)       // m * 2^e as a double
{
    double v = (double)m;
    if (e > 0) while (e--) v *= 2.0; else while (e++ < 0) v *= 0.5;
    return v;
}

int yn_encode(const char *text, int nbmant, int nbexpo, int fround, yn_float *r)
{
    memset(r, 0, sizeof *r);
    if (nbmant < 2 || nbmant > 63 || nbexpo < 2 || nbexpo > 62 || nbmant + nbexpo + 1 > YN_MAXBITS) return 0;

    // parse: [+-] digits [. digits] [(e|E) [+-] digits] ---------------------------
    const char *p = text;
    while (isspace((unsigned char)*p)) p++;
    int neg = 0;
    if (*p == '+' || *p == '-') neg = (*p++ == '-');
    bn D; bn_set(&D, 0);
    long long X = 0; int ndig = 0, nsig = 0;
    for (; isdigit((unsigned char)*p); p++, ndig++) {
        if (nsig || *p != '0') { if (!bn_mul_add(&D, 10, *p - '0')) return 0; nsig++; }
    }
    if (*p == '.') {
        for (p++; isdigit((unsigned char)*p); p++, ndig++) {
            if (nsig || *p != '0') { if (!bn_mul_add(&D, 10, *p - '0')) return 0; nsig++; }
            X--;
        }
    }
    if (ndig == 0) return 0;
    if (*p == 'e' || *p == 'E') {
        p++;
        int eneg = 0; long long ev = 0; int nd = 0;
        if (*p == '+' || *p == '-') eneg = (*p++ == '-');
        for (; isdigit((unsigned char)*p); p++, nd++) if (ev < 100000) ev = ev * 10 + (*p - '0');
        if (nd == 0) return 0;
        X += eneg ? -ev : ev;
    }
    while (isspace((unsigned char)*p)) p++;
    if (*p) return 0;                                            // trailing junk
    r->ok = 1;
    double exact = strtod(text, NULL);                           // diagnostics only

    long long emin = -((long long)1 << (nbexpo - 1));
    long long emax = ((long long)1 << (nbexpo - 1)) - 1;

    if (D.n == 0) { pack(r, nbmant, nbexpo, 1); return 1; }     // zero (either sign)
    if (X > 1200)  { r->overflow  = 1; pack(r, nbmant, nbexpo, 1); return 1; }
    if (X < -1200) { r->underflow = 1; r->inexact = 1; pack(r, nbmant, nbexpo, 1); r->delta = -exact; return 1; }

    // value = N / P, exact --------------------------------------------------------
    bn N = D, P; bn_set(&P, 1);
    for (long long i = 0; i < (X > 0 ? X : -X); i++)
        if (!bn_mul_add(X > 0 ? &N : &P, 10, 0)) return r->ok = 0;

    // m = N * 2^k / P with NBMANT bits: k from the bit lengths, corrected -------
    uint64_t q; int half, rem0;
    int k = nbmant - (bn_bitlen(&N) - bn_bitlen(&P));
    uint64_t lo = (uint64_t)1 << (nbmant - 1), hi = (uint64_t)1 << nbmant;
    for (int tries = 0; ; tries++) {
        if (!quot(&N, &P, k, &q, &half, &rem0) || tries > 4) return r->ok = 0;
        if (q >= hi) k--; else if (q < lo) k++; else break;
    }
    long long e = -k;

    if (e < emin) {                                              // below the smallest normal
        if (!quot(&N, &P, (int)-emin, &q, &half, &rem0)) return r->ok = 0;
        e = emin;
    }
    r->inexact = !rem0;
    if (half > 0 || (half == 0 && (q & 1))) q++;                // to nearest, ties to even
    if (q == hi) { q >>= 1; e++; }                              // carried out of the mantissa

    if (e > emax) { r->overflow = 1; pack(r, nbmant, nbexpo, 1); return 1; }
    if (q == 0)   { r->underflow = 1; pack(r, nbmant, nbexpo, 1); r->delta = -exact; return 1; }
    if (q < lo) {
        if (fround >= 1) { r->flushed = 1; r->underflow = 1; pack(r, nbmant, nbexpo, 1); r->delta = -exact; return 1; }
        r->denormal = 1;
    }
    r->s = neg; r->m = q; r->e = e;
    pack(r, nbmant, nbexpo, 0);
    r->delta = (neg ? -approx(q, e) : approx(q, e)) - exact;
    return 1;
}
