%{

#include "y.tab.h"

#include "../Headers/global.h"
#include "../Headers/symbols.h"
#include "../Headers/data_declar.h"
#include "../Headers/messages.h"
#include "../Headers/diag.h"
#include "../../common/xstr.h"

#include <stdlib.h>
#include <string.h>

int  conta_linhas(char *texto); // for block comments

/* ---- object-like #define -------------------------------------------------
 * Handled entirely in the lexer: a `#define NAME body` line records the body,
 * and a later use of NAME is replaced by re-lexing the body from a pushed
 * buffer (yy_scan_string + yypush_buffer_state), popped at <<EOF>>. The
 * per-macro `active` flag (set while a macro's body buffer is live) stops a
 * self-referential define from expanding forever. Object-like only --
 * function-like macros, #ifdef and #include are out of scope. */
static struct def_entry { char *name; char *body; int active; } *g_def = NULL;
static int  g_ndef = 0, g_def_cap = 0;      /* as many defines as the program has (#108 A11) */
static int *g_def_stack = NULL;             /* indices of the live body buffers: each define at most once */
static int  g_def_sp = 0;
static char *g_def_pending = NULL;          /* name captured before its body */

static int def_find(const char *name) {
    for (int i = 0; i < g_ndef; i++)
        if (strcmp(g_def[i].name, name) == 0) return i;
    return -1;
}
static void def_add(const char *name, char *body) {
    while (*body == ' ' || *body == '\t') body++;            /* trim leading WS  */
    size_t n = strlen(body);
    while (n && (body[n-1]==' '||body[n-1]=='\t'||body[n-1]=='\r')) body[--n] = 0; /* trailing WS/CR */
    int i = def_find(name);
    if (i < 0) {
        if (g_ndef == g_def_cap) {
            g_def_cap = g_def_cap ? 2 * g_def_cap : 64;
            g_def       = xrealloc(g_def,       (size_t)g_def_cap * sizeof *g_def);
            g_def_stack = xrealloc(g_def_stack, (size_t)g_def_cap * sizeof *g_def_stack);
        }
        i = g_ndef++;
        g_def[i].name = xstrdup(name);
        g_def[i].body = NULL;
    }
    free(g_def[i].body);
    g_def[i].body = xstrdup(body);
    g_def[i].active = 0;
}

/* flex's own error (an input it cannot read) is cmmcomp's: in both languages,
 * and the outputs of the run removed (#108 A13) */
#define YY_FATAL_ERROR(msg) ((void)yy_fatal_error, fatal(MSG_ERR_SCANNER, msg))   /* flex's own stays referenced: -Werror */

%}

%option nounput noinput nodefault

%x DEFNAME DEFBODY

LETRA  [A-Za-z_]
WS     [ \t]+

VARIA  {LETRA}({LETRA}|[0-9])*
INNUM  ([0-9]+|0[xX][0-9a-fA-F]+|0[bB][01]+)[uUlL]*
FEXPO  [eE][-+]?[0-9]+
FLNUM  ([0-9]+\.[0-9]*{FEXPO}?|\.[0-9]+{FEXPO}?|[0-9]+{FEXPO})[fFlL]?
CPART  ({FLNUM}|[0-9]+)
IMNUM  {CPART}{WS}*i

