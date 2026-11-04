// ----------------------------------------------------------------------------
// ALU operations -------------------------------------------------------------
// ----------------------------------------------------------------------------

// global includes
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

// local includes
#include "../Headers/convert.h"
#include "../Headers/oper.h"
#include "../Headers/stdlib.h"
#include "../Headers/global.h"
#include "../Headers/data_use.h"
#include "../Headers/directives.h"
#include "../Headers/symbols.h"
#include "../Headers/messages.h"
#include "../Headers/diag.h"
#include "../Headers/tpl.h"
#include "../../common/xstr.h"

// an int literal a float meets: the id of its float literal, which a memory
// form takes as it is (exact below 2^NBMANT, as add_instr's I2F_M rewrite); 0
// for any other operand
static int int_flit_id(expr e)
{
    if (e.type != TY_INT || e.id == 0 || v_table[e.id].is_const != 1) return 0;
    long n; char end[8];
    if (sscanf(v_table[e.id].name, "%ld%7s", &n, end) != 1 || labs(n) >= mant_pow2()) return 0;
    char t[32]; snprintf(t, sizeof t, "%ld.0", n);
    return lex_fnum(t);
}

// an int literal a float or a comp meets is taken as that float literal, so the
// float forms load it or take it from memory and nothing is converted
static void int_lit_float(expr *e1, expr *e2)
{
    int id;
    if (e2->type >= TY_FLOAT && (id = int_flit_id(*e1))) *e1 = expr_make(TY_FLOAT, id);
    if (e1->type >= TY_FLOAT && (id = int_flit_id(*e2))) *e2 = expr_make(TY_FLOAT, id);

    // a comp constant whose imaginary part is zero is a float, its real part
    for (expr *e = e1; e; e = (e == e1) ? e2 : NULL)
        if (e->type == TY_CCONST)
        {
            expr er, ei; get_cmp_cst(*e, &er, &ei);
            if (atof(v_table[ei.id].name) == 0.0) *e = num2exp(er.id, 2);
        }
}

// ----------------------------------------------------------------------------
// arithmetic operations ------------------------------------------------------
// ----------------------------------------------------------------------------

// negates a number
expr oper_neg(expr e)
{
    expr   etr, eti;

    char  neg[10]; if (acc_live == 0) strcpy( neg,   "NEG_M"); else strcpy( neg,  "P_NEG_M");
    char fneg[10]; if (acc_live == 0) strcpy(fneg, "F_NEG_M"); else strcpy(fneg, "PF_NEG_M");

    // a float or comp literal needs no instruction: its negation is another
    // literal (-2.5 is loaded as such, x - (3+4i) adds -3-4i). Not an int one: the
    // compiler holds int literals non-negative (lex_inum, builtin_pow's exponent
    // loop). Not when a float part is zero either: "-0.0" encodes as +0, and
    // F_NEG_M 0.0 gives -0 (TODO.md item 4)
    if (e.type == TY_FLOAT && e.id != 0 && v_table[e.id].is_const)
    {
        char *nm = v_table[e.id].name;
        if (atof(nm) != 0.0)
        {
            char *txt = nm[0] == '-' ? xstrdup(nm + 1) : xasprintf("-%s", nm);
            int id = lex_fnum(txt);
            free(txt);
            return num2exp(id, TY_FLOAT);
        }
    }
    if (e.type == TY_CCONST)
    {
        char *re, *im;
        cnum_split(v_table[e.id].name, &re, &im);
        int id = -1;
        if (atof(re) != 0.0 && atof(im) != 0.0)
        {   // each part's sign flipped on the text, as for a float above
            char *txt = xasprintf("%s%s%s%si", re[0] == '-' ? "" : "-", re + (re[0] == '-'),
                                               im[0] == '-' ? "+" : "-", im + (im[0] == '-'));
            id = lex_cnum(txt);
            free(txt);
        }
        free(re); free(im);
        if (id >= 0) return num2exp(id, TY_CCONST);
    }

    // when it is an int variable in memory
    if ((e.type == TY_INT) && (e.id != 0))
    {
        add_instr("%s %s\n", neg, v_table[e.id].name);
    }

    // when it is an int in the acc
    if ((e.type == TY_INT) && (e.id == 0))
    {
        add_instr("NEG\n");
    }

    // when it is a float variable in memory
    if ((e.type == TY_FLOAT) && (e.id != 0))
    {
        add_instr("%s %s\n", fneg, v_table[e.id].name);
    }

    // when it is a float in the acc
    if ((e.type == TY_FLOAT) && (e.id == 0))
    {
        add_instr("F_NEG\n");
    }

    // when it is a comp in memory (a constant or a variable)
    if (place_of(e) == PL_CM)
    {
        get_cmp_parts(e,&etr,&eti);

        add_instr("%s %s\n", fneg, v_table[etr.id].name);
        add_instr("PF_NEG_M %s\n", v_table[eti.id].name);
    }

    // when it is a comp in the acc
    if ((e.type == TY_COMP) && (e.id == 0))
    {
        add_instr("   SET_P __aux_var\n");
        add_instr(" F_NEG\n"          );
        add_instr("PF_NEG_M __aux_var\n");
    }

    acc_live = 1;

    // a comp constant got materialized into the comp (acc) representation
    if (e.type == TY_CCONST) e.type = TY_COMP;

    return expr_make(e.type, 0);
}

// the type of a binary operation's result: comp over float over int
static int result_type(expr e1, expr e2)
{
    if (e1.type > TY_FLOAT || e2.type > TY_FLOAT) return TY_COMP;
    if (e1.type > TY_INT   || e2.type > TY_INT  ) return TY_FLOAT;
    return TY_INT;
}

// addition: left operand by row, right by column (IM IA FM FA CM CA)
static const char *const add_tpl[PL_N][PL_N] = {
  /* IM */ {"$L $a\nADD $b", "ADD $a", "$C $a\nF_ADD $b", "P_I2F_M $a\nSF_ADD",
            "$C $a\nF_ADD $br\nP_LOD $bi", "SET __aux_var\nI2F_M $a\nSF_ADD\nP_LOD __aux_var"},
  /* IA */ {"ADD $b", "S_ADD", "I2F\nF_ADD $b", "SET_P __aux_var\nI2F\nF_ADD __aux_var",
            "I2F\nF_ADD $br\nP_LOD $bi", "SET_P __aux_var\nSET_P __aux_var1\nI2F\nF_ADD __aux_var1\nP_LOD __aux_var"},
  /* FM */ {"$C $b\nF_ADD $a", "I2F\nF_ADD $a", "$L $a\nF_ADD $b", "F_ADD $a",
            "$L $a\nF_ADD $br\nP_LOD $bi", "SET_P __aux_var\nF_ADD $a\nP_LOD __aux_var"},
  /* FA */ {"P_I2F_M $b\nSF_ADD", "I2F\nSF_ADD", "F_ADD $b", "SF_ADD",
            "F_ADD $br\nP_LOD $bi", "SET_P __aux_var\nSF_ADD\nP_LOD __aux_var"},
  /* CM */ {"$C $b\nF_ADD $ar\nP_LOD $ai", "I2F\nF_ADD $ar\nP_LOD $ai", "$L $b\nF_ADD $ar\nP_LOD $ai", "F_ADD $ar\nP_LOD $ai",
            "$L $ar\nF_ADD $br\nP_LOD $ai\nF_ADD $bi", "SET_P __aux_var\nF_ADD $ar\nP_LOD $ai\nF_ADD __aux_var"},
  /* CA */ {"SET_P __aux_var\nP_I2F_M $b\nSF_ADD\nP_LOD __aux_var", "SET_P __aux_var\nSET_P __aux_var1\nP_I2F_M __aux_var\nSF_ADD\nP_LOD __aux_var1",
            "SET_P __aux_var\nF_ADD $b\nP_LOD __aux_var", "SET_P __aux_var\nSET_P __aux_var1\nF_ADD __aux_var\nP_LOD __aux_var1",
            "SET_P __aux_var\nF_ADD $br\nP_LOD __aux_var\nF_ADD $bi", "SET_P __aux_var\nSET_P __aux_var1\nSET_P __aux_var2\nF_ADD __aux_var1\nP_LOD __aux_var2\nF_ADD __aux_var"},
};

