%{
// ----------------------------------------------------------------------------
// CPPComp — flex lexer ---------------------------------------------------------
// ----------------------------------------------------------------------------
// Tokenises a C99 subset for the YANC processor. The typedef-name hack is in
// play: on each IDENT seen, the lexer consults the symbol table. If the name
// resolves to a SK_TYPEDEF the lexer emits TYPEDEF_NAME instead of IDENT, so
// the bison grammar can disambiguate type-specifier vs expression contexts.
// `#pragma yanc <key> <value>` lines are parsed inline here and recorded on
// the global unit, so they never reach the parser.
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../Headers/ast.h"
#include "../Headers/types.h"
#include "../Headers/symtab.h"
#include "../Headers/messages.h"

#include "y.tab.h"

extern unit *g_unit;   // populated by parser/main, used here to record pragmas

static char *xstrdup_n(const char *s, int n)
{
    char *r = (char*)malloc(n + 1);
    if (!r) msg_internal("oom");
    memcpy(r, s, n); r[n] = 0; return r;
}

// extract the body of a "..." string literal, honouring \n \t \r \\ \" \0
// returns a freshly-allocated buffer and writes the byte length to *out_len.
static char *unescape_string(const char *src, int *out_len)
{
    int len = strlen(src);
    char *buf = (char*)malloc(len + 1);
    int j = 0;
    // src begins/ends with " — skip them
    for (int i = 1; i < len - 1; i++) {
        char c = src[i];
        if (c == '\\' && i+1 < len-1) {
            char n = src[++i];
            switch (n) {
                case 'n':  buf[j++] = '\n'; break;
                case 't':  buf[j++] = '\t'; break;
                case 'r':  buf[j++] = '\r'; break;
                case '0':  buf[j++] = '\0'; break;
                case '\\': buf[j++] = '\\'; break;
                case '"':  buf[j++] = '"';  break;
                case '\'': buf[j++] = '\''; break;
                default:   buf[j++] = n;    break;
            }
        } else {
            buf[j++] = c;
        }
    }
    buf[j] = 0;
    *out_len = j;
    return buf;
}

static long unescape_char(const char *src)
{
    // src is like 'x' or '\n' (length 3 or 4)
    if (src[1] != '\\') return (long)(unsigned char)src[1];
    switch (src[2]) {
        case 'n':  return '\n';
        case 't':  return '\t';
        case 'r':  return '\r';
        case '0':  return 0;
        case '\\': return '\\';
        case '\'': return '\'';
        case '"':  return '"';
        default:   return src[2];
    }
}

extern int tmpl_name_is(const char *n);   // function-template name -> TEMPLATE_NAME

static int classify_ident(const char *name)
{
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

INT_DEC {DIGIT}+[uUlL]*
INT_HEX 0[xX]{HEXIT}+[uUlL]*
FLOAT1  {DIGIT}+\.{DIGIT}*([eE][+-]?{DIGIT}+)?
FLOAT2  \.{DIGIT}+([eE][+-]?{DIGIT}+)?
FLOAT3  {DIGIT}+[eE][+-]?{DIGIT}+
FLOAT   {FLOAT1}|{FLOAT2}|{FLOAT3}

IDENTIF {LETTER}{ALNUM}*
STRLIT  \"(\\.|[^\\"\n])*\"
CHRLIT  \'(\\.|[^\\'\n])\'

%%

"//"[^\n]*               { /* line comment */ }
"/*"([^*]|\*+[^*/])*\*+"/"  { /* block comment */ for (char *p = yytext; *p; p++) if (*p == '\n') yylineno++; }

"#pragma"[ \t]+"yanc"[ \t]+{IDENTIF}[ \t]+[^\n]* {
    /* parse "#pragma yanc <key> <value>" */
    char key[64] = {0}, val[256] = {0};
    if (sscanf(yytext, "#pragma yanc %63s %255[^\n\r]", key, val) == 2) {
        /* trim trailing whitespace from val */
        int L = strlen(val);
        while (L > 0 && (val[L-1] == ' ' || val[L-1] == '\t' || val[L-1] == '\r')) val[--L] = 0;
        if (g_unit) {
            if      (!strcmp(key, "prname")) { free(g_unit->prname); g_unit->prname = xstrdup_n(val, strlen(val)); }
            else if (!strcmp(key, "nubits")) g_unit->nubits = atoi(val);
            else if (!strcmp(key, "nbmant")) g_unit->nbmant = atoi(val);
            else if (!strcmp(key, "nbexpo")) g_unit->nbexpo = atoi(val);
            else if (!strcmp(key, "nugain")) g_unit->nugain = atoi(val);
            else if (!strcmp(key, "ndstac")) g_unit->ndstac = atoi(val);
            else if (!strcmp(key, "sdepth")) g_unit->sdepth = atoi(val);
            else if (!strcmp(key, "nuioin")) g_unit->nuioin = atoi(val);
            else if (!strcmp(key, "nuioou")) g_unit->nuioou = atoi(val);
            else if (!strcmp(key, "fftsiz")) g_unit->fftsiz = atoi(val);
            else if (!strcmp(key, "itradd")) g_unit->itradd = atoi(val);
            else msg_warning(yylineno, "unknown #pragma yanc setting '%s'", key);
        }
    }
}

"#line"[^\n]*            { /* preprocessor line directive — ignore */ }
"#"[^\n]*                { /* any other stray '#' directive — ignore */ }

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
"true"                   { yylval.ival = 1; return INT_LIT; }
"false"                  { yylval.ival = 0; return INT_LIT; }
"nullptr"                { yylval.ival = 0; return INT_LIT; }

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

{INT_HEX}                { yylval.ival = (long)strtoull(yytext, NULL, 16); return INT_LIT; }  /* truncates to the target word; suffixes ignored */
{INT_DEC}                { yylval.ival = (long)strtoull(yytext, NULL, 10); return INT_LIT; }
{FLOAT}f?                { yylval.fval = strtod(yytext, NULL);     return FLOAT_LIT; }
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

.                        { msg_error(yylineno, "unexpected character '%c' (0x%02x)", yytext[0], (unsigned char)yytext[0]); }

%%

int yywrap(void) { return 1; }
