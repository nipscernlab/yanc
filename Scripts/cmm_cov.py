#!/usr/bin/env python3
"""Numbers every call that emits assembly in cmmcomp, for Scripts/cmm_cov.sh.

A copy of Compilers/CMMComp (and of Compilers/common, which its sources
include by relative path) is written to <out>, with each call that writes
assembly (add_instr, add_sinst, mac_cat, and the fprintf of dirac.c's
#array line) wrapped as (cov_hit(K), add_instr)(...): the call still runs
as before, and K is recorded as reached.

The operator tables (static const char *const NAME[PL_N][PL_N], templates
expanded by tpl.c) all go out through the one add_instr in tpl.c, so that
call cannot tell their cells apart. Each non-NULL cell is therefore a site
of its own: every use NAME[x][y] in the copy becomes
cov_cell(K0, NAME, x, y), which records K0 + x*PL_N + y and returns the
cell. A use that is not of that form stops the script, so that no table
escapes the count.

The copy also gets cov.c, which writes the reached numbers at exit to
$COV_OUT, and sites.tsv, one line per site: K, file, line, function (for a
cell, NAME[ROWxCOL]), the line's text (for a cell, its template).

usage: python3 Scripts/cmm_cov.py <repo root> <out dir>
"""
import os
import re
import shutil
import sys

repo, out = sys.argv[1], sys.argv[2]
cmm = os.path.join(repo, "Compilers", "CMMComp")
shutil.rmtree(out, ignore_errors=True)
dst = os.path.join(out, "Compilers", "CMMComp")
shutil.copytree(os.path.join(cmm, "Headers"), os.path.join(dst, "Headers"))
shutil.copytree(os.path.join(repo, "Compilers", "common"), os.path.join(out, "Compilers", "common"))
shutil.copy(os.path.join(repo, "Compilers", "yanc_version.h"), os.path.join(out, "Compilers"))
os.makedirs(os.path.join(dst, "Sources"))

# the calls that write assembly: add_instr, add_sinst, mac_cat (a float
# routine of Includes/ appended whole) and the fprintf of the Dirac
# temporaries' #array directive (dirac.c)
CALL = re.compile(r'\b(add_instr|add_sinst|mac_cat)\s*\(|\b(fprintf)(?=\s*\(\s*f\s*,\s*"#array)\s*\(')
FUNC = re.compile(r"^[A-Za-z_][\w \*]*?\b([A-Za-z_]\w*)\s*\([^;]*\)\s*\{?\s*$")


def code_end(line, in_block):
    """Where the code of a line ends (a // comment), skipping strings and
    /* */ comments; returns that column and whether a /* is still open."""
    j = 0
    while j < len(line):
        if in_block:
            e = line.find("*/", j)
            if e < 0:
                return len(line), True
            in_block, j = False, e + 2
            continue
        if line.startswith("//", j):
            return j, False
        if line.startswith("/*", j):
            in_block, j = True, j + 2
            continue
        if line[j] == '"':
            k = j + 1
            while k < len(line) and line[k] != '"':
                k += 2 if line[k] == "\\" else 1
            j = k + 1
            continue
        j += 1
    return len(line), in_block


PLACES = ["IM", "IA", "FM", "FA", "CM", "CA"]       # tpl.h: PL_IM .. PL_CA, PL_N
PL_N = len(PLACES)
TABLE = re.compile(r"static const char \*const (\w+)\[PL_N\]\[PL_N\] = \{(.*?)\n\};", re.S)
TOKEN = re.compile(r'/\*.*?\*/|"((?:[^"\\]|\\.)*)"|\bNULL\b|[{},]', re.S)


def table_cells(body, at):
    """The cells of a table, row by row: (template or None, offset in the
    file). Adjacent string literals make one cell, as in C."""
    rows, row, cur = [], None, None
    for m in TOKEN.finditer(body):
        tok = m.group(0)
        if tok.startswith("/*"):
            continue
        if tok == "{":
            row = []
        elif tok in (",", "}"):
            if cur is not None and row is not None:
                row.append(cur)
            cur = None
            if tok == "}" and row is not None:
                rows.append(row)
                row = None
        elif tok == "NULL":
            cur = (None, at + m.start())
        else:
            text = bytes(m.group(1), "utf-8").decode("unicode_escape")
            cur = (text, at + m.start()) if cur is None else (cur[0] + text, cur[1])
    return rows