// adds two numbers
expr oper_add(expr e1, expr e2)
{
    int_lit_float(&e1, &e2);
    emit_tpl(add_tpl[place_of(e1)][place_of(e2)], e1, e2);
    acc_live = 1;
    return expr_make(result_type(e1, e2), 0);
}

// oper_sub: the left operand by row, the right one by column (IM IA FM FA CM CA)
static const char *const sub_tpl[PL_N][PL_N] = {
  /* IM */ {NULL, NULL, "$C $a\nF_SU1 $b", "$C $a\nSF_SU1", "$C $a\nF_SU1 $br\nPF_NEG_M $bi", "SET __aux_var\nI2F_M $a\nSF_SU1\nPF_NEG_M __aux_var"},
  /* IA */ {NULL, NULL, "I2F\nF_SU1 $b", "SET_P __aux_var\nI2F\nF_SU1 __aux_var", "I2F\nF_SU1 $br\nPF_NEG_M $bi", "SET_P __aux_var\nSET_P __aux_var1\nI2F\nF_SU1 __aux_var1\nPF_NEG_M __aux_var"},
  /* FM */ {"$C $b\nF_SU2 $a", "I2F\nF_SU2 $a", "$L $a\nF_SU1 $b", "F_SU2 $a", "$L $a\nF_SU1 $br\nPF_NEG_M $bi", "SET_P __aux_var\nF_SU2 $a\nPF_NEG_M __aux_var"},
  /* FA */ {"P_I2F_M $b\nSF_SU2", "I2F\nSF_SU2", "F_SU1 $b", "SF_SU2", "F_SU1 $br\nPF_NEG_M $bi", "SET_P __aux_var\nSF_SU2\nPF_NEG_M __aux_var"},
  /* CM */ {"$C $b\nF_SU2 $ar\nP_LOD $ai", "I2F\nF_SU2 $ar\nP_LOD $ai", "$L $b\nF_SU2 $ar\nP_LOD $ai", "F_SU2 $ar\nP_LOD $ai", "$L $ar\nF_SU1 $br\nP_LOD $ai\nF_SU1 $bi", "SET_P __aux_var\nF_SU2 $ar\nP_LOD $ai\nF_SU1 __aux_var"},
  /* CA */ {"SET_P __aux_var\nP_I2F_M $b\nSF_SU2\nP_LOD __aux_var", "SET_P __aux_var\nSET_P __aux_var1\nP_I2F_M __aux_var\nSF_SU2\nP_LOD __aux_var1", "SET_P __aux_var\nF_SU1 $b\nP_LOD __aux_var", "SET_P __aux_var\nSET_P __aux_var1\nF_SU1 __aux_var\nP_LOD __aux_var1", "SET_P __aux_var\nF_SU1 $br\nP_LOD __aux_var\nF_SU1 $bi", "SET_P __aux_var\nSET_P __aux_var1\nSET_P __aux_var2\nF_SU1 __aux_var1\nP_LOD __aux_var2\nF_SU1 __aux_var"},
};
// subtraction between two numbers
expr oper_sub(expr e1, expr e2)
{

    int_lit_float(&e1, &e2);

    // minus a comp constant is plus its negation, part by part. Unlike
    // oper_neg, a zero part is fine here: it stays 0.0, x - 0.0 and x + 0.0
    // are the same value, and add_instr drops the F_ADD 0.0
    if (e2.type == TY_CCONST)
    {
        char *re, *im;
        cnum_split(v_table[e2.id].name, &re, &im);
        int zr = atof(re) == 0.0, zi = atof(im) == 0.0;
        char *txt = xasprintf("%s%s%s%si",
                 zr ? "" : (re[0] == '-' ? "" : "-"), zr ? "0.0" : re + (re[0] == '-'),
                 zi ? "+" : (im[0] == '-' ? "+" : "-"), zi ? "0.0" : im + (im[0] == '-'));
        int id = lex_cnum(txt);
        free(txt); free(re); free(im);
        return oper_add(e1, num2exp(id, TY_CCONST));
    }

    char ld [10]; if (acc_live == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );

    // int minus an int literal: ADD of its negation, one word (the literal
    // stays non-negative in the table, as oper_neg wants; only the operand
    // text carries the sign). x - 1 is LOD x; ADD -1, not NEG_M 1; ADD x
    if ((e1.type==TY_INT) && (e2.type==TY_INT) && (e2.id!=0) && v_table[e2.id].is_const
        && strspn(v_table[e2.id].name, "0123456789") == strlen(v_table[e2.id].name))
    {
        if (e1.id != 0) add_instr("%s %s\n", ld, v_table[e1.id].name);
        add_instr("ADD -%s\n", v_table[e2.id].name);
        acc_live = 1;
        return expr_make(TY_INT, 0);
    }

    // an int minus an int is an int plus its negation: sub_tpl has no int cell
    if ((e1.type==TY_INT) && (e2.type==TY_INT)) return oper_add(e1, oper_neg(e2));

    emit_tpl(sub_tpl[place_of(e1)][place_of(e2)], e1, e2);

    // comp var with comp var: the walker computes it part by part (ast.c,
    // comp_partwise), so it does not reach here

    acc_live = 1;

    return expr_make(result_type(e1, e2), 0);
}

