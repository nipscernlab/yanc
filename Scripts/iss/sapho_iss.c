/* sapho_iss: an instruction-set simulator of SAPHO (GitHub issue #32).

   It runs the program asmcomp built, from the two memory images
   (<proc>_inst.mif, <proc>_data.mif) and the parameters of the generated
   top (<proc>.v), and prints what a testbench can see of the processor,
   cycle by cycle:

       <cycle> F <addr>          the instruction address fetched
       <cycle> W <addr> <word>   a data-memory write
       <cycle> I <port>          an input read
       <cycle> O <port> <word>   an output
       <cycle> D <words>         the data stack's depth, when it changes
       <cycle> R <words>         the instruction stack's depth, when it changes

   Scripts/iss/tb_iss.v prints the same lines from the RTL, and
   Scripts/iss/iss_diff.sh compares the two. Cycle 0 is the first cycle out
   of reset; words are hexadecimal, NUBITS wide.

   What each instruction does is written here from Compilers/common/isa.tsv
   (its acc, stack, opnd, flow and io columns), not from instr_dec.v or
   core.v, so a decoder or a core that does something else shows up as a
   difference. What the ALU computes is the meaning Scripts/hw/tb_ula.v checks
   the ALU against (its ref_* functions), ported to C. The timing is the
   processor's own, as core.v documents it, because the trace is compared
   cycle by cycle: one instruction a cycle; a jump decided in decode, with no
   slot after it; the word at address 0 fetched on the reset edges and
   executed in cycle 0, the PC register then 1; OUT seen a cycle after its
   decode (io_ctrl registers it); the write of INC a cycle after its decode
   (#203); QUO, REM and F_QUO reading the division started STG-1 cycles
   before (#113).

   Not modelled: TOM and CAD (they need a partner processor), a stack that
   overflows or underflows, a data address past the memory, a float at
   #FROUND 1 or 2 that is not normalized or the canonical zero, an F_DIV at
   #FROUND 0 with a divisor that is not normalized (tb_ula.v leaves these
   undefined). The ISS stops with an error on the first three; the others
   are the generator's to avoid (Scripts/iss/rand_asm.py).

   Usage: sapho_iss <proc.v> <inst.mif> <data.mif> [-n cycles] [-i c1,c2,...] [-h]
     -n  cycles to run (default: until the halt loop, a JMP to itself, has
         run 4 cycles; the count is printed on stderr as "cycles N")
     -i  cycles in which the interrupt pin is high (#ITRAD programs)
     -h  print on stderr the cycles where an interrupt shows the most: "inc C"
         where INC or P_INC is in decode (the pulse drops its write), "stk C"
         where the data stack holds words (the pulse empties it)

   A depth is the stack's before the cycle's instruction: the words left by
   the cycles before it, as the stack's counter holds them.

   The input ports read in(k, port) for the k-th read of the run, the same
   function tb_iss.v uses (iss_in below). */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;
typedef unsigned __int128 u128;

/* ------------------------------------------------------------ parameters --- */

static int NU = 32, MAN = 23, EXP = 8, NBOPER = 9, MDATAS = 64, MINSTS = 64;
static int SDEPTH = 8, DDEPTH = 8, FFTSIZ = 3, FROUND = 0, ITRADD = 0;
static i64 NUGAIN = 128;
static int HAS_DIV = 0;        /* DIV, S_DIV, MOD or S_MOD: F_DIV shares their array */
static int MDATAW, MINSTW;
static u32 MASK;

static int clog2(int n) { int b = 0; while ((1 << b) < n) b++; return b; }

static void die(const char *m, long c)
{
    fprintf(stderr, "sapho_iss: %s (cycle %ld)\n", m, c);
    exit(2);
}

/* ".NAME(value)" in the generated top; returns 1 when found */
static int vparam(const char *text, const char *name, long *v)
{
    char key[64];
    const char *p;
    snprintf(key, sizeof key, ".%s(", name);
    p = strstr(text, key);
    if (!p) return 0;
    *v = strtol(p + strlen(key), NULL, 10);
    return 1;
}

