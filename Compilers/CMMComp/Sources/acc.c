// ----------------------------------------------------------------------------
// the accumulator: what it holds, in one place (step P9 of #93) ---------------
// ----------------------------------------------------------------------------
// Two things are known of the acc while cmmcomp emits:
// - acc_live: it holds a value an expression still needs, so the next operand
//   load pushes it (P_LOD) instead of overwriting it (LOD);
// - which values it holds: names of variables and constants, and keys of
//   values that are no variable (an element "v[k]", an index "[k]", a sum of
//   the Dirac loops' variables "[dk0+dd1]"). add_instr drops a LOD of a value
//   the acc holds, and the emitters ask acc_holds to reuse it.
// What it holds is learnt from each instruction emitted (acc_after) and from
// the emitters that know more than an instruction says (acc_now, acc_holds_elem).
// ----------------------------------------------------------------------------

// global includes
#include <stdlib.h>
#include <string.h>

// local includes
#include "../Headers/acc.h"
#include "../../common/xstr.h"

int acc_live = 0;   // 0: the acc holds nothing an expression needs (LOD), 1: it does (P_LOD)

// One-instruction window for the SET-then-LOD peephole. Holds the most
// recent string passed to add_instr so the next call can detect a redundant
// load. Cleared by every control-flow boundary (labels via add_sinst, capture
// push/pop, macro start/end) so the drop only fires within a basic block.
static char *last_str = NULL;           // NULL: none

// Name of the variable/constant currently sitting in the accumulator, or "" when
// unknown. A plain LOD/P_LOD/SET of an operand leaves that operand's value in the
// acc; every other op clobbers it. add_instr drops a "LOD x" whenever x is among
// acc_names (the acc holds it), which generalises the old "LOD x after SET x"
// peephole. Cleared at every basic-block boundary alongside last_str. A SET
// adds its name to the ones the acc already holds: after LOD 0.0; SET a; SET b;
// SET c the acc is 0.0, a, b and c, and a fourth "LOD 0.0" is dropped too.
#define ACC_NAMES 32
static char *acc_names[ACC_NAMES];       // each in a block of its own
static int   acc_n = 0;                  // how many of them the acc holds

static void acc_clear(void)
{
    for (int i = 0; i < acc_n; i++) free(acc_names[i]);
    acc_n = 0;
}

static void acc_only(const char *name)   // the acc holds this one name ("": none)
{
    acc_clear();
    if (name && name[0]) acc_names[acc_n++] = xstrdup(name);
}

// a store leaves the acc as it is, but an element or an index key ("v[k]",
// "[ii+j]") goes, since the store may write a word the key reads
static void acc_drop_keys(void)
{
    int k = 0;
    for (int i = 0; i < acc_n; i++)
        if (!strchr(acc_names[i], '[')) acc_names[k++] = acc_names[i];
        else free(acc_names[i]);
    acc_n = k;
}

// a SET of `name`: the acc keeps what it held, and is `name` too
static void acc_add(const char *name)
{
    acc_drop_keys();
    int k = 0;
    for (int i = 0; i < acc_n; i++)
        if (strcmp(acc_names[i], name) != 0) acc_names[k++] = acc_names[i];
        else free(acc_names[i]);
    acc_n = k;
    if (acc_n == ACC_NAMES)              // full: the oldest goes
    {
        free(acc_names[0]);
        memmove(acc_names, acc_names + 1, (size_t)(acc_n - 1) * sizeof acc_names[0]);
        acc_n--;
    }
    acc_names[acc_n++] = xstrdup(name);
}

void emit_peephole_reset(void)
{
    free(last_str); last_str = NULL;
    acc_clear();
}

// True when the accumulator is known to currently hold variable `name`. Used by
// the codegen (e.g. the comp c²+d² in oper_div / builtin_mod2) to square the
// acc-resident half first, so its LOD is dropped by the peephole above.
int acc_holds(const char *name)
{
    if (!name || !name[0]) return 0;
    for (int i = 0; i < acc_n; i++) if (strcmp(acc_names[i], name) == 0) return 1;
    return 0;
}

// An indexed store (STI, ISI) or read (LDI) leaves the element in the acc: the
// walker names it by the element, "v[k]", which no variable name can be, so
// that a read of the same element after it takes the acc (ast.c). Only when
// the store or read is the last instruction emitted; an instruction after it
// that keeps the acc (OUT, PSH, TOM, JIZ) keeps the name, any other forgets
// it, and a SET does too, since it may write an index
void acc_holds_elem(const char *key)
{
    if (last_str && (strncmp(last_str, "STI ", 4) == 0 || strncmp(last_str, "ISI ", 4) == 0
                  || strncmp(last_str, "LDI ", 4) == 0))   // a read leaves the element too
        acc_only(key);
}

// What the acc holds now, named by the caller (global.h): the index of an
// indexed store, computed into the acc ("[ii+j]", ast.c), so that a read at
// the same index inside the right-hand side pushes it with PSH instead of
// computing it again; the Dirac loops' sums and counters (dirac.c), which they
// know across a label (the counter at the top of a loop)
void acc_now(const char *key)
{
    acc_only(key);
}

// the first word of s, after any blanks (NULL: none); the block is the caller's
static char *word_of(const char *s)
{
    while (*s == ' ' || *s == '\t') s++;
    size_t n = strcspn(s, " \t\r\n");
    return n ? xasprintf("%.*s", (int)n, s) : NULL;
}

// a LOD of a value the acc holds already: add_instr drops it. Only the plain
// LOD: P_LOD also pushes, so it always runs. What the acc holds is forgotten
// at every basic-block boundary (emit_peephole_reset), so the drop never
// crosses a label or a control-flow merge
int acc_has_load(const char *str)
{
    if (strncmp(str, "LOD ", 4) != 0) return 0;
    char *v = word_of(str + 4);
    int held = v && acc_holds(v);
    free(v);
    return held;
}

// after an instruction was emitted: what the acc holds now. str is a block
// of the caller's that this module keeps (last_str) until the next one
void acc_after(char *str)
{
    // track what is now in the accumulator: a plain LOD / P_LOD / SET of an
    // operand leaves that operand's value there, OUT, PSH, TOM, JIZ (only
    // read it: isa.tsv; a JIZ's target has a label, which clears all this)
    // and SET_V leave the acc as it is, anything else clobbers it. A SET keeps
    // the names before it too: after LOD y; SET y_n1 the acc is both
    char *nm = strncmp(str, "SET ",   4) == 0 ? word_of(str + 4)
             : strncmp(str, "LOD ",   4) == 0 ? word_of(str + 4)
             : strncmp(str, "P_LOD ", 6) == 0 ? word_of(str + 6) : NULL;
    if      (nm && strncmp(str, "SET ", 4) == 0)
        acc_add(nm);
    else if (nm)
        acc_only(nm);
    else if (strncmp(str, "OUT ", 4) == 0 || strcmp(str, "PSH\n") == 0
          || strncmp(str, "TOM ", 4) == 0 || strncmp(str, "JIZ ", 4) == 0)
        ;                                // they read the acc and leave it as it is
    else if (strncmp(str, "SET_V ", 6) == 0)
        acc_drop_keys();                 // an element store: as a SET, the keys go
    else
        acc_clear();
    free(nm);

    // remember this emit for the next peephole check
    free(last_str);
    last_str = str;
}