// oper_mult: the left operand by row, the right one by column (IM IA FM FA CM CA)
static const char *const mult_tpl[PL_N][PL_N] = {
  /* IM */ {"$L $a\nMLT $b", "MLT $a", "$C $a\nF_MLT $b", "P_I2F_M $a\nSF_MLT", "$C $a\nF_MLT $br\nP_I2F_M $a\nF_MLT $bi", "SET_P __aux_var\nP_I2F_M $a\nSF_MLT\nP_LOD __aux_var\nP_I2F_M $a\nSF_MLT"},
  /* IA */ {"MLT $b", "S_MLT", "I2F\nF_MLT $b", "SET_P __aux_var\nI2F\nF_MLT __aux_var", "I2F\nSET __aux_var\nF_MLT $br\nP_LOD __aux_var\nF_MLT $bi", "SET_P __aux_var\nSET_P __aux_var1\nI2F\nSET __aux_var2\nF_MLT __aux_var1\nP_LOD __aux_var\nF_MLT __aux_var2"},
  /* FM */ {"$C $b\nF_MLT $a", "I2F\nF_MLT $a", "$L $a\nF_MLT $b", "F_MLT $a", "$L $a\nF_MLT $br\nP_LOD $a\nF_MLT $bi", "SET_P __aux_var\nF_MLT $a\nP_LOD __aux_var\nF_MLT $a"},
  /* FA */ {"P_I2F_M $b\nSF_MLT", "I2F\nSF_MLT", "F_MLT $b", "SF_MLT", "SET __aux_var\nF_MLT $br\nP_LOD $bi\nF_MLT __aux_var", "SET_P __aux_var\nSET_P __aux_var1\nSET __aux_var2\nF_MLT __aux_var1\nP_LOD __aux_var\nF_MLT __aux_var2"},
  /* CM */ {"$C $b\nF_MLT $ar\nP_I2F_M $b\nF_MLT $ai", "I2F\nSET __aux_var\nF_MLT $ar\nP_LOD __aux_var\nF_MLT $ai", "$L $b\nF_MLT $ar\nP_LOD $b\nF_MLT $ai", "SET __aux_var\nF_MLT $ar\nP_LOD __aux_var\nF_MLT $ai", "$L $ar\nF_MLT $br\nP_LOD $ai\nF_MLT $bi\nSF_SU2\nP_LOD $ar\nF_MLT $bi\nP_LOD $ai\nF_MLT $br\nSF_ADD", "SET_P __aux_var\nSET __aux_var1\nF_MLT $ar\nP_LOD $ai\nF_MLT __aux_var\nSF_SU2\nP_LOD $ar\nF_MLT __aux_var\nP_LOD $ai\nF_MLT __aux_var1\nSF_ADD"},
  /* CA */ {"SET_P __aux_var\nP_I2F_M $b\nSET __aux_var1\nSF_MLT\nP_LOD __aux_var\nF_MLT __aux_var1", "I2F\nSET_P __aux_var\nSET_P __aux_var1\nF_MLT __aux_var\nP_LOD __aux_var1\nF_MLT __aux_var", "SET_P __aux_var\nF_MLT $b\nP_LOD __aux_var\nF_MLT $b", "SET_P __aux_var\nSET_P __aux_var1\nF_MLT __aux_var\nP_LOD __aux_var1\nF_MLT __aux_var", "SET_P __aux_var\nSET __aux_var1\nF_MLT $br\nP_LOD __aux_var\nF_MLT $bi\nSF_SU2\nP_LOD __aux_var\nF_MLT $br\nP_LOD __aux_var1\nF_MLT $bi\nSF_ADD", "SET_P __aux_var\nSET_P __aux_var1\nSET_P __aux_var2\nSET __aux_var3\nF_MLT __aux_var1\nP_LOD __aux_var\nF_MLT __aux_var2\nSF_SU2\nP_LOD __aux_var1\nF_MLT __aux_var2\nP_LOD __aux_var\nF_MLT __aux_var3\nSF_ADD"},
};
// multiplies two numbers
expr oper_mult(expr e1, expr e2)
{
    expr etr, eti;

    int_lit_float(&e1, &e2);

    // a comp times a pure imaginary constant ci: (a+bi)(ci) = -bc + aci, two
    // products and no sum (the multiplication is commutative: the constant goes
    // right)
    if (e1.type == TY_CCONST && e2.type == TY_COMP) { expr t = e1; e1 = e2; e2 = t; }
    if (e1.type == TY_COMP && e2.type == TY_CCONST)
    {
        expr er, ei; get_cmp_cst(e2, &er, &ei);
        char *c = v_table[ei.id].name;
        if (atof(v_table[er.id].name) == 0.0 && atof(c) != 0.0)
        {
            char *nt = c[0] == '-' ? xstrdup(c + 1) : xasprintf("-%s", c);
            int nid = lex_fnum(nt);                                   // -c
            free(nt);
            if (e1.id != 0)                                            // in memory: 4 words
            {
                get_cmp_ets(e1, &etr, &eti);
                add_instr("%s %s\n", acc_live ? "P_LOD" : "LOD", v_table[eti.id].name);
                add_instr("F_MLT %s\n", v_table[nid].name);            // real: -c b
                add_instr("P_LOD %s\n", v_table[etr.id].name);
                add_instr("F_MLT %s\n", c);                            // imag: c a
            }
            else                                                       // in the acc (real on the stack)
            {
                add_instr("F_MLT %s\n", v_table[nid].name);            // -c b
                add_instr("SET_P __aux_var\n");                        // the real part back in the acc
                add_instr("F_MLT %s\n", c);                            // c a
                add_instr("SET __aux_var1\n");
                add_instr("LOD __aux_var\n");
                add_instr("P_LOD __aux_var1\n");
            }
            acc_live = 1;
            return expr_make(TY_COMP, 0);
        }
    }

    emit_tpl(mult_tpl[place_of(e1)][place_of(e2)], e1, e2);

    // a comp in memory by a comp in memory (a constant by a constant, a
    // variable by a constant) is the table's [CM][CM]: a constant left of a
    // variable or the acc was swapped above
    // comp var with comp var: the walker computes it part by part (ast.c,
    // comp_partwise), so it does not reach here

    acc_live = 1;

    return expr_make(result_type(e1, e2), 0);
}

// Emits er² + ei² into the accumulator -- the |comp|² used as the division
// denominator and by mod2/abs. er/ei are the real/imag halves (data-memory
// cells). The sum is commutative, so square whichever half the accumulator
// already holds first: its LOD is then dropped by the add_instr peephole (e.g.
// `r = x/y` right after `y = ...` leaves d live, so squaring d² first saves the
// load). When neither half is live it falls to the original er²-first order, so
// the emitted assembly is unchanged.
void emit_sq_sum(expr er, expr ei)
{
    if (ei.id != 0 && acc_holds(v_table[ei.id].name) &&
        !(er.id != 0 && acc_holds(v_table[er.id].name)))
    {
        oper_mult(ei, ei);
        oper_mult(er, er);
    }
    else
    {
        oper_mult(er, er);
        oper_mult(ei, ei);
    }
    oper_add(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));
}

