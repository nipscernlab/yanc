%{

#include "y.tab.h"

#include "../Headers/global.h"
#include "../Headers/variaveis.h"
#include "../Headers/data_declar.h"

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
#define CMM_MAX_DEFINES 256
static struct { char name[64]; char *body; int active; } g_def[CMM_MAX_DEFINES];
static int  g_ndef = 0;
static int  g_def_stack[CMM_MAX_DEFINES];   /* indices of the live body buffers */
static int  g_def_sp = 0;
static char g_def_pending[64];              /* name captured before its body   */

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
        if (g_ndef >= CMM_MAX_DEFINES) return;               /* table full: ignore */
        i = g_ndef++;
        snprintf(g_def[i].name, sizeof g_def[i].name, "%s", name);
        g_def[i].body = NULL;
    }
    free(g_def[i].body);
    g_def[i].body = strdup(body);
    g_def[i].active = 0;
}

%}

%option nounput noinput

%x DEFNAME DEFBODY

LETRA  [A-Za-z_]
WS     [ \t]+

VARIA  {LETRA}({LETRA}|[0-9])*
INNUM  [0-9]+
FLNUM  (0|[1-9]+[0-9]*)\.?[0-9]*
CONUM  [-]*{WS}*{FLNUM}{WS}*[+|-]{WS}*{FLNUM}{WS}*[i]

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

"#PRACA"      return ITRADD;
"#TOAQUI"     return TOAQUI;

 /* object-like #define NAME body -- recorded here, expanded at the {VARIA} rule */
"#define"               BEGIN(DEFNAME);
<DEFNAME>{WS}           ;
<DEFNAME>{VARIA}        { snprintf(g_def_pending, sizeof g_def_pending, "%s", yytext); BEGIN(DEFBODY); }
<DEFNAME>\n             { line_num++; BEGIN(INITIAL); }
<DEFNAME>.              BEGIN(INITIAL);
<DEFBODY>[^\n]*         { def_add(g_def_pending, yytext); BEGIN(INITIAL); }

"in"          return INN;
"fin"         return FIN;
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

"comp"       {type_tmp = 3; yylval.ival = 3; return TYPE;}
"float"      {type_tmp = 2; yylval.ival = 2; return TYPE;}
"int"        {type_tmp = 1; yylval.ival = 1; return TYPE;}
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
                    yylval.ival = exec_id(yytext);
                    return ID;
                }
             }
			 
{INNUM}      {
                yylval.ival = exec_inum(yytext);
                return INUM;
             }
			 
{FLNUM}      {
                yylval.ival = exec_fnum(yytext);
                return FNUM;
             }
			 
{CONUM}      {
                yylval.ival = exec_cnum(yytext);
                return CNUM;
             }
			 
{STRIN}      {
                if (find_var(yytext) == -1) add_var(yytext);
                yylval.ival = find_var(yytext);
                return STRING;
             }
			 
{CARES}      {return *yytext;}
			 
\n            line_num++;

{BLOCO}      {line_num += conta_linhas(yytext);}
{COMEN}      ;
{WS}         ;
.            ;

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
   for(int i=0;i<strlen(texto);i++) if (texto[i] == '\n') cnt++;
   return cnt;
}