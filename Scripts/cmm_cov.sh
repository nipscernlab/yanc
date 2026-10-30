#!/bin/bash
# Which of cmmcomp's hand-written assembly sequences the tests check, and
# which ones they check for being right.
#
# Every add_instr( / add_sinst( call in Compilers/CMMComp/Sources emits a
# piece of assembly. Scripts/cmm_cov.py numbers the calls in a copy of the
# sources; this script builds that copy and compiles every
# Compilers/CMMComp/Tests/<t>/<t>.cmm that has a golden.asm, checking that
# the numbered copy still gives the golden (the numbering changes nothing).
# Then, for each call:
#   reached   it runs for some test with a golden.asm: a change to the
#             assembly it emits shows in the regression, but nothing says
#             the assembly computes the right thing;
#   verified  it runs for some test listed in Compilers/CMMComp/Tests/verified.txt:
#             a test the regression simulates and whose expected output was
#             computed apart from YANC (a Python reference, values worked out
#             by hand), not recorded from YANC's own simulation.
#
#   bash Scripts/cmm_cov.sh           the counts, per file
#   bash Scripts/cmm_cov.sh --list    and every call not verified, marked
#                                     "unreached" or "reached"
#   bash Scripts/cmm_cov.sh --check   exit 1 when more calls are unreached or
#                                     unverified than Compilers/CMMComp/Tests/cov_baseline.txt
#                                     allows (Scripts/regress.sh runs this)
# Needs gcc, bison, flex and python3.
ROOT=$(cd "$(dirname "$0")/.." && pwd)
MODE=${1:-}
T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT

python3 "$ROOT/Scripts/cmm_cov.py" "$ROOT" "$T" || exit 1
S="$T/Compilers/CMMComp/Sources"
C="$T/Compilers/common"
(cd "$S" && bison -y -d CMMComp.y 2>/dev/null && flex CMMComp.l) || { echo "cmm_cov: bison/flex failed"; exit 1; }
mkdir -p "$T/bin" "$T/w" "$T/hits"
cp -r "$ROOT/Compilers/CMMComp/Includes" "$T/Macros"          # cmmcomp looks for ../Macros next to its bin/
(cd "$S" && gcc -O0 -w -o "$T/bin/cmmcomp.exe" ast.c data_assign.c data_declar.c data_use.c itr.c \
    diretivas.c funcoes.c labels.c oper.c saltos.c stdlib.c t2t.c variaveis.c array_index.c global.c \
    macros.c messages.c args.c dirac.c lex.yy.c y.tab.c cov.c \
    "$C/yanc_num.c" "$C/asm_share.c" "$C/proc_name.c" "$C/console_utf8.c" "$C/exe_dir.c" \
    "$C/yanc_lang.c" -lm) || { echo "cmm_cov: the numbered copy did not build"; exit 1; }

bad=0
for d in "$ROOT"/Compilers/CMMComp/Tests/*/; do
    t=$(basename "$d")
    [ -f "$d/$t.cmm" ] && [ -f "$d/golden.asm" ] || continue
    cp -r "$d" "$T/w/$t"
    COV_OUT="$T/hits/$t" "$T/bin/cmmcomp.exe" -o "$T/w/$t/out" "$T/w/$t/$t.cmm" >/dev/null 2>&1
    touch "$T/hits/$t"
    tr -d '\r' < "$T/w/$t/out/$t.asm" > "$T/a.txt" 2>/dev/null
    tr -d '\r' < "$d/golden.asm" > "$T/g.txt"
    if ! cmp -s "$T/a.txt" "$T/g.txt"; then
        echo "cmm_cov: the numbered compiler does not give the golden of $t"; bad=$((bad + 1))
    fi
done
[ $bad -eq 0 ] || exit 1

python3 - "$T" "$MODE" "$ROOT/Compilers/CMMComp/Tests" <<'EOF'
import collections, os, sys
T, mode, tests = sys.argv[1], sys.argv[2], sys.argv[3]
hits = {}
for t in os.listdir(T + "/hits"):
    hits[t] = set(int(x) for x in open(T + "/hits/" + t) if x.strip())
verified = []
for l in open(tests + "/verified.txt", encoding="utf-8"):
    l = l.split("#")[0].strip()
    if not l:
        continue
    name = l.split("|")[0].strip()
    if name not in hits:
        sys.exit("cmm_cov: verified.txt names '%s', which is not a test with a golden.asm" % name)
    verified.append(name)
reached = set().union(*hits.values()) if hits else set()
checked = set().union(*(hits[t] for t in verified)) if verified else set()
sites = [l.rstrip("\n").split("\t", 4) for l in open(T + "/Compilers/CMMComp/sites.tsv", encoding="utf-8")]
unreached = [s for s in sites if int(s[0]) not in reached]
unverified = [s for s in sites if int(s[0]) not in checked]
print("cmm_cov: %d tests (%d verified), %d calls emit assembly: %d reached by no test, "
      "%d not verified" % (len(hits), len(verified), len(sites), len(unreached), len(unverified)))
tot = collections.Counter(s[1] for s in sites)
nr = collections.Counter(s[1] for s in unreached)
nv = collections.Counter(s[1] for s in unverified)
print("  %-16s %6s %10s %11s" % ("file", "calls", "unreached", "unverified"))
for f in sorted(tot, key=lambda f: -nv[f]):
    print("  %-16s %6d %10d %11d" % (f, tot[f], nr[f], nv[f]))
if mode == "--list":
    for k, f, ln, fn, txt in unverified:
        print("%-9s %s:%s  %s  %s" % ("unreached" if int(k) not in reached else "reached", f, ln, fn, txt))
if mode == "--check":
    base = {}
    for l in open(tests + "/cov_baseline.txt", encoding="utf-8"):
        p = l.split()
        if len(p) == 2 and p[0] in ("unreached", "unverified"):
            base[p[0]] = int(p[1])
    if set(base) != {"unreached", "unverified"}:
        sys.exit("cmm_cov: cov_baseline.txt needs an 'unreached N' and an 'unverified N' line")
    bad = False
    for key, now in (("unreached", len(unreached)), ("unverified", len(unverified))):
        if now > base[key]:
            print("cmm_cov: %d calls %s, the baseline allows %d: a new emitter needs a test "
                  "(bash Scripts/cmm_cov.sh --list)" % (now, key, base[key]))
            bad = True
        elif now < base[key]:
            print("cmm_cov: %d calls %s, %d fewer than the baseline: lower it in "
                  "Compilers/CMMComp/Tests/cov_baseline.txt" % (now, key, base[key] - now))
    sys.exit(1 if bad else 0)
EOF