// oper_div: the left operand by row, the right one by column (IM IA FM FA CM CA)
static const char *const div_tpl[PL_N][PL_N] = {
  /* IM */ {"$L $b\nDIV $a", "DIV $a", "$C $a\nP_LOD $b\nSF_DIV", "SET __aux_var\nI2F_M $a\nP_LOD __aux_var\nSF_DIV", NULL, NULL},
  /* IA */ {"$L $b\nS_DIV", "S_DIV", "I2F\nP_LOD $b\nSF_DIV", "SET_P __aux_soma\nI2F\nP_LOD __aux_soma\nSF_DIV", NULL, NULL},
  /* FM */ {"$C $b\nF_DIV $a", "I2F\nF_DIV $a", "$L $b\nF_DIV $a", "F_DIV $a", NULL, NULL},
  /* FA */ {"P_I2F_M $b\nSF_DIV", "I2F\nSF_DIV", "P_LOD $b\nSF_DIV", "SF_DIV", NULL, NULL},
  /* CM */ {NULL, NULL, NULL, NULL, NULL, NULL},
  /* CA */ {NULL, NULL, NULL, NULL, NULL, NULL},
};
// division between two numbers
expr oper_div(expr e1, expr e2)
{
    expr etr, eti;

    int_lit_float(&e1, &e2);

    // a division by a float literal is a multiplication by its reciprocal: one
    // word, and no divider. A power of two has an exact reciprocal, so it always
    // is. Any other literal gives the reciprocal's rounding, which the program
    // takes only to keep the float divider out (Luciano, 2026-10-31, issue
    // #69): both forms are emitted, the division between #DIVK and #MULK, the
    // multiplication between #MULK and #ENDK, and asm_divk keeps the division
    // when anything else in the program needs the divider, else the product
    static int divk_inner = 0;
    if (!divk_inner && e2.type == TY_FLOAT && e2.id != 0 && v_table[e2.id].is_const == 1)
    {
        double c = strtod(v_table[e2.id].name, NULL), r = 1.0 / c;
        if (c != 0.0 && isfinite(r) && fabs(r) < 1e37 && fabs(r) > 1e-37)
        {
            char t[40]; snprintf(t, sizeof t, "%.9g", r);
            if (!strpbrk(t, ".eE")) strcat(t, ".0");
            ectx.fnum_quiet = 1; int rid = lex_fnum(t); ectx.fnum_quiet = 0;   // the user did not write it
            int ex; if (fabs(frexp(c, &ex)) == 0.5) return oper_mult(e1, num2exp(rid, TY_FLOAT));

            int acc0 = acc_live;
            add_sinst(0, "#DIVK\n");
            divk_inner = 1; oper_div(e1, e2); divk_inner = 0;
            add_sinst(0, "#MULK\n");
            acc_live = acc0;
            expr m = oper_mult(e1, num2exp(rid, TY_FLOAT));
            add_sinst(0, "#ENDK\n");
            return m;
        }
    }

    emit_tpl(div_tpl[place_of(e1)][place_of(e2)], e1, e2);

    // int var with comp in memory
    if ((e1.type==TY_INT) && (e1.id!=0) && (place_of(e2) == PL_CM))
    {
        get_cmp_parts(e2,&etr,&eti);

        emit_sq_sum(etr, eti);
        add_instr("SET __aux_var\n");   // save the result

        acc_live = 0;                   // acc released

        oper_mult(e1, etr);           // multiply int with real part
        add_instr("P_LOD __aux_var\n"); // fetch the denominator
        oper_div(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));     // divide

        oper_mult(e1, eti);           // multiply int with imag part
        add_instr("P_LOD __aux_var\n"); // fetch the denominator
        oper_div(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));     // divide
        oper_neg (expr_make(TY_FLOAT, 0));            // negate the imaginary part
    }


    // int var with comp acc
    if ((e1.type==TY_INT) && (e1.id!=0) && (e2.type==TY_COMP) && (e2.id==0))
    {
        add_instr("SET_P __aux_var \n"); // save the imaginary part
        add_instr("SET   __aux_var1\n"); // save the real part

        add_instr("F_MLT __aux_var1\n"); // real part squared
        add_instr("P_LOD __aux_var \n"); // fetch the imaginary part
        add_instr("F_MLT __aux_var \n"); // imaginary part squared
        add_instr("SF_ADD        \n"); // sum of the squares
        add_instr("SET   __aux_var2\n"); // save the squared magnitude

        add_instr("I2F_M %s\n", v_table[e1.id].name); 
        add_instr("SET   __aux_var3\n"); // save the float
        add_instr("F_MLT __aux_var1\n");
        add_instr("P_LOD __aux_var2\n"); // fetch the squared magnitude
        oper_div(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));      // divide

        add_instr("P_LOD __aux_var3\n"); // fetch the float
        add_instr("F_MLT __aux_var \n");
        add_instr("P_LOD __aux_var2\n"); // fetch the squared magnitude
        oper_div(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));      // divide
        oper_neg (expr_make(TY_FLOAT, 0));
    }

    // int acc with comp in memory
    if ((e1.type==TY_INT) && (e1.id==0) && (place_of(e2) == PL_CM))
    {
        get_cmp_parts(e2,&etr,&eti);

        add_instr("I2F\n");
        add_instr("SET __aux_var\n");
        acc_live = 0;

        emit_sq_sum(etr, eti);
        add_instr("SET   __aux_var1\n"); // save the result

        add_instr("LOD   __aux_var\n");
        oper_mult(expr_make(TY_FLOAT, 0), etr);         // multiply float with real part
        add_instr("P_LOD __aux_var1\n"); // fetch the denominator
        oper_div(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));      // divide

        add_instr("P_LOD  __aux_var\n");
        oper_mult(expr_make(TY_FLOAT, 0), eti);         // multiply float with imag part
        add_instr("P_LOD __aux_var1\n"); // fetch the denominator
        oper_div(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));      // divide
        oper_neg (expr_make(TY_FLOAT, 0));
    }


    // int acc with comp acc
    if ((e1.type==TY_INT) && (e1.id==0) && (e2.type==TY_COMP) && (e2.id==0))
    {
        add_instr("SET_P __aux_var \n"); // save the imaginary part
        add_instr("SET_P __aux_var1\n"); // save the real part
        add_instr("I2F           \n");
        add_instr("SET   __aux_var2\n"); // save the float

        add_instr("LOD   __aux_var1\n"); // push the real part onto the stack
        add_instr("F_MLT __aux_var1\n");
        add_instr("P_LOD __aux_var \n"); // fetch the imaginary part
        add_instr("F_MLT __aux_var \n");
        add_instr("SF_ADD        \n");
        add_instr("SET   __aux_var3\n"); // save the squared magnitude

        add_instr("LOD   __aux_var2\n"); // load the float
        add_instr("F_MLT __aux_var1\n");
        add_instr("P_LOD __aux_var3\n"); // fetch the squared magnitude
        oper_div(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));      // divide

        add_instr("P_LOD __aux_var2\n"); // fetch the float
        add_instr("F_MLT __aux_var \n");
        add_instr("P_LOD __aux_var3\n"); // fetch the squared magnitude
        oper_div(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));      // divide
        oper_neg (expr_make(TY_FLOAT, 0));
    }

    // float var with comp in memory
    if ((e1.type==TY_FLOAT) && (e1.id!=0) && (place_of(e2) == PL_CM))
    {
        get_cmp_parts(e2,&etr,&eti);

        emit_sq_sum(etr, eti);
        add_instr("SET   __aux_var\n"); // save the result
        acc_live = 0;

        oper_mult(e1, etr);           // multiply float with real part
        add_instr("P_LOD __aux_var\n"); // fetch the denominator
        oper_div(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));     // divide

        oper_mult(e1, eti);           // multiply float with imag part
        add_instr("P_LOD __aux_var\n"); // fetch the denominator
        oper_div(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));     // divide
        oper_neg (expr_make(TY_FLOAT, 0));
    }


    // float var with comp acc
    if ((e1.type==TY_FLOAT) && (e1.id!=0) && (e2.type==TY_COMP) && (e2.id==0))
    {
        add_instr("SET_P __aux_var \n");             // save the imaginary part
        add_instr("SET   __aux_var1\n");             // save the real part
        add_instr("F_MLT __aux_var1\n");
        add_instr("P_LOD __aux_var \n");             // fetch the imaginary part
        add_instr("F_MLT __aux_var \n");
        add_instr("SF_ADD        \n");
        add_instr("SET   __aux_var2\n");             // save the squared magnitude

        add_instr("LOD %s\n"  , v_table[e1.id].name); // load the float
        add_instr("F_MLT __aux_var1\n");
        add_instr("P_LOD __aux_var2\n");             // fetch the squared magnitude
        oper_div(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));                  // divide

        add_instr("P_LOD %s\n", v_table[e1.id].name); // load the float
        add_instr("F_MLT __aux_var \n");
        add_instr("P_LOD __aux_var2\n");             // fetch the squared magnitude
        oper_div(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));                  // divide
        oper_neg (expr_make(TY_FLOAT, 0));
    }

    // float acc with comp in memory
    if ((e1.type==TY_FLOAT) && (e1.id==0) && (place_of(e2) == PL_CM))
    {
        get_cmp_parts(e2,&etr,&eti);

        add_instr("SET   __aux_var\n");
        acc_live = 0;

        emit_sq_sum(etr, eti);
        add_instr("SET   __aux_var1\n"); // save the result

        add_instr("LOD   __aux_var \n");
        oper_mult(expr_make(TY_FLOAT, 0), etr);         // multiply float with real part
        add_instr("P_LOD __aux_var1\n"); // fetch the denominator
        oper_div(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));      // divide

        add_instr("P_LOD __aux_var \n");
        oper_mult(expr_make(TY_FLOAT, 0), eti);         // multiply float with imag part
        add_instr("P_LOD __aux_var1\n"); // fetch the denominator
        oper_div(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));      // divide
        oper_neg (expr_make(TY_FLOAT, 0));
    }


    // float acc with comp acc
    if ((e1.type==TY_FLOAT) && (e1.id==0) && (e2.type==TY_COMP) && (e2.id==0))
    {
        add_instr("SET_P __aux_var \n"); // save the imaginary part
        add_instr("SET_P __aux_var1\n"); // save the real part
        add_instr("SET   __aux_var2\n"); // save the float

        add_instr("LOD   __aux_var1\n"); // push the real part onto the stack
        add_instr("F_MLT __aux_var1\n");
        add_instr("P_LOD __aux_var \n"); // fetch the imaginary part
        add_instr("F_MLT __aux_var \n");
        add_instr("SF_ADD        \n");
        add_instr("SET   __aux_var3\n"); // save the squared magnitude

        add_instr("LOD   __aux_var2\n"); // load the float
        add_instr("F_MLT __aux_var1\n");
        add_instr("P_LOD __aux_var3\n"); // fetch the squared magnitude
        oper_div(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));      // divide

        add_instr("P_LOD __aux_var2\n"); // fetch the float
        add_instr("F_MLT __aux_var \n");
        add_instr("P_LOD __aux_var3\n"); // fetch the squared magnitude
        oper_div(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));      // divide
        oper_neg (expr_make(TY_FLOAT, 0));
    }

    // comp in memory with int var
    if ((place_of(e1) == PL_CM) && (e2.type==TY_INT) && (e2.id!=0))
    {
        get_cmp_parts(e1,&etr,&eti);

        oper_div(etr, e2);
        oper_div(eti, e2);
    }

    // comp in memory with int acc
    if ((place_of(e1) == PL_CM) && (e2.type==TY_INT) && (e2.id==0))
    {
        get_cmp_parts(e1,&etr,&eti);

        add_instr("I2F\n");
        add_instr("SET   __aux_var\n");
        oper_div(etr, expr_make(TY_FLOAT, 0));
        add_instr("P_LOD __aux_var\n");
        oper_div(eti, expr_make(TY_FLOAT, 0));
    }

    // comp in memory with float var
    if ((place_of(e1) == PL_CM) && (e2.type==TY_FLOAT) && (e2.id!=0))
    {
        get_cmp_parts(e1,&etr,&eti);

        oper_div(etr, e2);
        oper_div(eti, e2);
    }

    // comp in memory with float acc
    if ((place_of(e1) == PL_CM) && (e2.type==TY_FLOAT) && (e2.id==0))
    {
        get_cmp_parts(e1,&etr,&eti);

        add_instr("SET   __aux_var\n");
        oper_div(etr, expr_make(TY_FLOAT, 0));
        add_instr("P_LOD __aux_var\n");
        oper_div(eti, expr_make(TY_FLOAT, 0));
    }

    // comp in memory with comp in memory
    if ((place_of(e1) == PL_CM) && (place_of(e2) == PL_CM))
    {
        expr et1r, et2r;
        expr et1i, et2i;

        get_cmp_parts(e1,&et1r,&et1i);
        get_cmp_parts(e2,&et2r,&et2i);

        emit_sq_sum(et2r, et2i);
        add_instr("SET   __aux_var\n");
        acc_live = 0;

        oper_mult(et1r, et2r);
        oper_mult(et1i, et2i);
        oper_add(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));
        add_instr("P_LOD __aux_var\n");
        oper_div(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));

        oper_mult(et1i, et2r);
        oper_mult(et1r, et2i);
        oper_sub(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));
        add_instr("P_LOD __aux_var\n");
        oper_div(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));
    }


    // comp in memory with comp acc
    if ((place_of(e1) == PL_CM) && (e2.type==TY_COMP) && (e2.id==0))
    {
        get_cmp_parts(e1,&etr,&eti);

        add_instr("SET_P __aux_var \n");
        add_instr("SET   __aux_var1\n");
        add_instr("F_MLT __aux_var1\n");
        add_instr("P_LOD __aux_var \n");
        add_instr("F_MLT __aux_var \n");
        oper_add(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));
        add_instr("SET   __aux_var2\n");

        add_instr("LOD   __aux_var1\n");
        oper_mult(etr, expr_make(TY_FLOAT, 0));
        add_instr("P_LOD __aux_var \n");
        oper_mult(eti, expr_make(TY_FLOAT, 0));
        oper_add(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));
        add_instr("P_LOD __aux_var2\n");
        oper_div(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));

        add_instr("P_LOD __aux_var1\n");
        oper_mult(eti, expr_make(TY_FLOAT, 0));
        add_instr("P_LOD __aux_var \n");
        oper_mult(etr, expr_make(TY_FLOAT, 0));
        oper_sub(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));
        add_instr("P_LOD __aux_var2\n");
        oper_div(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));
    }








    // comp acc with int var
    if ((e1.type==TY_COMP) && (e1.id==0) && (e2.type==TY_INT) && (e2.id!=0))
    {
        add_instr("SET_P __aux_var \n");
        add_instr("P_I2F_M %s\n", v_table[e2.id].name);
        add_instr("SET   __aux_var1\n");
        oper_div(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));
        add_instr("P_LOD __aux_var \n");
        add_instr("P_LOD __aux_var1\n");
        oper_div(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));
    }

    // comp acc with int acc
    if ((e1.type==TY_COMP) && (e1.id==0) && (e2.type==TY_INT) && (e2.id==0))
    {
        add_instr("I2F\n");
        add_instr("SET_P __aux_var \n");
        add_instr("SET_P __aux_var1\n");
        add_instr("P_LOD __aux_var \n");
        oper_div(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));
        add_instr("P_LOD __aux_var1\n");
        add_instr("P_LOD __aux_var \n");
        oper_div(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));
    }

    // comp acc with float var
    if ((e1.type==TY_COMP) && (e1.id==0) && (e2.type==TY_FLOAT) && (e2.id!=0))
    {
        add_instr("SET_P __aux_var\n");
        add_instr("P_LOD %s\n", v_table[e2.id].name);
        oper_div(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));
        add_instr("P_LOD __aux_var\n");
        add_instr("P_LOD %s\n", v_table[e2.id].name);
        oper_div(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));
    }

    // comp acc with float acc
    if ((e1.type==TY_COMP) && (e1.id==0) && (e2.type==TY_FLOAT) && (e2.id==0))
    {
        add_instr("SET_P __aux_var \n");
        add_instr("SET_P __aux_var1\n");
        add_instr("P_LOD __aux_var \n");
        oper_div(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));
        add_instr("P_LOD __aux_var1\n");
        add_instr("P_LOD __aux_var \n");
        oper_div(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));
    }

    // comp acc with comp in memory
    if ((e1.type==TY_COMP) && (e1.id==0) && (place_of(e2) == PL_CM))
    {
        get_cmp_parts(e2,&etr,&eti);

        add_instr("SET_P __aux_var \n");
        add_instr("SET   __aux_var1\n");
        acc_live = 0;

        emit_sq_sum(etr, eti);
        add_instr("SET   __aux_var2\n");

        add_instr("LOD   __aux_var1\n");
        oper_mult(etr, expr_make(TY_FLOAT, 0));
        add_instr("P_LOD __aux_var \n");
        oper_mult(eti, expr_make(TY_FLOAT, 0));
        oper_add(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));
        add_instr("P_LOD __aux_var2\n");
        oper_div(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));

        add_instr("P_LOD __aux_var1\n");
        oper_mult(eti, expr_make(TY_FLOAT, 0));
        add_instr("P_LOD __aux_var \n");
        oper_mult(etr, expr_make(TY_FLOAT, 0));
        oper_sub(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));
        add_instr("P_LOD __aux_var2\n");
        oper_div(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));
        oper_neg (expr_make(TY_FLOAT, 0));
    }


    // comp acc with comp acc
    if ((e1.type==TY_COMP) && (e1.id==0) && (e2.type==TY_COMP) && (e2.id==0))
    {
        add_instr("SET_P __aux_var \n");
        add_instr("SET_P __aux_var1\n");
        add_instr("SET_P __aux_var2\n");
        add_instr("SET   __aux_var3\n");

        add_instr("LOD   __aux_var1\n");
        add_instr("F_MLT __aux_var1\n");
        add_instr("P_LOD __aux_var \n");
        add_instr("F_MLT __aux_var \n");
        add_instr("SF_ADD        \n");
        add_instr("SET   __aux_var4\n");

        add_instr("LOD   __aux_var3\n");
        add_instr("F_MLT __aux_var1\n");
        add_instr("P_LOD __aux_var2\n");
        add_instr("F_MLT __aux_var \n");
        add_instr("SF_ADD        \n");
        add_instr("P_LOD __aux_var4\n");
        oper_div(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));

        add_instr("P_LOD __aux_var3\n");
        add_instr("F_MLT __aux_var \n");
        add_instr("P_LOD __aux_var2\n");
        add_instr("F_MLT __aux_var1\n");
        oper_sub(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));
        add_instr("P_LOD __aux_var4\n");
        oper_div(expr_make(TY_FLOAT, 0), expr_make(TY_FLOAT, 0));
        oper_neg (expr_make(TY_FLOAT, 0));
    }

    acc_live = 1;

    return expr_make(result_type(e1, e2), 0);
}

