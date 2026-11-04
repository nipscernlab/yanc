// ----------------------------------------------------------------------------
// cmm_unit_test: cmmcomp's helpers tested one by one -------------------------
// ----------------------------------------------------------------------------
// Linked with every object of cmmcomp, the parser's main renamed
// (Scripts/regress.sh), so a test sets the state it needs in cfg, pctx and
// ectx (context.h) and calls a function directly, without a .cmm. An error
// that would end cmmcomp comes back here through fatal_hook and a longjmp.
// Prints one line per failed check and exits 1 if any failed (step P8 of #93).
// ----------------------------------------------------------------------------

#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../Headers/acc.h"
#include "../Headers/context.h"
#include "../Headers/convert.h"
#include "../Headers/diag.h"
#include "../Headers/directives.h"
#include "../Headers/global.h"
#include "../Headers/symbols.h"
#include "../Headers/tpl.h"
#include "../../common/xstr.h"

static int     failed = 0, checked = 0;
static jmp_buf on_fatal;

static void check_str(const char *what, const char *got, const char *want)
{
    checked++;
    if (strcmp(got, want) != 0) { printf("FAIL %s: got '%s', want '%s'\n", what, got, want); failed++; }
}

static void check_int(const char *what, long long got, long long want)
{
    checked++;
    if (got != want) { printf("FAIL %s: got %lld, want %lld\n", what, got, want); failed++; }
}

static void back_here(void) { longjmp(on_fatal, 1); }

// a local is <function>_<name>; anything else keeps its name (#94 A)
static void test_strip_fn(void)
{
    char a[] = "main_x", b[] = "mainly", c[] = "main", d[] = "f_g_x", e[] = "x";
    check_str("strip_fn main_x",  strip_fn(a, "main"), "x");
    check_str("strip_fn mainly",  strip_fn(b, "main"), "mainly");
    check_str("strip_fn main",    strip_fn(c, "main"), "main");
    check_str("strip_fn f_g_x",   strip_fn(d, "f"),    "g_x");
    check_str("strip_fn global",  strip_fn(e, ""),     "x");

    pctx.fname = "main"; ectx.emit_fname = "";
    check_str("strip_fn_prefix while parsing", strip_fn_prefix(a), "x");
    pctx.fname = ""; ectx.emit_fname = "main";
    check_str("strip_fn_prefix while walking", strip_fn_prefix(a), "x");
    ectx.emit_fname = "";
    check_str("strip_fn_prefix at the top", strip_fn_prefix(a), "main_x");
}

// a float literal the one way the .asm reads it
static void test_fnum_text(void)
{
    const char *cases[][2] = {
        {".5", "0.5"}, {"5.", "5.0"}, {"007.5", "7.5"}, {"1e-8", "1.0e-8"},
        {"2.5E+3f", "2.5e3"}, {"-0.25", "-0.25"}, {"000", "0.0"}, {"+3", "3.0"},
        {"1.5L", "1.5"},
    };
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
        char *t = fnum_text(cases[i][0]);
        char what[64]; snprintf(what, sizeof what, "fnum_text %s", cases[i][0]);
        check_str(what, t, cases[i][1]);
        free(t);
    }
    char *long_one = malloc(3002);                 // any length (#66)
    memset(long_one, '7', 3000); long_one[3000] = '.'; long_one[3001] = 0;
    char *t = fnum_text(long_one);
    check_int("fnum_text length of 3000 digits and a point", (long long)strlen(t), 3002);
    free(t); free(long_one);
}

// a comp literal's two parts: a sign after an e belongs to the exponent
static void test_cnum_split(void)
{
    const char *cases[][3] = {
        {"3+4i", "3", "4"}, {"-1.5-2i", "-1.5", "-2"}, {"1e-8+2e-8i", "1e-8", "2e-8"},
        {"0+2.5i", "0", "2.5"}, {"1E+3-1E-3i", "1E+3", "-1E-3"},
    };
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
        char *re, *im, what[64];
        cnum_split(cases[i][0], &re, &im);
        snprintf(what, sizeof what, "cnum_split %s re", cases[i][0]); check_str(what, re, cases[i][1]);
        snprintf(what, sizeof what, "cnum_split %s im", cases[i][0]); check_str(what, im, cases[i][2]);
        free(re); free(im);
    }
}

// 2^NBMANT, defined for any #NBMANT (#94 J)
static void test_mant_pow2(void)
{
    int keep = cfg.nbmant;
    cfg.nbmant = 23; check_int("mant_pow2 23", mant_pow2(), 1LL << 23);
    cfg.nbmant = 31; check_int("mant_pow2 31", mant_pow2(), 1LL << 31);
    cfg.nbmant = 70; check_int("mant_pow2 70 held at 62", mant_pow2(), 1LL << 62);
    cfg.nbmant = -1; check_int("mant_pow2 -1 held at 0", mant_pow2(), 1);
    cfg.nbmant = keep;
}

