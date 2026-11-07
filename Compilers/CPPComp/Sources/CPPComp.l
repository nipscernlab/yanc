%{
// ----------------------------------------------------------------------------
// CPPComp — flex lexer ---------------------------------------------------------
// ----------------------------------------------------------------------------
// Tokenises the preprocessed text of a C++ program (cpppp.c), which has no
// comment and no directive left but `#pragma`. The typedef-name hack is in
// play: on each IDENT seen, the lexer consults the symbol table. If the name
// resolves to a SK_TYPEDEF the lexer emits TYPEDEF_NAME instead of IDENT, so
// the bison grammar can disambiguate type-specifier vs expression contexts.
// `#pragma yanc <key> <value>` lines are parsed inline here and recorded on
// the global unit, so they never reach the parser.
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>

#include "../Headers/ast.h"
#include "../Headers/types.h"
#include "../Headers/symtab.h"
#include "../Headers/messages.h"
#include "../Headers/config.h"
#include "../Headers/cpppp.h"
#include "../../common/xstr.h"

#include "y.tab.h"

extern unit *g_unit;   // populated by parser/main, used here to record pragmas
extern int  g_lit_uns; // set alongside yylval.ival for INT_LIT, read by the parser

// the word width of the program: its `#pragma yanc nubits`, else the target's
static int word_bits(void) { return g_unit && g_unit->nubits > 0 ? g_unit->nubits : CFG_NUBITS; }

// An integer literal's value, and whether it is unsigned: a u/U suffix, or a
// value too big for int. C++ makes an unsuffixed hex literal unsigned that
// way; a decimal one would be long long, which is one 32-bit word here too,
// so unsigned is what keeps its value. A value that does not fit the word,
// even unsigned, is an error: C++ has no type for it.
static long int_lit(const char *text, int base)
{
    errno = 0;
    unsigned long long v = strtoull(text, NULL, base);
    int w = word_bits();
    if (errno == ERANGE || (w < 64 && (v >> w) != 0))
        msg_error(yylineno, MSG_ERR_INT_TOO_BIG, text, w);
    g_lit_uns = strpbrk(text, "uU") != NULL || v > 0x7FFFFFFFULL;
    return (long)v;
}

static char *xstrdup_n(const char *s, int n)
{
    char *r = (char*)xmalloc(n + 1);
    memcpy(r, s, n); r[n] = 0; return r;
}

static int is_ident_char(char c) { return isalnum((unsigned char)c) || c == '_'; }

// A float literal as the .asm takes it, digits.digits[e[-]digits], with no
// suffix and no leading zeros (.5 -> 0.5, 5. -> 5.0, 1e5 -> 1.0e5), the form
// cmmcomp's fnum_text writes. The assembler rounds this text once, exactly, to
// the target format; going through a double first would round twice (#12).
static char *float_text(const char *p)
{
    char *r = (char*)xmalloc(strlen(p) + 4), *o = r;   // at most "0", ".", "0" added
    while (*p == '0' && p[1] >= '0' && p[1] <= '9') p++;
    if (*p == '.') *o++ = '0';
    while (*p >= '0' && *p <= '9') *o++ = *p++;
    *o++ = '.';
    if (*p == '.') p++;
    if (!(*p >= '0' && *p <= '9')) *o++ = '0';
    while (*p >= '0' && *p <= '9') *o++ = *p++;
    if (*p == 'e' || *p == 'E') {
        *o++ = 'e'; p++;
        if (*p == '+') p++; else if (*p == '-') *o++ = *p++;
        while (*p >= '0' && *p <= '9') *o++ = *p++;
    }
    *o = 0;                                           // the f/F/l/L suffix stays out
    return r;
}

// the escape past the backslash at *p, in the literal lit (pp_escape: the
// preprocessor's #if reads the same escapes)
static int escape_at(const char **p, const char *lit)
{
    int v = pp_escape(p);
    if (v == PP_ESC_BAD)   msg_error(yylineno, MSG_ERR_BAD_ESCAPE, **p);
    if (v == PP_ESC_RANGE) msg_error(yylineno, MSG_ERR_ESCAPE_RANGE, lit);
    return v;
}

// the bytes of a "..." string literal, in a new block; *out_len gets their
// number (a \0 may be one of them)
static char *unescape_string(const char *src, int *out_len)
{
    const char *end = src + strlen(src) - 1;          // the closing quote
    char *buf = (char*)xmalloc((size_t)(end - src) + 1);
    int j = 0;
    for (const char *p = src + 1; p < end; ) {
        if (*p == '\\') { p++; buf[j++] = (char)escape_at(&p, src); }
        else buf[j++] = *p++;
    }
    buf[j] = 0;
    *out_len = j;
    return buf;
}

// the value of a char literal: 'x' or an escape
static long unescape_char(const char *src)
{
    const char *p = src + 1;
    if (*p != '\\') return (unsigned char)*p;
    p++;
    return escape_at(&p, src);
}

// `#pragma <rest>`, as the preprocessor passes it on: `#pragma yanc <setting>
// <value>` sets the processor; any other pragma is ignored, as C++ does
static void pragma_line(const char *p)
{
    static const struct { const char *key; size_t off; int lo, hi; } settings[] = {
        { "nubits", offsetof(unit, nubits), 2, 32 },       // a SAPHO word has 2 to 32 bits
        { "nbmant", offsetof(unit, nbmant), 1, 30 },       // NUBITS = NBMANT + NBEXPO + 1
        { "nbexpo", offsetof(unit, nbexpo), 1, 30 },
        { "nugain", offsetof(unit, nugain), 1, INT_MAX },  // and a power of two
        { "ndstac", offsetof(unit, ndstac), 1, INT_MAX },
        { "sdepth", offsetof(unit, sdepth), 1, INT_MAX },
        { "nuioin", offsetof(unit, nuioin), 1, INT_MAX },
        { "nuioou", offsetof(unit, nuioou), 1, INT_MAX },
    };
    while (*p == ' ' || *p == '\t') p++;
    if (strncmp(p, "yanc", 4) != 0 || is_ident_char(p[4])) return;
    p += 4;
    while (*p == ' ' || *p == '\t') p++;
    const char *k = p;
    while (is_ident_char(*p)) p++;
    char *key = xstrdup_n(k, (int)(p - k));
    while (*p == ' ' || *p == '\t') p++;
    char *val = xstrdup(p);
    for (int L = (int)strlen(val); L > 0 && isspace((unsigned char)val[L-1]); ) val[--L] = 0;

    if (!*key)                  msg_error(yylineno, MSG_ERR_PRAGMA_KEY);
    if (!strcmp(key, "prname")) msg_error(yylineno, MSG_ERR_PRAGMA_PRNAME);
    if (!strcmp(key, "fftsiz")) msg_error(yylineno, MSG_ERR_PRAGMA_FFTSIZ);
    if (!strcmp(key, "itradd")) msg_error(yylineno, MSG_ERR_PRAGMA_ITRADD);
    size_t i = 0, n = sizeof settings / sizeof settings[0];
    while (i < n && strcmp(key, settings[i].key)) i++;
    if (i == n) { msg_warning(yylineno, MSG_WARN_PRAGMA_UNKNOWN, key); free(key); free(val); return; }

    char *end;
    errno = 0;
    long v = strtol(val, &end, 10);
    if (!*val || *end || errno == ERANGE) msg_error(yylineno, MSG_ERR_PRAGMA_VALUE, key, val);
    if (v < settings[i].lo || v > settings[i].hi) {
        if (settings[i].hi == INT_MAX) msg_error(yylineno, MSG_ERR_PRAGMA_MIN, key, v, settings[i].lo);
        msg_error(yylineno, MSG_ERR_PRAGMA_RANGE, key, v, settings[i].lo, settings[i].hi);
    }
    if (!strcmp(key, "nugain") && (v & (v - 1)) != 0) msg_error(yylineno, MSG_ERR_PRAGMA_POW2, v);
    if (g_unit) *(int *)((char *)g_unit + settings[i].off) = (int)v;
    free(key); free(val);
}

extern int tmpl_name_is(const char *n);   // function-template name -> TEMPLATE_NAME

static int classify_ident(const char *name)
{
    // the language's limit on a name (issue #66): checked once, here
    size_t len = strlen(name);
    if (len > CPP_NAME_MAX)
        msg_error(yylineno, MSG_ERR_NAME_TOO_LONG, (int)len, CPP_NAME_MAX);
    // C++ reserves every name with `__` in it, and here those are the
    // compiler's own names in the .asm (__fp, __L..., Class__method)
    if (strstr(name, "__"))
        msg_error(yylineno, MSG_ERR_RESERVED_DUNDER, name);
    sym *s = st_find(name);
    if (s && s->kind == SK_TYPEDEF) return TYPEDEF_NAME;
    if (tmpl_name_is(name)) return TEMPLATE_NAME;
    return IDENT;
}

%}