STRIN  \"(\\.|[^\\"])*\"
CARES  [-+*()=#;:,\]\[{}<>~!%/&^|⟩]
BLOCO  "/*"((\*+[^/*])|([^*]))*\**"*/"
COMEN  "//"[^\n]*

%%

"#PRNAME"     return PRNAME;
"#NUBITS"     return NUBITS;
"#NBMANT"     return NBMANT;
"#NBEXPO"     return NBEXPO;
"#NDSTAC"     return NDSTAC;
"#SDEPTH"     return SDEPTH;
"#NUIOIN"     return NUIOIN;
"#NUIOOU"     return NUIOOU;
"#NUGAIN"     return NUGAIN;
"#FFTSIZ"     return FFTSIZ;
"#FROUND"     return FROUND;

"#PRACA"      return ITRADD;
"#TOAQUI"     return TOAQUI;
"#WAVE"       return WAVE;

 /* object-like #define NAME body -- recorded here, expanded at the {VARIA} rule */
"#define"               BEGIN(DEFNAME);
<DEFNAME>{WS}           ;
<DEFNAME>{VARIA}        { if (yyleng > CMM_NAME_MAX) fatal(MSG_ERR_NAME_TOO_LONG, pctx.line_num+1, CMM_NAME_MAX);
                          free(g_def_pending); g_def_pending = xstrdup(yytext); BEGIN(DEFBODY); }
<DEFNAME>\n             { pctx.line_num++; BEGIN(INITIAL); }
<DEFNAME>.              BEGIN(INITIAL);
<DEFBODY>[^\n]+         { def_add(g_def_pending, yytext); BEGIN(INITIAL); }
 /* no body: the define is empty, and the newline still ends the line (#108 A7) */
<DEFBODY>\n             { char none[] = ""; def_add(g_def_pending, none); pctx.line_num++; BEGIN(INITIAL); }

"in"          return INN;
"fin"         return FIN;
"toma"        return TOMA;
"cade"        return CADE;
"fcade"       return FCADE;
"toma_sync"   return TOMAS;
"cade_sync"   return CADES;
"fcade_sync"  return FCADES;
"out"         return OUT;
"fout"        return FOUT;

"norm"        return NRM;
"pset"        return PST;
"abs"         return ABS;
"sign"        return SGN;
"copy"        return COPY;

"sqrt"        return SQRT;
"atan"        return ATAN;
"sin"         return SIN;
"cos"         return COS;
"tan"         return TAN;
"cosh"        return COSH;
"sinh"        return SINH;
"tanh"        return TANH;
"exp"         return EXP;
"log"         return LOG;
"pow"         return POW;
"floor"       return FLOOR;
"ceil"        return CEIL;
"round"       return ROUND;

"real"        return REAL;
"imag"        return IMAG;
"conj"        return CONJ;
"fase"        return FASE;
"complex"     return COMP;
"mod2"        return MOD2;

"while"       return WHILE;
"do"          return DO;
"for"         return FOR;
"break"       return BREAK;
"continue"    return CONTINUE;
"if"          return IF;
"else"        return ELSE;
"switch"      return SWITCH;
"case"        return CASE;
"default"     return DEFAULT;
"return"      return RET;

"<<"          return  SHIFTL;
">>"          return  SHIFTR;
">>>"         return SSHIFTR;

">="          return GREQU;
"<="          return LESEQ;
"=="          return EQU;
"!="          return DIF;

"&&"          return LAN;
"||"          return LOR;

"++"          return PPLUS;

"⟩"           return BRA;
"⟨"           return KET;
"|I|"         return EYE;
"|0⟩"         return VZERO;

 /* |in(p)⟩, an input ket, as one token: x | in(0) is a valid bitwise or */
"|"[ \t]*"in"[ \t]*"("[ \t]*{INNUM}[ \t]*")"[ \t]*"⟩" {
                const char *p = strchr(yytext, '(') + 1;
                while (*p == ' ' || *p == '\t') p++;
                char *num = xasprintf("%.*s", (int)strcspn(p, " \t)"), p);   /* any length: a cut port was another port (#108 A4) */
                yylval.ival = lex_inum(num);
                free(num);
                return KETIN;
             }

 /* A bar followed by the name of an array opens or continues a Dirac chain
  * (DBAR); any other bar is the bitwise or, or the bar that closes a matrix
  * or a bra. An array name without an index is not an operand of an or, so
  * no program changes meaning; with an index (c | a[k]) it is, and the
  * longer rule below keeps that bar an or. The name goes back to the input
  * (yyless) and is lexed as usual. A #define name is not expanded here: it
  * counts as no array. */
"|"[ \t\r\n]*{VARIA}[ \t\r\n]*"[" {
                yyless(1);
                return '|';
             }
"|"[ \t\r\n]*{VARIA} {
                const char *name = yytext + 1 + strspn(yytext + 1, " \t\r\n");
                int k = (def_find(name) >= 0) ? -1 : find_id(name);
                yyless(1);
                return (k >= 0 && v_table[k].dims) ? DBAR : '|';
             }
 /* Two bars before the name of an array are a matrix that closes and a chain
  * that opens (|A||B|), not the logical or: the first bar goes out alone and
  * the second is lexed by the rule above. Before anything else, an indexed
  * array included (x || a[k]), they stay the logical or. */
"||"[ \t\r\n]*{VARIA}[ \t\r\n]*"[" {
                yyless(2);
                return LOR;
             }
"||"[ \t\r\n]*{VARIA} {
                const char *name = yytext + 2 + strspn(yytext + 2, " \t\r\n");
                int k = (def_find(name) >= 0) ? -1 : find_id(name);
                if (k >= 0 && v_table[k].dims) {yyless(1); return '|';}
                yyless(2);
                return LOR;
             }

"comp"       {pctx.type_tmp = 3; yylval.ival = 3; return TYPE;}
"float"      {pctx.type_tmp = 2; yylval.ival = 2; return TYPE;}
"int"        {pctx.type_tmp = 1; yylval.ival = 1; return TYPE;}
"void"       {              yylval.ival = 0; return TYPE;}
			 
{VARIA}      {
                int di = def_find(yytext);
                if (di >= 0 && !g_def[di].active) {
                    // expand: re-lex the macro body from a pushed buffer; the
                    // `active` flag blocks self-reference (no token returned here).
                    // yy_scan_string() switches the current buffer to the new one,
                    // so switch back before yypush_buffer_state() -- otherwise the
                    // outer buffer is never saved and the EOF pop frees it twice.
                    g_def[di].active = 1;
                    g_def_stack[g_def_sp++] = di;
                    YY_BUFFER_STATE cur = YY_CURRENT_BUFFER;
                    YY_BUFFER_STATE body = yy_scan_string(g_def[di].body);
                    yy_switch_to_buffer(cur);
                    yypush_buffer_state(body);
                } else {
                    // `__` starts the compiler's own names in the .asm
                    if (yytext[0] == '_' && yytext[1] == '_')
                        {fatal(MSG_ERR_NAME_UNDERSCORES, pctx.line_num+1, yytext);}
                    yylval.ival = lex_id(yytext);
                    return ID;
                }
             }
			 
{INNUM}      {
                int k = inum_kind(yytext);
                yylval.ival = (k == 1 || k == 2) ? lex_inum_neg(yytext) : k == 3 ? inum_min_id() : lex_inum(yytext);
                return k == 0 ? INUM : k == 1 ? NINUM : k == 2 ? NIMIN : DIMIN;
             }
			 
{FLNUM}      {
                yylval.ival = lex_fnum(yytext);
                return FNUM;
             }
			 
{IMNUM}/[^A-Za-z0-9_] {
                /* an imaginary literal, 2i or 2.5i: the comp 0 + 2i. The
                   parser writes a + bi as an expression and joins the two
                   literals (expr_addsub), so that y*3+2i is (y*3) + 2i */
                char *t = xasprintf("0+%s", yytext);
                yylval.ival = lex_cnum(t);
                free(t);
                return CNUM;
             }
			 
{STRIN}      {
                if (find_var(yytext) == -1) add_var(yytext);
                yylval.ival = find_var(yytext);
                return STRING;
             }
			 
{CARES}      {return *yytext;}
			 
\n            pctx.line_num++;

{BLOCO}      {pctx.line_num += conta_linhas(yytext);}
{COMEN}      ;
{WS}         ;
[\r\f\v]     ;
 /* any other character is no part of C±: 'a' read as a, without a word (#108 A8) */
[\xC0-\xF7][\x80-\xBF]* {fatal(MSG_ERR_UNKNOWN_CHAR, pctx.line_num+1, yytext);}
.            {fatal(MSG_ERR_UNKNOWN_CHAR, pctx.line_num+1, yytext);}

 /* End of a macro body buffer -> clear its active flag and resume the buffer
  * underneath. At the real end of the file (no live macro) stop. */
<<EOF>>      {
                if (g_def_sp > 0) {
                    g_def[g_def_stack[--g_def_sp]].active = 0;
                    yypop_buffer_state();
                } else {
                    yyterminate();
                }
             }
%%

int yywrap(void) {return 1;}

int conta_linhas(char *texto)
{
   int cnt = 0;
   for (const char *c = texto; *c; c++) if (*c == '\n') cnt++;   /* one pass: strlen each turn was quadratic (#108 A21) */
   return cnt;
}