// oper_mod: the left operand by row, the right one by column (IM IA FM FA CM CA)
static const char *const mod_tpl[PL_N][PL_N] = {
  /* IM */ {"$L $b\nMOD $a", "MOD $a", NULL, NULL, NULL, NULL},
  /* IA */ {"P_LOD $b\nS_MOD", "S_MOD", NULL, NULL, NULL, NULL},
  /* FM */ {NULL, NULL, NULL, NULL, NULL, NULL},
  /* FA */ {NULL, NULL, NULL, NULL, NULL, NULL},
  /* CM */ {NULL, NULL, NULL, NULL, NULL, NULL},
  /* CA */ {NULL, NULL, NULL, NULL, NULL, NULL},
};
// division remainder
expr oper_mod(expr e1, expr e2)
{

    if ((e1.type > TY_INT) || (e2.type > TY_INT))
        fatal(MSG_ERR_MOD_NON_INT, pctx.line_num+1);

    emit_tpl(mod_tpl[place_of(e1)][place_of(e2)], e1, e2);

    acc_live = 1;

    return expr_make(TY_INT, 0); // int in the accumulator
}

// ----------------------------------------------------------------------------
// comparison operations ------------------------------------------------------
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// comparisons with a comp ----------------------------------------------------
// ----------------------------------------------------------------------------
// As in C, two complex numbers are equal when their real parts are equal and
// their imaginary parts are equal; a real is the comp with a zero imaginary
// part. They have no order: <, >, <= and >= with a comp are an error
// (Luciano, 2026-10-30; it compared |z|^2, so (1+0i) == (0+1i) and
// -5 == 3+4i held). The result is (re1 == re2) AND (im1 == im2), EQU being
// one unit for int and float.