static char *slurp(const char *path)
{
    FILE *f = fopen(path, "rb");
    long n;
    char *b;
    if (!f) { fprintf(stderr, "sapho_iss: cannot open %s\n", path); exit(2); }
    fseek(f, 0, SEEK_END); n = ftell(f); fseek(f, 0, SEEK_SET);
    b = malloc(n + 1);
    if (fread(b, 1, n, f) != (size_t)n) { fprintf(stderr, "sapho_iss: cannot read %s\n", path); exit(2); }
    b[n] = 0; fclose(f);
    return b;
}

static void read_params(const char *vfile)
{
    char *t = slurp(vfile);
    long v;
    if (vparam(t, "NUBITS", &v)) NU = v;
    if (vparam(t, "NBMANT", &v)) MAN = v;
    if (vparam(t, "NBEXPO", &v)) EXP = v;
    if (vparam(t, "NBOPER", &v)) NBOPER = v;
    if (vparam(t, "NUGAIN", &v)) NUGAIN = v;
    if (vparam(t, "MDATAS", &v)) MDATAS = v;
    if (vparam(t, "MINSTS", &v)) MINSTS = v;
    if (vparam(t, "SDEPTH", &v)) SDEPTH = v;
    if (vparam(t, "DDEPTH", &v)) DDEPTH = v;
    if (vparam(t, "FFTSIZ", &v)) FFTSIZ = v;
    if (vparam(t, "FROUND", &v)) FROUND = v;
    if (vparam(t, "ITRADD", &v)) ITRADD = v;
    HAS_DIV = (vparam(t, "DIV", &v) && v) || (vparam(t, "S_DIV", &v) && v) ||
              (vparam(t, "MOD", &v) && v) || (vparam(t, "S_MOD", &v) && v);
    free(t);
    MDATAW = clog2(MDATAS);
    MINSTW = clog2(MINSTS);
    MASK = (NU >= 32) ? 0xffffffffu : ((1u << NU) - 1);
}

/* a .mif: one binary word a line */
static u32 *read_mif(const char *path, int n)
{
    char *t = slurp(path), *p = t;
    u32 *m = calloc(n, sizeof *m);
    int i = 0;
    while (*p && i < n) {
        u32 w = 0;
        int any = 0;
        while (*p == '0' || *p == '1') { w = (w << 1) | (u32)(*p - '0'); p++; any = 1; }
        if (any) m[i++] = w;
        while (*p && *p != '0' && *p != '1') p++;
    }
    free(t);
    return m;
}

/* --------------------------------------------------------------- words --- */

static i64 sx(u32 w) { return (NU >= 64) ? (i64)w : (i64)((u64)w << (64 - NU)) >> (64 - NU); }
static u32 wd(i64 v) { return (u32)v & MASK; }

/* the input ports: the k-th read of the run, at port p; small numbers a
   quarter of the time, any word otherwise (tb_iss.v computes the same) */
static u32 iss_in(u32 k, u32 p)
{
    u32 h = k * 2654435761u ^ (p * 40503u);
    h ^= h >> 15; h *= 2246822519u; h ^= h >> 13;
    if ((h & 3) == 0) return wd((i64)((h >> 8) % 33) - 16);
    return wd(h);
}

/* ---------------------------------------------------------------- float --- */
/* value = mantissa * 2^(exponent - MAN), no hidden bit (docs/floating-point.md);
   everything below is Scripts/hw/tb_ula.v's reference, in C */

static int EMIN, EMAX;
static u32 ZERO, MMASK, EMASK;

static int f_sgn(u32 w) { return (w >> (MAN + EXP)) & 1; }
static int f_exp(u32 w) { int e = (w >> MAN) & EMASK; return (e & (1 << (EXP - 1))) ? e - (1 << EXP) : e; }
static u32 f_man(u32 w) { return w & MMASK; }
static u32 f_word(int s, int e, u32 m) { return ((u32)s << (MAN + EXP)) | (((u32)e & EMASK) << MAN) | (m & MMASK); }

static u32 fword(int s, int e, u32 m) { return (FROUND != 0 && (m & MMASK) == 0) ? f_word(0, e, m) : f_word(s, e, m); }