%option nounput noinput yylineno

DIGIT   [0-9]
HEXIT   [0-9a-fA-F]
LETTER  [a-zA-Z_]
ALNUM   [a-zA-Z0-9_]

SUFFIX  ([uU]([lL]|ll|LL)?|([lL]|ll|LL)[uU]?)
INT_DEC {DIGIT}+{SUFFIX}?
INT_HEX 0[xX]{HEXIT}+{SUFFIX}?
FLOAT1  {DIGIT}+\.{DIGIT}*([eE][+-]?{DIGIT}+)?
FLOAT2  \.{DIGIT}+([eE][+-]?{DIGIT}+)?
FLOAT3  {DIGIT}+[eE][+-]?{DIGIT}+
FLOAT   {FLOAT1}|{FLOAT2}|{FLOAT3}

IDENTIF {LETTER}{ALNUM}*
STRLIT  \"(\\.|[^\\"\n])*\"
CHRLIT  \'(\\([0-7]{1,3}|x{HEXIT}+|.)|[^\\'\n])\'

%%

^"#pragma"[^\n]*         { pragma_line(yytext + 7); }

[ \t\r]+                 { }
\n                       { }

  /* keywords ----------------------------------------------------------- */

"void"                   { return KW_VOID;     }
"int"                    { return KW_INT;      }
"float"                  { return KW_FLOAT;    }
"char"                   { return KW_CHAR;     }
"short"                  { return KW_SHORT;    }
"long"                   { return KW_LONG;     }
"double"                 { return KW_DOUBLE;   }
"_Bool"                  { return KW_BOOL;     }
"bool"                   { return KW_BOOL;     }
"__yanc_int8"            { return KW_I8;       }   /* <cstdint>'s exact-width types */
"__yanc_uint8"           { return KW_U8;       }
"__yanc_int16"           { return KW_I16;      }
"__yanc_uint16"          { return KW_U16;      }
"unsigned"               { return KW_UNSIGNED; }
"signed"                 { return KW_SIGNED;   }
"__signed"               { return KW_SIGNED;   }
"__signed__"             { return KW_SIGNED;   }