// one side of the comparison: in memory (the parts' operands) or in the acc
typedef struct {
    int  acc;         // 1: in the acc (a comp: real on the stack, imag in the acc)
    int  comp;        // 1: a comp, 0: a real
    int  is_int;      // a real int: its real part is converted (I2F)
    const char *re;   // in memory: the real part's operand (a v_table name)
    const char *im;   // in memory: the imaginary part's operand (a v_table name)
} cmp_side;

static const char *cmp_flit(const char *t)
{
    char *b = xstrdup(t);                    // lex_fnum may edit its text
    int id = lex_fnum(b);
    free(b);
    return v_table[id].name;
}

static cmp_side cmp_side_of(expr e)
{
    cmp_side d; memset(&d, 0, sizeof d);
    d.comp = (e.type >= TY_COMP);
    if (e.type == TY_CCONST) {                                  // a comp constant: two float literals
        expr er, ei; get_cmp_cst(e, &er, &ei);
        d.re = v_table[er.id].name;
        d.im = v_table[ei.id].name;
        return d;
    }
    if (e.id == 0) {d.acc = 1; d.is_int = (e.type == TY_INT); return d;}
    v_table[e.id].used = 1;
    if (e.type == TY_COMP) {
        d.re = v_table[e.id].name;
        d.im = sym(get_img_id(e.id))->name;
    } else if (e.type == TY_INT && v_table[e.id].is_const == 1) {  // an int literal: the float literal
        char *t = xasprintf("%s.0", v_table[e.id].name);
        d.re = cmp_flit(t);
        d.im = cmp_flit("0.0");
        free(t);
    } else {
        d.is_int = (e.type == TY_INT);
        d.re = v_table[e.id].name;
        d.im = cmp_flit("0.0");
    }
    return d;
}

static expr oper_cmp_comp(expr e1, expr e2, int type)
{
    if (type != 2) fatal(MSG_ERR_CMP_COMP_ORDER, pctx.line_num+1);

    cmp_side a = cmp_side_of(e1), b = cmp_side_of(e2);
    const char *z = cmp_flit("0.0");

    if (!a.acc && !b.acc)                               // both in memory: 5 words
    {
        if (b.is_int) {cmp_side t = a; a = b; b = t;}   // EQU is symmetric: the int one is loaded
        add_instr("%s %s\n", a.is_int ? (acc_live ? "P_I2F_M" : "I2F_M") : (acc_live ? "P_LOD" : "LOD"), a.re);
        add_instr("EQU %s\n", b.re);
        add_instr("P_LOD %s\n", a.im);
        add_instr("EQU %s\n", b.im);
        add_instr("S_AND\n");
    }
    else if (a.acc != b.acc)                            // one in the acc (EQU is symmetric)
    {
        cmp_side x = a.acc ? a : b, m = a.acc ? b : a;
        if (x.comp) {                                   // real on the stack, imag in the acc
            add_instr("EQU %s\n", m.im);
            add_instr("SET_P __aux_var\n");
            if (m.is_int) {add_instr("P_I2F_M %s\n", m.re); add_instr("S_EQU\n");}
            else           add_instr("EQU %s\n", m.re);
            add_instr("AND __aux_var\n");
        } else {                                        // a real in the acc, a comp in memory
            if (x.is_int) add_instr("I2F\n");
            add_instr("EQU %s\n", m.re);
            add_instr("P_LOD %s\n", m.im);
            add_instr("EQU %s\n", z);
            add_instr("S_AND\n");
        }
    }
    else if (a.comp && b.comp)                          // stack: re1, im1, re2; acc: im2
    {
        add_instr("SET_P __aux_var\n");
        add_instr("SET_P __aux_var1\n");
        add_instr("EQU __aux_var\n");
        add_instr("SET_P __aux_var2\n");
        add_instr("EQU __aux_var1\n");
        add_instr("AND __aux_var2\n");
    }
    else if (a.comp)                                    // stack: re1, im1; acc: the real
    {
        if (b.is_int) add_instr("I2F\n");
        add_instr("SET_P __aux_var\n");
        add_instr("EQU %s\n", z);
        add_instr("SET_P __aux_var1\n");
        add_instr("EQU __aux_var\n");
        add_instr("AND __aux_var1\n");
    }
    else                                                // stack: the real, re2; acc: im2
    {
        add_instr("EQU %s\n", z);
        add_instr("SET_P __aux_var\n");
        if (a.is_int) {
            add_instr("SET_P __aux_var1\n");
            add_instr("I2F\n");
            add_instr("EQU __aux_var1\n");
        } else add_instr("S_EQU\n");
        add_instr("AND __aux_var\n");
    }

    acc_live = 1;
    return expr_make(TY_INT, 0); // int boolean in the accumulator
}