static int msb128(u128 x) { int p = -1; while (x) { p++; x >>= 1; } return p; }

/* M * 2^(E - MAN), encoded; mode 2 rounds to the nearest, ties to even, 1
   truncates, both saturating and flushing to the canonical zero; mode 0
   truncates and wraps the exponent */
static u32 pack(int mode, int s, u128 M, i64 E)
{
    u128 q, rem, half, one = 1;
    i64 ee;
    int p, k;
    if (M == 0) return ZERO;
    p = msb128(M);
    k = p - (MAN - 1);
    if (k > 0) {
        q = M >> k;
        rem = M & ((one << k) - 1);
        half = one << (k - 1);
        if (mode == 2 && (rem > half || (rem == half && (q & 1)))) q = q + 1;
        ee = E + k;
        if ((q >> MAN) & 1) { q >>= 1; ee++; }
    } else {
        q = M << (-k);
        ee = E + k;
    }
    if (mode == 0)    return f_word(s, (int)ee, (u32)q);
    if (ee > EMAX)    return f_word(s, EMAX, MMASK);
    if (ee < EMIN)    return ZERO;
    return f_word(s, (int)ee, (u32)q);
}

static u32 neg_f(u32 x) { return x ^ (1u << (MAN + EXP)); }

/* x + y; level 2 the exact sum, rounded (computed on the normalized operands
   with guard bits and a sticky bit, which rounds as the exact sum does);
   levels 0 and 1 as ula_denorm aligns them */
static u32 ref_fadd(u32 x, u32 y)
{
    int ex = f_exp(x), ey = f_exp(y), sx_ = f_sgn(x), sy = f_sgn(y), sr;
    if (FROUND == 2) {
        u64 mx = f_man(x), my = f_man(y);
        i64 nx = ex, ny = ey, d;
        u128 A, B, S;
        const int G = 2 * MAN + 6;
        if (mx == 0 && my == 0) return ZERO;
        if (mx == 0) return pack(2, sy, my, ey);
        if (my == 0) return pack(2, sx_, mx, ex);
        while (!((mx >> (MAN - 1)) & 1)) { mx <<= 1; nx--; }
        while (!((my >> (MAN - 1)) & 1)) { my <<= 1; ny--; }
        if (nx < ny) { u64 t = mx; mx = my; my = t; i64 te = nx; nx = ny; ny = te; int ts = sx_; sx_ = sy; sy = ts; }
        d = nx - ny;
        A = (u128)mx << G;
        if (d <= G) B = ((u128)my << G) >> d;
        else        B = 1;                                  /* below every bit kept: a sticky */
        if (sx_ == sy)   { S = A + B; sr = sx_; }
        else if (A >= B) { S = A - B; sr = sx_; }
        else             { S = B - A; sr = sy; }
        return pack(2, sr, S, nx - G);
    } else {
        int eb, sh, sb, ss;
        u32 mb, ms, al, T;
        if (ex < ey) { eb = ey; sh = ey - ex; sb = sy; ss = sx_; mb = f_man(y); ms = f_man(x); }
        else         { eb = ex; sh = ex - ey; sb = sx_; ss = sy; mb = f_man(x); ms = f_man(y); }
        al = (sh >= MAN) ? 0 : (ms >> sh);
        if (sb == ss)      { T = mb + al; sr = sb; }
        else if (mb >= al) { T = mb - al; sr = sb; }
        else               { T = al - mb; sr = ss; }
        if (FROUND == 1) return pack(1, sr, T, eb);
        if (T == 1) return f_word(sr, EMIN, 0);             /* the legacy -0.0 */
        return pack(0, sr, T >> 1, (i64)eb + 1);
    }
}

static u32 ref_fmlt(u32 x, u32 y)
{
    u128 P = (u128)f_man(x) * f_man(y);
    int s = f_sgn(x) ^ f_sgn(y);
    if (FROUND == 0) return pack(0, s, P >> MAN, (i64)f_exp(x) + f_exp(y) + MAN);
    return pack(FROUND, s, P, (i64)f_exp(x) + f_exp(y));
}