static void test_int_to_bin(void)
{
    char *b = int_to_bin(5, 8);   check_str("int_to_bin 5, 8",  b, "00000101"); free(b);
    b = int_to_bin(-1, 4);        check_str("int_to_bin -1, 4", b, "1111");     free(b);
    b = int_to_bin(0, 20);        check_str("int_to_bin 0, 20", b, "00000000000000000000"); free(b);
}

// the lexer's checks on a name: 'i' is the imaginary unit, and a name has at
// most CMM_NAME_MAX characters; both stop cmmcomp, which comes back here
static void test_lex_id(void)
{
    pctx.fname = "";
    int stopped = 0;
    fatal_hook(back_here);
    if (setjmp(on_fatal) == 0) { char i[] = "i"; lex_id(i); } else stopped = 1;
    check_int("lex_id i stops", stopped, 1);

    char *name = malloc(CMM_NAME_MAX + 2);
    memset(name, 'n', CMM_NAME_MAX + 1); name[CMM_NAME_MAX + 1] = 0;
    stopped = 0;
    fatal_hook(back_here);
    if (setjmp(on_fatal) == 0) lex_id(name); else stopped = 1;
    check_int("lex_id of CMM_NAME_MAX + 1 stops", stopped, 1);

    name[CMM_NAME_MAX] = 0;                         // exactly the limit: taken
    fatal_hook(back_here);
    stopped = 0;
    int id = -1;
    if (setjmp(on_fatal) == 0) id = lex_id(name); else stopped = 1;
    check_int("lex_id of CMM_NAME_MAX goes through", stopped, 0);
    if (!stopped) check_str("lex_id of CMM_NAME_MAX keeps the name", v_table[id].name, name);
    fatal_hook(NULL);
    free(name);

    pctx.fname = "f";                               // a local: <function>_<name>
    char x[] = "x";
    id = lex_id(x);
    check_str("lex_id of a local", v_table[id].name, "f_x");
    pctx.fname = "";
}

// what the acc holds after each instruction (acc.c, step P9 of #93)
static void after(const char *instr) { acc_after(xstrdup(instr)); }

static void test_acc(void)
{
    emit_peephole_reset();
    after("LOD x\n");
    check_int("acc after LOD x holds x", acc_holds("x"), 1);
    check_int("a second LOD x is dropped", acc_has_load("LOD x\n"), 1);
    check_int("a P_LOD x is not", acc_has_load("P_LOD x\n"), 0);
    after("SET y\n");
    check_int("acc after SET y holds x", acc_holds("x"), 1);
    check_int("acc after SET y holds y", acc_holds("y"), 1);
    acc_now("[dk0+dd1]");
    check_int("acc_now names the acc alone", acc_holds("x"), 0);
    check_int("acc_now's key", acc_holds("[dk0+dd1]"), 1);
    after("PSH\n");
    check_int("a PSH keeps the key", acc_holds("[dk0+dd1]"), 1);
    after("SET dk0\n");
    check_int("a SET drops a key in brackets", acc_holds("[dk0+dd1]"), 0);
    check_int("and the acc is what it set", acc_holds("dk0"), 1);
    after("ADD 1\n");
    check_int("any other instruction forgets", acc_holds("dk0"), 0);
    after("LOD z\n");
    emit_peephole_reset();
    check_int("a label forgets", acc_holds("z"), 0);

    after("LOD v\n");
    acc_now("[k]");
    after("SET_V w 3\n");
    check_int("a SET_V drops a key", acc_holds("[k]"), 0);
    after("LOD v\n");
    after("SET_V w 3\n");
    check_int("a SET_V keeps a name", acc_holds("v"), 1);

    after("STI w\n");
    acc_holds_elem("w[k]");
    check_int("an element after STI", acc_holds("w[k]"), 1);
    after("LDI w\n");
    acc_holds_elem("w[j]");
    check_int("an element after LDI", acc_holds("w[j]"), 1);
    check_int("which is all the acc holds", acc_holds("w[k]"), 0);
    after("ADD 1\n");
    acc_holds_elem("w[i]");
    check_int("no element after any other instruction", acc_holds("w[i]"), 0);

    after("LOD n0\n");                           // 32 names at most: the oldest goes
    for (int i = 1; i <= 32; i++) { char *set = xasprintf("SET n%d\n", i); after(set); free(set); }
    check_int("the 33rd name drops the oldest", acc_holds("n0"), 0);
    check_int("and keeps the next one", acc_holds("n1"), 1);
    check_int("and holds the newest", acc_holds("n32"), 1);
    emit_peephole_reset();
}

