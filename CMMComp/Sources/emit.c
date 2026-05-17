// ----------------------------------------------------------------------------
// emit sink stack (see emit.h for the design notes) --------------------------
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "..\Headers\emit.h"
#include "..\Headers\messages.h"

// ----------------------------------------------------------------------------
// local types and state ------------------------------------------------------
// ----------------------------------------------------------------------------

typedef struct {
    char *buf;     // NUL-terminated assembly chunk being accumulated
    int   len;     // current length (excluding the trailing NUL)
    int   cap;     // current allocation size of buf
} capture;

// stack of active captures. stk_n == 0 means no capture is active (default).
static capture *stk      = NULL;
static int      stk_n    = 0;
static int      stk_cap  = 0;

// ----------------------------------------------------------------------------
// public API -----------------------------------------------------------------
// ----------------------------------------------------------------------------

void emit_push_capture(void)
{
    if (stk_n + 1 > stk_cap)
    {
        int new_cap = stk_cap ? stk_cap * 2 : 8;
        capture *t = realloc(stk, (size_t)new_cap * sizeof(*t));
        if (!t) {fprintf(stderr, MSG_ERR_OUT_OF_MEMORY); exit(EXIT_FAILURE);}
        stk     = t;
        stk_cap = new_cap;
    }
    stk[stk_n].buf = NULL;
    stk[stk_n].len = 0;
    stk[stk_n].cap = 0;
    stk_n++;
}

char *emit_pop_capture(void)
{
    capture *c = &stk[--stk_n];
    char    *r = c->buf;

    // ensure the caller always gets a valid heap string, even if nothing
    // was ever appended to this capture
    if (!r)
    {
        r = malloc(1);
        if (!r) {fprintf(stderr, MSG_ERR_OUT_OF_MEMORY); exit(EXIT_FAILURE);}
        r[0] = '\0';
    }

    // detach the buffer from the (now-popped) capture slot
    c->buf = NULL;
    c->len = 0;
    c->cap = 0;
    return r;
}

int emit_is_capturing(void)
{
    return stk_n > 0;
}

void emit_append(const char *s)
{
    if (stk_n == 0) return;             // nothing to do when not capturing

    capture *c   = &stk[stk_n - 1];
    int      len = (int)strlen(s);

    // grow if the new bytes (+ NUL) don't fit
    if (c->len + len + 1 > c->cap)
    {
        int new_cap = c->cap ? c->cap : 256;
        while (new_cap < c->len + len + 1) new_cap *= 2;
        char *t = realloc(c->buf, (size_t)new_cap);
        if (!t) {fprintf(stderr, MSG_ERR_OUT_OF_MEMORY); exit(EXIT_FAILURE);}
        c->buf = t;
        c->cap = new_cap;
    }
    memcpy(c->buf + c->len, s, (size_t)len + 1);
    c->len += len;
}