static u32 ref_i2f(u32 x)
{
    if (FROUND == 0) {
        i64 lo = (i64)((u64)x << (64 - MAN)) >> (64 - MAN);
        u64 lm = (lo < 0) ? (u64)(-lo) : (u64)lo;
        return pack(0, lo < 0, lm & MMASK, 0);
    } else {
        i64 v = sx(x);
        u64 mg = (v < 0) ? (u64)(-v) : (u64)v;
        return pack(FROUND, v < 0, mg, 0);
    }
}

static u32 ref_frot(u32 x)
{
    int h = (f_exp(x) + (MAN - 1)) / 2;
    return pack(FROUND, f_sgn(x), 1, h);
}

static u32 ref_fdiv(u32 x, u32 y)
{
    int sq = f_sgn(x) ^ f_sgn(y);
    u128 A, Q, R;
    if (FROUND != 0 && f_man(y) == 0) return f_word(sq, EMAX, MMASK);
    if (FROUND == 0) {
        if (f_man(y) == 0) die("F_DIV by a zero mantissa at #FROUND 0 is not defined", -1);
        A = (u128)f_man(x) << (MAN - 1); Q = A / f_man(y);
        if (Q == 0) return f_word(sq, EMIN, 0);
        return pack(0, sq, Q, (i64)f_exp(x) - f_exp(y) - MAN + 1);
    }
    A = (u128)f_man(x) << (2 * MAN + 8); Q = A / f_man(y); R = A % f_man(y);
    Q = (Q << 1) | (R != 0);
    return pack(FROUND, sq, Q, (i64)f_exp(x) - f_exp(y) - (2 * MAN + 8) - 1);
}

static u32 ref_xpo(u32 w)
{
    if ((w & ((1u << (MAN + EXP)) - 1)) == 0) return 0;
    return wd(f_exp(w) + (MAN - 1));
}

static u32 ref_f2i(u32 w)
{
    int e = f_exp(w), s = f_sgn(w);
    u64 m = f_man(w), mag;
    int big = 0;
    if (e >= 0) {
        if (m && msb128(m) + e >= NU - 1) big = 1;
        mag = (e >= 64 - MAN) ? 0 : (m << e);
    } else {
        mag = (-e >= 64) ? 0 : (m >> (-e));
        if (mag >> (NU - 1)) big = 1;
    }
    if (FROUND != 0 && big)
        return s ? (1u << (NU - 1)) : (MASK >> 1);
    if (FROUND != 0) return s ? wd(-(i64)mag) : wd((i64)mag);
    /* level 0 wraps: the low NUBITS bits of the integer */
    mag &= MASK;
    if (e >= 0 && e >= NU) mag = 0;
    return s ? wd(-(i64)mag) : wd((i64)mag);
}

static u32 ref_scl(u32 kw, u32 w)
{
    i64 e = (i64)f_exp(w) + sx(kw);
    if (f_man(w) == 0) return (FROUND != 0) ? ZERO : f_word(f_sgn(w), EMIN, 0);
    if (FROUND != 0 && e < EMIN) return ZERO;
    if (FROUND != 0 && e > EMAX) return f_word(f_sgn(w), EMAX, MMASK);
    if (e > EMAX) e = EMAX;
    if (e < EMIN) e = EMIN;
    return f_word(f_sgn(w), (int)e, f_man(w));
}

static double fval(u32 w)
{
    double v = ldexp((double)f_man(w), f_exp(w) - MAN);
    return f_sgn(w) ? -v : v;
}

/* --------------------------------------------------------- integer ALU --- */

static u32 shl(u32 a, u32 b) { return (b >= (u32)NU) ? 0 : wd((i64)((u64)a << b)); }
static u32 shr(u32 a, u32 b) { return (b >= (u32)NU) ? 0 : (a & MASK) >> b; }
static u32 srs(u32 a, u32 b) { return (b >= (u32)NU) ? wd(sx(a) < 0 ? -1 : 0) : wd(sx(a) >> b); }
static u32 iabs(u32 a) { return sx(a) < 0 ? wd(-sx(a)) : a; }
static u32 b2w(int b) { return b ? 1 : 0; }