"if"                     { return KW_IF;       }
"else"                   { return KW_ELSE;     }
"while"                  { return KW_WHILE;    }
"for"                    { return KW_FOR;      }
"do"                     { return KW_DO;       }
"switch"                 { return KW_SWITCH;   }
"case"                   { return KW_CASE;     }
"default"                { return KW_DEFAULT;  }
"break"                  { return KW_BREAK;    }
"continue"               { return KW_CONTINUE; }
"return"                 { return KW_RETURN;   }
"goto"                   { return KW_GOTO;     }

"asm"                    { return KW_ASM;      }
"__asm__"                { return KW_ASM;      }
"struct"                 { return KW_STRUCT;   }
"class"                  { return KW_CLASS;    }
"this"                   { return KW_THIS;     }
"virtual"                { return KW_VIRTUAL;  }
"override"               { return KW_OVERRIDE; }
"final"                  { return KW_FINAL;    }
"static_cast"            { return KW_CPPCAST;  }
"reinterpret_cast"       { return KW_CPPCAST;  }
"const_cast"             { return KW_CPPCAST;  }
"dynamic_cast"           { return KW_CPPCAST;  }
"explicit"               { /* ignored */ }
"mutable"                { /* ignored */ }
"operator"               { return KW_OPERATOR; }
"template"               { return KW_TEMPLATE; }
"typename"               { return KW_TYPENAME; }
"auto"                   { return KW_AUTO;     }
"public"                 { return KW_PUBLIC;    }
"private"                { return KW_PRIVATE;   }
"protected"              { return KW_PROTECTED; }
"union"                  { return KW_UNION;    }
"typedef"                { return KW_TYPEDEF;  }
"enum"                   { return KW_ENUM;     }
"sizeof"                 { return KW_SIZEOF;   }
"new"                    { return KW_NEW;      }
"delete"                 { return KW_DELETE;   }
"namespace"              { return KW_NAMESPACE; }
"using"                  { return KW_USING;    }
"true"                   { yylval.ival = 1; g_lit_uns = 2; return INT_LIT; }   /* 2: a bool */
"false"                  { yylval.ival = 0; g_lit_uns = 2; return INT_LIT; }
"nullptr"                { yylval.ival = 0; g_lit_uns = 0; return INT_LIT; }

