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

#include "../Headers/context.h"
#include "../Headers/convert.h"
#include "../Headers/diag.h"
#include "../Headers/directives.h"
#include "../Headers/global.h"
#include "../Headers/symbols.h"

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

int main(void)
{
    add_var("__acc");                               // as parse_init: id 0 is the accumulator
    test_strip_fn();
    test_fnum_text();
    test_cnum_split();
    test_mant_pow2();
    test_int_to_bin();
    test_lex_id();
    printf("cmm_unit_test: %d checks, %d failed\n", checked, failed);
    return failed ? 1 : 0;
}