static u32 idiv(u32 a, u32 b)
{
    i64 x = sx(a), y = sx(b);
    if (y == 0) return wd(x < 0 ? 1 : -1);                /* #34 */
    return wd(x / y);                                     /* i64: INT_MIN / -1 wraps in wd */
}

static u32 imod(u32 a, u32 b)
{
    i64 x = sx(a), y = sx(b);
    if (y == 0) return a;
    return wd(x % y);
}

/* ----------------------------------------------------------- the opcodes --- */

enum {
    NOP = 0, LOD, P_LOD, LDI, ILI, SET, SET_P, STI, ISI, PSH, POP, INN, F_INN, P_INN, PF_INN, OUT,
    JMP, JIZ, JNZ, CAL, RET, TOM, CAD, ADD, S_ADD, MLT, S_MLT, DIV, S_DIV, MOD, S_MOD, SGN, S_SGN,
    F_ADD, SF_ADD, F_SU1, F_SU2, SF_SU1, SF_SU2, F_MLT, SF_MLT, F_DIV, SF_DIV, F_SGN, SF_SGN,
    F_SCL, SF_SCL, NEG, NEG_M, P_NEG_M, ABS, ABS_M, P_ABS_M, PST, PST_M, P_PST_M, NRM, NRM_M,
    P_NRM_M, INV, INV_M, P_INV_M, LIN, LIN_M, P_LIN_M, F_NEG, F_NEG_M, PF_NEG_M, F_ABS, F_ABS_M,
    PF_ABS_M, F_PST, F_PST_M, PF_PST_M, F_ROT, XPO, XPO_M, I2F, I2F_M, P_I2F_M, F2I, F2I_M,
    P_F2I_M, AND, S_AND, ORR, S_ORR, XOR, S_XOR, LAN, S_LAN, LOR, S_LOR, LES, S_LES, GRE, S_GRE,
    EQU, S_EQU, F_LES, SF_LES, F_GRE, SF_GRE, SHL, S_SHL, SHR, S_SHR, SRS, S_SRS, INC, P_INC,
    QUO, REM, F_QUO, NOPCODES
};

/* a two-operand operation: a is the memory word or the stack's top (in1),
   b the accumulator (in2); the result goes to the accumulator */
static u32 binop(int op, u32 a, u32 b)
{
    switch (op) {
    case ADD: case S_ADD: return wd(sx(a) + sx(b));
    case MLT: case S_MLT: return wd((i64)((u64)sx(a) * (u64)sx(b)));
    case SGN: case S_SGN: return (sx(a) < 0) ? wd(-sx(iabs(b))) : iabs(b);
    case F_ADD: case SF_ADD: return ref_fadd(a, b);
    case F_SU1: case SF_SU1: return ref_fadd(neg_f(a), b);    /* acc - in1 */
    case F_SU2: case SF_SU2: return ref_fadd(a, neg_f(b));    /* in1 - acc */
    case F_MLT: case SF_MLT: return ref_fmlt(a, b);
    case F_SGN: case SF_SGN: return fword(f_sgn(a), f_exp(b), f_man(b));
    case F_SCL: case SF_SCL: return ref_scl(a, b);            /* acc * 2^in1 */
    case AND: case S_AND: return a & b;
    case ORR: case S_ORR: return a | b;
    case XOR: case S_XOR: return a ^ b;
    case LAN: case S_LAN: return b2w(a != 0 && b != 0);
    case LOR: case S_LOR: return b2w(a != 0 || b != 0);
    case LES: case S_LES: return b2w(sx(a) < sx(b));          /* in1 < acc */
    case GRE: case S_GRE: return b2w(sx(a) > sx(b));
    case EQU: case S_EQU: return b2w(a == b);
    case F_LES: case SF_LES: return b2w(fval(a) < fval(b));
    case F_GRE: case SF_GRE: return b2w(fval(a) > fval(b));
    case SHL: case S_SHL: return shl(a, b);                   /* in1 shifted by acc */
    case SHR: case S_SHR: return shr(a, b);
    case SRS: case S_SRS: return srs(a, b);
    }
    die("binop: not a two-operand opcode", -1);
    return 0;
}