"_Generic"               { return KW_GENERIC; }
"_Static_assert"         { return KW_STATIC_ASSERT; }
"static_assert"          { return KW_STATIC_ASSERT; }   /* C23 / <assert.h> spelling */
"static"                 { return KW_STATIC;   }
"extern"                 { return KW_EXTERN;   }
"const"                  { return KW_CONST;    }
"constexpr"              { return KW_CONST;    }  /* treat as const: int globals
                                                     get the existing constant
                                                     folding, float consts behave
                                                     as runtime-initialised const */
"inline"                 { /* ignored — every fn is potentially inlined */ }
"__inline"               { /* ignored */ }
"__inline__"             { /* ignored */ }
"register"               { /* ignored */ }
"volatile"               { /* ignored — relevant only for INN/OUT, which are explicit */ }
"noexcept"               { /* ignored — no exceptions on this target; only the
                              function-specifier form is recognised (noexcept(expr)
                              would leave a dangling paren expression). */ }
"__volatile__"           { /* ignored */ }
"restrict"               { /* ignored — no aliasing model on this target */ }
"__restrict"             { /* ignored */ }
"__restrict__"           { /* ignored */ }
"_Noreturn"              { /* ignored */ }

  /* literals ----------------------------------------------------------- */

{INT_HEX}                { yylval.ival = int_lit(yytext, 16); return INT_LIT; }  /* truncates to the target word */
{INT_DEC}                { /* a leading 0 makes it octal, as in C++ (010 is 8); an 8 or 9 then is an error */
                           int oct = yytext[0] == '0' && yytext[1] >= '0' && yytext[1] <= '9';
                           if (oct && strpbrk(yytext, "89")) msg_error(yylineno, MSG_ERR_OCTAL, yytext);
                           yylval.ival = int_lit(yytext, oct ? 8 : 10); return INT_LIT; }
{FLOAT}[fFlL]?           { yylval.sval = float_text(yytext);       return FLOAT_LIT; }
{DIGIT}{ALNUM}*          { msg_error(yylineno, MSG_ERR_NUM_SUFFIX, yytext); }   /* 1uuu, 12ab: not a number */
{CHRLIT}                 { yylval.ival = unescape_char(yytext);    return CHAR_LIT; }
{STRLIT}                 {
                           int len = 0;
                           char *bytes = unescape_string(yytext, &len);
                           yylval.sval = bytes;
                           // we sneak the length in via the global g_str_len so the
                           // parser can read it (alternative: a struct in %union)
                           extern int g_str_len; g_str_len = len;
                           return STRING_LIT;
                         }

  /* identifiers (typedef hack) ----------------------------------------- */

