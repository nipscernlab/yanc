#!/usr/bin/env python3
"""Numbers every call that emits assembly in cmmcomp, for Scripts/cmm_cov.sh.

A copy of Compilers/CMMComp (and of Compilers/common, which its sources
include by relative path) is written to <out>, with each add_instr( and
add_sinst( call of the sources wrapped as (cov_hit(K), add_instr)(...):
the call still runs as before, and K is recorded as reached. The copy also
gets cov.c, which writes the reached numbers at exit to $COV_OUT, and
sites.tsv, one line per call: K, file, line, function, the line's text.

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

CALL = re.compile(r"\badd_(instr|sinst)\s*\(")
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


sites = []
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
            k = len(sites)
            sites.append((k, name, i, func, ln.strip()[:160]))
            new += ln[last:m.start()] + "(cov_hit(%d), add_%s)(" % (k, m.group(1))
            last, mine = m.end(), True
        out_lines.append(new + ln[last:])
        in_block = in_block_next
    body = "\n".join(out_lines)
    if mine:
        body = "void cov_hit(int k);\n" + body
    open(os.path.join(dst, "Sources", name), "w", encoding="utf-8").write(body)

open(os.path.join(dst, "Sources", "cov.c"), "w").write(r'''#include <stdio.h>
#include <stdlib.h>
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
''' % (len(sites), len(sites)))

with open(os.path.join(dst, "sites.tsv"), "w", encoding="utf-8") as f:
    for s in sites:
        f.write("%d\t%s\t%d\t%s\t%s\n" % s)
print("cmm_cov: %d calls emit assembly" % len(sites))