/* a one-operand operation on x (the accumulator, or the memory word for _M) */
static u32 unop(int op, u32 x)
{
    switch (op) {
    case NEG: case NEG_M: case P_NEG_M: return wd(-sx(x));
    case ABS: case ABS_M: case P_ABS_M: return iabs(x);
    case PST: case PST_M: case P_PST_M: return sx(x) < 0 ? 0 : x;
    case NRM: case NRM_M: case P_NRM_M: return wd(sx(x) / NUGAIN);
    case INV: case INV_M: case P_INV_M: return wd(~(i64)x);
    case LIN: case LIN_M: case P_LIN_M: return b2w(x == 0);
    case F_NEG: case F_NEG_M: case PF_NEG_M: return fword(!f_sgn(x), f_exp(x), f_man(x));
    case F_ABS: case F_ABS_M: case PF_ABS_M: return x & ((1u << (MAN + EXP)) - 1);
    case F_PST: case F_PST_M: case PF_PST_M: return f_sgn(x) ? ZERO : x;
    case F_ROT: return ref_frot(x);
    case XPO: case XPO_M: return ref_xpo(x);
    case I2F: case I2F_M: case P_I2F_M: return ref_i2f(x);
    case F2I: case F2I_M: case P_F2I_M: return ref_f2i(x);
    }
    die("unop: not a one-operand opcode", -1);
    return 0;
}

/* --------------------------------------------------------------- state --- */

static u32 *imem, *dmem;
static u32 acc;
static u32 *dstk, *istk;
static int dsp, isp;
static long cyc;
static u32 nin;                     /* input reads so far */

static void dpush(u32 v) { if (dsp >= DDEPTH) die("data stack overflow", cyc); dstk[dsp++] = v; }
static u32  dpop(void)   { if (dsp <= 0) die("data stack underflow", cyc); return dstk[--dsp]; }
static void ipush(u32 v) { if (isp >= SDEPTH) die("instruction stack overflow", cyc); istk[isp++] = v; }
static u32  ipop(void)   { if (isp <= 0) die("instruction stack underflow", cyc); return istk[--isp]; }

static u32 daddr(u32 a)
{
    a &= (1u << MDATAW) - 1;
    if ((int)a >= MDATAS) die("data address past the memory", cyc);
    return a;
}

static u32 rd(u32 a) { return dmem[daddr(a)]; }
static void wr(u32 a, u32 v) { a = daddr(a); dmem[a] = v; printf("%ld W %x %x\n", cyc, a, v); }

/* the low FFTSIZ bits of an index reversed, the bits above kept (rel_addr) */
static u32 fft_rev(u32 x)
{
    u32 lo = 0;
    int i;
    for (i = 0; i < FFTSIZ; i++) if ((x >> i) & 1) lo |= 1u << (FFTSIZ - 1 - i);
    return (x & ~((1u << FFTSIZ) - 1)) | lo;
}

static u32 ix(u32 x) { return x & ((1u << MDATAW) - 1); }

/* the divisions started, by cycle: QUO reads the one STG-1 cycles back */
#define DHIST 16
static struct { long c; int fl; u32 a, b; } dh[DHIST];

static void div_start(int fl, u32 a, u32 b) { int s = cyc % DHIST; dh[s].c = cyc; dh[s].fl = fl; dh[s].a = a; dh[s].b = b; }

static int div_back(int fl, u32 *a, u32 *b)
{
    int stg = fl ? (HAS_DIV ? 5 : 4) : 5;                 /* ASM_DIV_WORDS_FLT / _INT */
    long c = cyc - (stg - 1);
    int s = (int)(((c % DHIST) + DHIST) % DHIST);
    if (c < 0 || dh[s].c != c || dh[s].fl != fl) return 0;
    *a = dh[s].a; *b = dh[s].b;
    return 1;
}

/* ---------------------------------------------------------------- main --- */