{IDENTIF}([ \t]*"::"[ \t]*{IDENTIF})+ {
                           /* A whole qualified chain `A::b` / `A::B::c`. Which
                              token the FIRST name gets depends on what the LAST
                              one denotes, because after `T x(` the parser has
                              one token of lookahead to choose between a
                              parameter type and a constructor argument:
                              `Filter f(std::uint8_t v)` is a prototype while
                              `Filter f(geo::scale)` declares an object. Emit
                              NS_IDENT when the chain ends in a type and
                              NS_VIDENT when it ends in a value, then push
                              everything after the first name back so the `::`
                              and the remaining names lex as usual.
                              A chain whose FIRST name is itself a type
                              (`Color::Red`, `Filter::apply`) is left alone: it
                              keeps returning TYPEDEF_NAME as before. */
                           int fl = 0;
                           while (fl < yyleng && is_ident_char(yytext[fl])) fl++;
                           int ls = yyleng;
                           while (ls > 0 && is_ident_char(yytext[ls-1])) ls--;
                           char *first = xstrdup_n(yytext, fl);
                           char *last  = xstrdup_n(yytext + ls, yyleng - ls);
                           int ft = classify_ident(first);
                           int lt = classify_ident(last);
                           free(last);
                           yyless(fl);                 /* keep only the first name */
                           yylval.sval = first;
                           if (ft != IDENT) return ft;
                           return lt == IDENT ? NS_VIDENT : NS_IDENT; }

{IDENTIF}/[ \t]*"::"     { /* a name followed by `::`: a type stays TYPEDEF_NAME.
                              Reached when the chain is not a plain run of names
                              (`N::Type<...>::x`, `Color::` before a template
                              bracket, ...) — the rule above handles the rest. */
                           yylval.sval = xstrdup_n(yytext, yyleng);
                           int t = classify_ident(yytext);
                           return t == IDENT ? NS_IDENT : t; }
{IDENTIF}                { yylval.sval = xstrdup_n(yytext, yyleng); return classify_ident(yytext); }

  /* multi-char operators ----------------------------------------------- */

"=="                     { return TOK_EQ;     }
"!="                     { return TOK_NE;     }
"<="                     { return TOK_LE;     }
">="                     { return TOK_GE;     }
"<<="                    { return TOK_SHLEQ;  }
">>="                    { return TOK_SHREQ;  }
"<<"                     { return TOK_SHL;    }
">>"                     { return TOK_SHR;    }
"&&"                     { return TOK_LAND;   }
"||"                     { return TOK_LOR;    }
"++"                     { return TOK_INC;    }
"--"                     { return TOK_DEC;    }
"+="                     { return TOK_PLUSEQ; }
"-="                     { return TOK_MINUSEQ;}
"*="                     { return TOK_STAREQ; }
"/="                     { return TOK_SLASHEQ;}
"%="                     { return TOK_PERCENTEQ; }
"&="                     { return TOK_AMPEQ;  }
"|="                     { return TOK_PIPEEQ; }
"^="                     { return TOK_CARETEQ;}
"->"                     { return TOK_ARROW;  }
"::"                     { return TOK_SCOPE;  }
"..."                    { return TOK_ELLIPSIS; }

  /* single-char punctuation, returned by literal ASCII value ----------- */
[(){}\[\];,.\?\:+\-*/%&|^~!=<>] { return yytext[0]; }

.                        { msg_error(yylineno, MSG_ERR_BAD_CHAR, yytext[0], (unsigned char)yytext[0]); }

%%

int yywrap(void) { return 1; }