// oper_cmp: the left operand by row, the right one by column (IM IA FM FA CM CA)
static const char *const cmp_tpl[PL_N][PL_N] = {
  /* IM */ {NULL, "$1 $a", "$C $a\n$4 $b", NULL, NULL, NULL},
  /* IA */ {"$2 $b", "S_$1", "I2F\n$4 $b", "SET_P __aux_var\nI2F\n$4 __aux_var", NULL, NULL},
  /* FM */ {"$C $b\n$3 $a", "I2F\n$3 $a", NULL, "$3 $a", NULL, NULL},
  /* FA */ {NULL, NULL, "$4 $b", NULL, NULL, NULL},
  /* CM */ {NULL, NULL, NULL, NULL, NULL, NULL},
  /* CA */ {NULL, NULL, NULL, NULL, NULL, NULL},
};
// compares greater-than, less-than and equal-to
// - separar o igual pra economizar clock
// - review to avoid unnecessary stack use!
expr oper_cmp(expr e1, expr e2, int type)
{

    if (e1.type >= TY_COMP || e2.type >= TY_COMP) return oper_cmp_comp(e1, e2, type);   // a comp: its own rule

    int_lit_float(&e1, &e2);

    char ld [10]; if (acc_live == 0) strcpy(ld , "LOD" ); else strcpy(ld ,"P_LOD"  );

    char op[16] = "";  // initialize so an unhandled 'type' produces a deterministic empty mnemonic instead of stack garbage
    switch (type)
    {
        case 0: strcpy(op, "LES"); break;
        case 1: strcpy(op, "GRE"); break;
        case 2: strcpy(op, "EQU"); break;
        default:
            fatal(MSG_ERR_INTERNAL, "oper_cmp", pctx.line_num+1);
    }

    // The memory forms compute `X op acc` (ula in1 = memory, in2 = acc), so an
    // operand already in the acc is compared against one in memory in a single
    // instruction: e1 in acc, e2 in memory is `rev e2` (a < b == b > a); e2 in
    // acc, e1 in memory is `op e1`. EQU is one unit for int and float.
    char rev [16]; strcpy(rev, type == 0 ? "GRE" : type == 1 ? "LES" : "EQU");
    char fop [20]; if (type == 2) strcpy(fop , "EQU"); else snprintf(fop , sizeof fop , "F_%s", op );
    char frev[20]; if (type == 2) strcpy(frev, "EQU"); else snprintf(frev, sizeof frev, "F_%s", rev);

    // int var with int var: the variable is loaded and the literal stays in
    // memory, so a value the acc already holds is not loaded again
    if ((e1.type==TY_INT) && (e1.id!=0) && (e2.type==TY_INT) && (e2.id!=0))
    {
        if (v_table[e2.id].is_const == 1 && v_table[e1.id].is_const != 1)
        {
            add_instr("%s %s\n", ld , v_table[e1.id].name);
            add_instr("%s %s\n", rev, v_table[e2.id].name);
        }
        else
        {
            add_instr("%s %s\n", ld, v_table[e2.id].name);
            add_instr("%s %s\n", op, v_table[e1.id].name);
        }
    }

    emit_tpl_ops(cmp_tpl[place_of(e1)][place_of(e2)], e1, e2, (const char *const[]){op, rev, fop, frev});

    // int var with float acc: the variable is converted over the float, which
    // the stack then holds (an int literal is a float literal by now)
    if ((e1.type==TY_INT) && (e1.id!=0) && (e2.type==TY_FLOAT) && (e2.id==0))
    {
        add_instr("P_I2F_M %s\n", v_table[e1.id].name);
        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", rev);
    }

    // float var with float var: the variable is loaded, the literal stays in memory
    if ((e1.type==TY_FLOAT) && (e1.id!=0) && (e2.type==TY_FLOAT) && (e2.id!=0))
    {
        if (v_table[e1.id].is_const == 1 && v_table[e2.id].is_const != 1)
        {
            add_instr("%s %s\n", ld , v_table[e2.id].name);
            add_instr("%s %s\n", fop, v_table[e1.id].name);
        }
        else
        {
            add_instr("%s %s\n", ld  , v_table[e1.id].name);
            add_instr("%s %s\n", frev, v_table[e2.id].name);
        }
    }

    // float acc with int var (an int literal is a float literal by now)
    if ((e1.type==TY_FLOAT) && (e1.id==0) && (e2.type==TY_INT) && (e2.id!=0))
    {
        add_instr("P_I2F_M %s\n", v_table[e2.id].name);
        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
    }

    // float acc with int acc
    if ((e1.type==TY_FLOAT) && (e1.id==0) && (e2.type==TY_INT) && (e2.id==0))
    {
        add_instr("I2F\n");
        
        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
    }

    // float acc with float acc
    if ((e1.type==TY_FLOAT) && (e1.id==0) && (e2.type==TY_FLOAT) && (e2.id==0))
    {
        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
    }

    acc_live = 1;

    return expr_make(TY_INT, 0); // int boolean in the accumulator
}

// compares greater-or-equal (opposite of less-than)
expr oper_ge(expr e1, expr e2)
{
    return oper_lin(oper_cmp(e1, e2, 0));
}

// compares less-or-equal (opposite of greater-than)
expr oper_le(expr e1, expr e2)
{
    return oper_lin(oper_cmp(e1, e2, 1));
}

// compares not-equal (opposite of equal-to)
expr oper_ne(expr e1, expr e2)
{
    return oper_lin(oper_cmp(e1, e2, 2));
}

// ----------------------------------------------------------------------------
// conditional operations (used in if/else/while) -----------------------------
// ----------------------------------------------------------------------------

// logical inversion (!)
expr oper_lin(expr e)
{
    expr etr, eti;

    char ld [10]; if (acc_live == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );
    char lin[10]; if (acc_live == 0) strcpy(lin,"LIN_M"); else strcpy(lin,"P_LIN_M");

    // when it is an int in memory
    if ((e.type == TY_INT) && (e.id != 0))
    {
        add_instr("%s %s\n", lin, v_table[e.id].name);
    }

    // when it is an int in the acc
    if ((e.type == TY_INT) && (e.id == 0))
    {
        add_instr("LIN\n");
    }

    // when it is a float variable in memory
    if ((e.type == TY_FLOAT) && (e.id != 0))
    {
        // a float is zero exactly when its mantissa is (see emit_cond_int_load)
        add_instr("%s %s\n", ld, v_table[e.id].name);
        add_instr("AND %lld\n", mant_pow2() - 1);
        add_instr("LIN\n");
    }

    // when it is a float in the acc
    if ((e.type == TY_FLOAT) && (e.id == 0))
    {
        add_instr("AND %lld\n", mant_pow2() - 1);
        add_instr("LIN\n");
    }

    // when it is a comp const
    if (e.type == TY_CCONST)
    {
        // a comp is true when its real or its imaginary part is not zero
        // (C99): (re OR im) AND mask, then LIN
        get_cmp_cst(e,&etr,&eti);

        add_instr("%s %s\n", ld, v_table[etr.id].name);
        add_instr("ORR %s\n", v_table[eti.id].name);
        add_instr("AND %lld\n", mant_pow2() - 1);
        add_instr("LIN\n");
    }

    // when it is a comp variable
    if ((e.type == TY_COMP) && (e.id != 0))
    {
        add_instr("%s %s\n", ld, v_table[e.id].name);
        add_instr("ORR %s\n", sym(get_img_id(e.id))->name);
        add_instr("AND %lld\n", mant_pow2() - 1);
        add_instr("LIN\n");
    }

    // when it is a comp in the acc (the imag part in the acc, the real on the stack)
    if ((e.type == TY_COMP) && (e.id == 0))
    {
        add_instr("S_ORR\n");
        add_instr("AND %lld\n", mant_pow2() - 1);
        add_instr("LIN\n");
    }

    acc_live = 1;

    return expr_make(TY_INT, 0); // int in the accumulator
}