int main(int argc, char **argv)
{
    long ncyc = -1, itr[64];
    int nitr = 0, i, halt_run = 0, hot = 0, last_d = -1, last_r = -1;
    u32 pc_reg = 1, ir;              /* the PC register, and the word in decode */
    long inc_c = -1; u32 inc_a = 0, inc_v = 0;
    long out_c = -1; u32 out_p = 0, out_v = 0;

    if (argc < 4) {
        fprintf(stderr, "usage: sapho_iss <proc.v> <inst.mif> <data.mif> [-n cycles] [-i c1,c2,...]\n");
        return 2;
    }
    for (i = 4; i < argc; i++) {
        if (!strcmp(argv[i], "-n") && i + 1 < argc) ncyc = atol(argv[++i]);
        else if (!strcmp(argv[i], "-h")) hot = 1;
        else if (!strcmp(argv[i], "-i") && i + 1 < argc) {
            char *p = argv[++i];
            while (*p && nitr < 64) { itr[nitr++] = strtol(p, &p, 10); if (*p == ',') p++; }
        } else { fprintf(stderr, "sapho_iss: unknown option %s\n", argv[i]); return 2; }
    }

    read_params(argv[1]);
    EMIN = -(1 << (EXP - 1)); EMAX = (1 << (EXP - 1)) - 1;
    MMASK = (1u << MAN) - 1; EMASK = (1u << EXP) - 1;
    ZERO = 1u << (MAN + EXP - 1);
    if (1 + EXP + MAN != NU) die("NUBITS is not 1 + NBEXPO + NBMANT", -1);

    imem = read_mif(argv[2], MINSTS);
    dmem = read_mif(argv[3], MDATAS);
    dstk = calloc(DDEPTH + 1, sizeof *dstk);
    istk = calloc(SDEPTH + 1, sizeof *istk);
    for (i = 0; i < DHIST; i++) dh[i].c = -1;

    ir = imem[0];                    /* fetched on the reset edge */
    acc = 0;
    for (cyc = 0; ncyc < 0 || cyc < ncyc; cyc++) {
        int op = (int)(ir >> NBOPER), j, it = 0;
        u32 x = ir & ((1u << NBOPER) - 1);
        u32 fetch = pc_reg, a, b;
        int jump = 0;

        for (j = 0; j < nitr; j++) if (itr[j] == cyc) it = ITRADD > 0;

        if (dsp != last_d) { printf("%ld D %x\n", cyc, dsp); last_d = dsp; }
        if (isp != last_r) { printf("%ld R %x\n", cyc, isp); last_r = isp; }
        if (hot && (op == INC || op == P_INC)) fprintf(stderr, "inc %ld\n", cyc);
        if (hot && dsp > 0) fprintf(stderr, "stk %ld\n", cyc);

        /* what a cycle before left to show now */
        if (inc_c == cyc) { dmem[daddr(inc_a)] = inc_v; printf("%ld W %x %x\n", cyc, daddr(inc_a), inc_v); }
        if (out_c == cyc) printf("%ld O %x %x\n", cyc, out_p, out_v);

        switch (op) {
        case NOP: break;
        case LOD: acc = rd(x); break;
        case P_LOD: dpush(acc); acc = rd(x); break;
        case LDI: acc = rd(x + ix(acc)); break;
        case ILI: acc = rd(x + fft_rev(ix(acc))); break;
        case SET: wr(x, acc); break;
        case SET_P: wr(x, acc); acc = dpop(); break;
        case STI: a = dpop(); wr(x + ix(a), acc); break;
        case ISI: a = dpop(); wr(x + fft_rev(ix(a)), acc); break;
        case PSH: dpush(acc); break;
        case POP: acc = dpop(); break;
        case INN: case F_INN: case P_INN: case PF_INN:
            printf("%ld I %x\n", cyc, x);
            a = iss_in(nin++, x);
            if (op == P_INN || op == PF_INN) dpush(acc);
            acc = (op == F_INN || op == PF_INN) ? ref_i2f(a) : a;
            break;
        case OUT: out_c = cyc + 1; out_p = x; out_v = acc; break;
        case JMP: fetch = x; jump = 1; break;
        case JIZ: if (acc == 0) { fetch = x; jump = 1; } break;
        case JNZ: if (acc != 0) { fetch = x; jump = 1; } break;
        case CAL: ipush(pc_reg); fetch = x; jump = 1; break;
        case RET: fetch = ipop(); jump = 1; break;
        case TOM: case CAD: die("TOM and CAD are not modelled", cyc); break;
        case DIV: case MOD: div_start(0, rd(x), acc); break;
        case S_DIV: case S_MOD: div_start(0, dpop(), acc); break;
        case F_DIV: div_start(1, rd(x), acc); break;
        case SF_DIV: div_start(1, dpop(), acc); break;
        case QUO: if (!div_back(0, &a, &b)) die("QUO with no DIV STG-1 cycles before", cyc); acc = idiv(a, b); break;
        case REM: if (!div_back(0, &a, &b)) die("REM with no MOD STG-1 cycles before", cyc); acc = imod(a, b); break;
        case F_QUO: if (!div_back(1, &a, &b)) die("F_QUO with no F_DIV STG-1 cycles before", cyc); acc = ref_fdiv(a, b); break;
        case INC: case P_INC:
            if (op == P_INC) dpush(acc);
            acc = wd(sx(rd(x)) + 1);
            if (!it) { inc_c = cyc + 1; inc_a = x; inc_v = acc; }
            break;
        case ADD: case MLT: case SGN: case F_ADD: case F_SU1: case F_SU2: case F_MLT: case F_SGN:
        case F_SCL: case AND: case ORR: case XOR: case LAN: case LOR: case LES: case GRE: case EQU:
        case F_LES: case F_GRE: case SHL: case SHR: case SRS:
            acc = binop(op, rd(x), acc); break;
        case S_ADD: case S_MLT: case S_SGN: case SF_ADD: case SF_SU1: case SF_SU2: case SF_MLT:
        case SF_SGN: case SF_SCL: case S_AND: case S_ORR: case S_XOR: case S_LAN: case S_LOR:
        case S_LES: case S_GRE: case S_EQU: case SF_LES: case SF_GRE: case S_SHL: case S_SHR:
        case S_SRS:
            a = dpop(); acc = binop(op, a, acc); break;
        case NEG: case ABS: case PST: case NRM: case INV: case LIN: case F_NEG: case F_ABS:
        case F_PST: case F_ROT: case XPO: case I2F: case F2I:
            acc = unop(op, acc); break;
        case NEG_M: case ABS_M: case PST_M: case NRM_M: case INV_M: case LIN_M: case F_NEG_M:
        case F_ABS_M: case F_PST_M: case XPO_M: case I2F_M: case F2I_M:
            acc = unop(op, rd(x)); break;
        case P_NEG_M: case P_ABS_M: case P_PST_M: case P_NRM_M: case P_INV_M: case P_LIN_M:
        case PF_NEG_M: case PF_ABS_M: case PF_PST_M: case P_I2F_M: case P_F2I_M:
            dpush(acc); acc = unop(op, rd(x)); break;
        default: die("an opcode the instruction set does not have", cyc);
        }

        /* the interrupt: the fetch goes to ITRADD and both stacks empty, the
           push or pop of the instruction in decode with them (#188) */
        if (it) { fetch = ITRADD; jump = 1; dsp = 0; isp = 0; }

        fetch &= (1u << MINSTW) - 1;
        if ((int)fetch >= MINSTS) die("instruction address past the memory", cyc);
        printf("%ld F %x\n", cyc, fetch);

        /* the halt loop: a JMP to itself */
        if (ncyc < 0) {
            if (cyc > 5000000) die("no halt loop reached in 5,000,000 cycles", cyc);
            if (op == JMP && jump && fetch == (pc_reg - 1)) { if (++halt_run >= 4) { cyc++; break; } }
            else halt_run = 0;
        }

        pc_reg = fetch + 1;
        ir = imem[fetch];
    }
    fprintf(stderr, "cycles %ld\n", cyc);
    return 0;
}