sites = []
slots = 0          # the cov_hit numbers handed out; a NULL cell takes one but is no site
src = os.path.join(cmm, "Sources")
for name in sorted(os.listdir(src)):
    path = os.path.join(src, name)
    if name in ("lex.yy.c", "y.tab.c", "y.tab.h"):
        continue                                    # generated: made again from the copies
    if not name.endswith(".c"):
        shutil.copy(path, os.path.join(dst, "Sources", name))
        continue
    lines = open(path, encoding="utf-8", errors="replace").read().split("\n")
    func, in_block, out_lines, mine = "?", False, [], False
    for i, ln in enumerate(lines, 1):
        m = FUNC.match(ln)
        if m and m.group(1) not in ("if", "while", "for", "switch"):
            func = m.group(1)
        cut, in_block_next = code_end(ln, in_block)
        new, last = "", 0
        for m in CALL.finditer(ln):
            if m.start() >= cut or in_block:
                break
            if re.search(r"\bvoid\s*$", ln[:m.start()]):      # the definition, a prototype
                continue
            fn = m.group(1) or m.group(2)
            k = slots
            slots += 1
            sites.append((k, name, i, func, ln.strip()[:160]))
            new += ln[last:m.start()] + "(cov_hit(%d), %s)(" % (k, fn)
            last, mine = m.end(), True
        out_lines.append(new + ln[last:])
        in_block = in_block_next
    body = "\n".join(out_lines)
    tables = {}
    for t in TABLE.finditer(body):
        rows = table_cells(t.group(2), t.start(2))
        if len(rows) != PL_N or any(len(r) != PL_N for r in rows):
            sys.exit("cmm_cov: %s in %s is not %d x %d" % (t.group(1), name, PL_N, PL_N))
        tables[t.group(1)] = (slots, t.start(), t.end())
        for x, r in enumerate(rows):
            for y, (tpl, off) in enumerate(r):
                if tpl is not None:
                    sites.append((slots + x * PL_N + y, name, body.count("\n", 0, off) + 1,
                                  "%s[%sx%s]" % (t.group(1), PLACES[x], PLACES[y]),
                                  tpl.replace("\n", "\\n")[:160]))
        slots += PL_N * PL_N
    for tname, (k0, start, end) in sorted(tables.items(), key=lambda kv: -kv[1][1]):
        use = re.compile(r"\b%s\b(\[(?:[^\[\]]|\[[^\]]*\])*\])?(\[[^\]]*\])?" % tname)
        parts, last = [], 0
        for m in use.finditer(body):
            if start <= m.start() < end:
                continue                                  # the definition
            if not (m.group(1) and m.group(2)):
                sys.exit("cmm_cov: %s:%d uses %s other than as %s[x][y]"
                         % (name, body.count("\n", 0, m.start()) + 1, tname, tname))
            parts.append(body[last:m.start()] + "cov_cell(%d, %s, %s, %s)"
                         % (k0, tname, m.group(1)[1:-1], m.group(2)[1:-1]))
            last = m.end()
        body = "".join(parts) + body[last:]
    if tables:
        body = body.replace('#include "../Headers/tpl.h"\n', '#include "../Headers/tpl.h"\n'
                            "const char *cov_cell(int k0, const char *const (*t)[PL_N], int x, int y);\n", 1)
        if "cov_cell(int k0" not in body:
            sys.exit("cmm_cov: %s has a table but does not include tpl.h" % name)
    if mine:
        body = "void cov_hit(int k);\n" + body
    open(os.path.join(dst, "Sources", name), "w", encoding="utf-8").write(body)

open(os.path.join(dst, "Sources", "cov.c"), "w").write(r'''#include <stdio.h>
#include <stdlib.h>
#include "../Headers/tpl.h"
static unsigned char seen[%d];
static int registered;
static void dump(void)
{
    const char *p = getenv("COV_OUT");
    FILE *f = fopen(p ? p : "cov_hits.txt", "a");
    if (!f) return;
    for (int k = 0; k < %d; k++) if (seen[k]) fprintf(f, "%%d\n", k);
    fclose(f);
}
void cov_hit(int k) {if (!registered) {registered = 1; atexit(dump);} seen[k] = 1;}
const char *cov_cell(int k0, const char *const (*t)[PL_N], int x, int y)
{
    if (t[x][y]) cov_hit(k0 + x * PL_N + y);
    return t[x][y];
}
''' % (slots, slots))

with open(os.path.join(dst, "sites.tsv"), "w", encoding="utf-8") as f:
    for s in sites:
        f.write("%d\t%s\t%d\t%s\t%s\n" % s)
print("cmm_cov: %d sites emit assembly" % len(sites))