// the instructions template t gives for e1 and e2 (tpl.c, step P10 of #93)
static char *run_tpl(const char *t, expr e1, expr e2, const char *const *ops)
{
    FILE *keep_asm = ectx.f_asm, *keep_lin = ectx.f_lin;
    ectx.f_asm = tmpfile(); ectx.f_lin = tmpfile();
    if (!ectx.f_asm || !ectx.f_lin) { printf("FAIL tpl: no temporary file\n"); exit(1); }
    emit_peephole_reset();
    emit_tpl_ops(t, e1, e2, ops);
    long n = ftell(ectx.f_asm);
    char *out = xmalloc((size_t)n + 1);
    rewind(ectx.f_asm);
    out[fread(out, 1, (size_t)n, ectx.f_asm)] = 0;
    fclose(ectx.f_asm); fclose(ectx.f_lin);
    ectx.f_asm = keep_asm; ectx.f_lin = keep_lin;
    return out;
}

static void check_tpl(const char *what, const char *t, expr e1, expr e2,
                      const char *const *ops, const char *want)
{
    char *got = run_tpl(t, e1, e2, ops);
    check_str(what, got, want);
    free(got);
}

static void test_tpl(void)
{
    pctx.fname = ""; ectx.emit_fname = "";
    char nx[] = "x", ny[] = "y", nf[] = "f", nc[] = "c", nd[] = "d", nk[] = "1+2i";
    expr x = expr_make(TY_INT, lex_id(nx)), y = expr_make(TY_INT, lex_id(ny));
    expr f = expr_make(TY_FLOAT, lex_id(nf));
    expr c = expr_make(TY_COMP, lex_id(nc)), d = expr_make(TY_COMP, lex_id(nd));
    add_var(nk);
    expr k = expr_make(TY_CCONST, find_var(nk));
    expr ia = expr_make(TY_INT, 0), fa = expr_make(TY_FLOAT, 0), ca = expr_make(TY_COMP, 0);

    check_int("place_of int var",    place_of(x),  PL_IM);
    check_int("place_of int acc",    place_of(ia), PL_IA);
    check_int("place_of float var",  place_of(f),  PL_FM);
    check_int("place_of float acc",  place_of(fa), PL_FA);
    check_int("place_of comp var",   place_of(c),  PL_CM);
    check_int("place_of comp const", place_of(k),  PL_CM);
    check_int("place_of comp acc",   place_of(ca), PL_CA);

    int keep = acc_live;
    acc_live = 0;
    check_tpl("tpl $a $b, acc free", "$L $a\nADD $b", x, y, NULL, "LOD x\nADD y\n");
    check_tpl("tpl $C, acc free",    "$C $a\nF_ADD $b", x, f, NULL, "I2F_M x\nF_ADD f\n");
    acc_live = 1;
    check_tpl("tpl $L, acc live",    "$L $a\nADD $b", x, y, NULL, "P_LOD x\nADD y\n");
    check_tpl("tpl $C, acc live",    "$C $a\nF_ADD $b", x, f, NULL, "P_I2F_M x\nF_ADD f\n");
    acc_live = 0;
    check_tpl("tpl comp var parts", "$L $ar\nF_ADD $br\nP_LOD $ai\nF_ADD $bi", c, d, NULL,
              "LOD c\nF_ADD d\nP_LOD c_i\nF_ADD d_i\n");
    check_tpl("tpl comp const parts", "F_ADD $br\nP_LOD $bi", fa, k, NULL, "F_ADD 1.0\nP_LOD 2.0\n");
    const char *const words[] = {"W1", "W2", "W3", "W4", "W5", "W6", "W7", "W8", "W9"};
    check_tpl("tpl $1 .. $9", "$1 $2\n$9 $5", ia, ia, words, "W1 W2\nW9 W5\n");
    check_tpl("tpl NULL emits nothing", NULL, x, y, NULL, "");
    check_tpl("tpl a ?MSG_ line is no instruction", "?MSG_WARN_IDX1_FLOAT\nADD $b", ia, y, NULL, "ADD y\n");
    acc_live = keep;

    int stopped = 0;
    fatal_hook(back_here);
    if (setjmp(on_fatal) == 0) free(run_tpl("?MSG_NO_SUCH_WARNING", ia, ia, NULL)); else stopped = 1;
    fatal_hook(NULL);
    check_int("tpl an unknown ?MSG_ stops", stopped, 1);
}

int main(void)
{
    add_var("__acc");                               // as parse_init: id 0 is the accumulator
    test_strip_fn();
    test_fnum_text();
    test_cnum_split();
    test_mant_pow2();
    test_int_to_bin();
    test_lex_id();
    test_acc();
    test_tpl();
    printf("cmm_unit_test: %d checks, %d failed\n", checked, failed);
    return failed ? 1 : 0;
}