// logical and/or (&& ||)
expr oper_logic(expr e1, expr e2, int type)
{

    if ((e1.type > TY_FLOAT) || (e2.type > TY_FLOAT))
    {
        fatal(MSG_ERR_LOGIC_NON_INT, pctx.line_num+1);
    }

    char ld[10];
    if (acc_live == 0) strcpy(ld,"LOD"); else strcpy(ld,"P_LOD");

    char op[16];
    switch (type)
    {
        case 0: strcpy(op, "LAN"); break;
        case 1: strcpy(op, "LOR" ); break;
        default: fatal(MSG_ERR_INTERNAL, "oper_logic", pctx.line_num+1);
    }

    // a float operand counts by its truth, the word AND 2^NBMANT - 1
    // (zero exactly when the float is); LAN and LOR commute, so the order of
    // the two operands does not matter
    if ((e1.type == TY_FLOAT) || (e2.type == TY_FLOAT))
    {
        long long mask = mant_pow2() - 1;
        if ((e1.id != 0) && (e2.id != 0))                     // var with var
        {
            add_instr("%s %s\n", ld, v_table[e2.id].name);
            if (e2.type == TY_FLOAT) add_instr("AND %lld\n", mask);
            if (e1.type == TY_FLOAT) { add_instr("P_LOD %s\n", v_table[e1.id].name); add_instr("AND %lld\n", mask); add_instr("S_%s\n", op); }
            else              add_instr("%s %s\n", op, v_table[e1.id].name);
        }
        else if ((e1.id != 0) || (e2.id != 0))                // var with acc
        {
            expr ea = (e1.id == 0) ? e1 : e2;                  // the one in the acc
            expr ev = (e1.id == 0) ? e2 : e1;                  // the one in memory
            if (ea.type == TY_FLOAT) add_instr("AND %lld\n", mask);
            if (ev.type == TY_FLOAT) { add_instr("P_LOD %s\n", v_table[ev.id].name); add_instr("AND %lld\n", mask); add_instr("S_%s\n", op); }
            else              add_instr("%s %s\n", op, v_table[ev.id].name);
        }
        else                                                  // acc with acc: e2 in the acc, e1 on the stack
        {
            if (e2.type == TY_FLOAT) add_instr("AND %lld\n", mask);
            if (e1.type == TY_FLOAT) { add_instr("SET_P __aux_var\n"); add_instr("AND %lld\n", mask); add_instr("%s __aux_var\n", op); }
            else              add_instr("S_%s\n", op);
        }
        acc_live = 1;
        return expr_make(TY_INT, 0);
    }

    // int var with int var
    if ((e1.type==TY_INT) && (e1.id!=0) && (e2.type==TY_INT) && (e2.id!=0))
    {
        add_instr("%s %s\n", ld, v_table[e2.id].name);
        add_instr("%s %s\n", op, v_table[e1.id].name);
    }

    // int var with int acc
    if ((e1.type==TY_INT) && (e1.id!=0) && (e2.type==TY_INT) && (e2.id==0))
    {
        add_instr("%s %s\n", op, v_table[e1.id].name);
    }

    // int acc with int var
    if ((e1.type==TY_INT) && (e1.id==0) && (e2.type==TY_INT) && (e2.id!=0))
    {
        add_instr("%s %s\n", op, v_table[e2.id].name);
    }

    // int acc with int acc
    if ((e1.type==TY_INT) && (e1.id==0) && (e2.type==TY_INT) && (e2.id==0))
    {
        add_instr("S_%s\n", op);
    }

    acc_live = 1;

    return expr_make(TY_INT, 0); // int boolean in the accumulator
}

// ----------------------------------------------------------------------------
// bitwise logical-gate operations (~ & |) ------------------------------------
// ----------------------------------------------------------------------------

// inverter gate
expr oper_inv(expr e)
{

    if (e.type > TY_INT)
        fatal(MSG_ERR_INV_NON_INT, pctx.line_num+1);

    char inv[10]; if (acc_live == 0) strcpy(inv,"INV_M"); else strcpy(inv,"P_INV_M");

    // when it is an int in memory
    if ((e.type == TY_INT) && (e.id != 0))
    {
        add_instr("%s %s\n", inv, v_table[e.id].name);
    }

    // when it is an int in the acc
    if ((e.type == TY_INT) && (e.id == 0))
    {
        add_instr("INV\n");
    }

    acc_live = 1;

    return expr_make(TY_INT, 0); // int in the accumulator
}

// two-input logical gates (& | ^)
expr oper_bitw(expr e1, expr e2, int type)
{

    if (e1.type > TY_FLOAT || e2.type > TY_FLOAT)
    {
        fatal(MSG_ERR_BITWISE_COMPLEX, pctx.line_num+1);
    }
    
    char op[16];

    switch (type)
    {
        case  0: strcpy(op, "AND"); break;
        case  1: strcpy(op, "ORR"); break;
        case  2: strcpy(op, "XOR"); break;
        default: fatal(MSG_ERR_INTERNAL, "oper_bitw", pctx.line_num+1);
    }

    char ld[10]; if (acc_live == 0) strcpy(ld,"LOD"); else strcpy(ld,"P_LOD");

    // var with var
    if ((e1.id != 0) && (e2.id != 0))
    {
        add_instr("%s %s\n", ld, v_table[e2.id].name);
        add_instr("%s %s\n", op, v_table[e1.id].name);
    }

    // var with acc
    if ((e1.id != 0) && (e2.id == 0))
    {
        add_instr("%s %s\n", op, v_table[e1.id].name);
    }

    // acc with var: the three are commutative, so the var is the memory operand
    if ((e1.id == 0) && (e2.id != 0))
    {
        add_instr("%s %s\n", op, v_table[e2.id].name);
    }

    // acc with acc
    if ((e1.id == 0) && (e2.id == 0))
    {
        add_instr("S_%s\n", op);
    }

    acc_live = 1;

    return expr_make(TY_INT, 0); // int in the accumulator
}

// ----------------------------------------------------------------------------
// bit-shift operations -------------------------------------------------------
// ----------------------------------------------------------------------------

expr oper_shift(expr e1, expr e2, int type)
{

    if (e1.type > TY_FLOAT)
        fatal(MSG_ERR_SHIFT_COMPLEX, pctx.line_num+1);

    if (e2.type > TY_FLOAT)
        fatal(MSG_ERR_SHIFT_BY_COMP, pctx.line_num+1);

    char op[16];

    switch (type)
    {
        case  0: strcpy(op, "SHL"); break;
        case  1: strcpy(op, "SHR"); break;
        case  2: strcpy(op, "SRS"); break;
        default: fatal(MSG_ERR_INTERNAL, "oper_shift", pctx.line_num+1);
    }

    char ld[10]; if (acc_live == 0) strcpy(ld,"LOD"); else strcpy(ld,"P_LOD");

    // int/float var with int var
    if ((e1.id != 0) && (e2.type == TY_INT) && (e2.id != 0))
    {
        add_instr("%s %s\n", ld, v_table[e2.id].name);
        add_instr("%s %s\n", op, v_table[e1.id].name);
    }

    // int/float var with int acc
    if ((e1.id != 0) && (e2.type == TY_INT) && (e2.id == 0))
    {
        add_instr("%s %s\n", op, v_table[e1.id].name);
    }

    // int/float var with float var
    if ((e1.id != 0) && (e2.type == TY_FLOAT) && (e2.id != 0))
    {
        warn(MSG_WARN_SHIFT_BY_FLOAT, pctx.line_num+1);

        add_instr("%s %s\n", acc_live ? "P_F2I_M" : "F2I_M", v_table[e2.id].name);
        add_instr("%s %s\n", op, v_table[e1.id].name);
    }

    // int/float var with float acc
    if ((e1.id != 0) && (e2.type == TY_FLOAT) && (e2.id == 0))
    {
        warn(MSG_WARN_SHIFT_BY_FLOAT, pctx.line_num+1);

        add_instr("F2I\n");
        add_instr("%s %s\n", op, v_table[e1.id].name);
    }

    // int/float acc with int var
    if ((e1.id == 0) && (e2.type == TY_INT) && (e2.id != 0))
    {
        add_instr("P_LOD %s\n", v_table[e2.id].name);
        add_instr("S_%s\n", op);
    }

    // int/float acc with int acc
    if ((e1.id == 0) && (e2.type == TY_INT) && (e2.id == 0))
    {
        add_instr("S_%s\n", op);
    }

    // int/float acc with float var
    if ((e1.id == 0) && (e2.type == TY_FLOAT) && (e2.id != 0))
    {
        warn(MSG_WARN_SHIFT_BY_FLOAT, pctx.line_num+1);

        add_instr("P_F2I_M %s\n", v_table[e2.id].name);
        add_instr("S_%s\n", op);
    }

    // int/float acc with float acc
    if ((e1.id == 0) && (e2.type == TY_FLOAT) && (e2.id == 0))
    {
        warn(MSG_WARN_SHIFT_BY_FLOAT, pctx.line_num+1);
        
        add_instr("F2I\n");
        add_instr("S_%s\n", op);
    }

    acc_live = 1;

    return expr_make(TY_INT, 0); // int in the accumulator
}