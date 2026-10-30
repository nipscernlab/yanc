#!/usr/bin/env python3
"""Every combination of the Dirac notation through cmmcomp.

Reads Compilers/CMMComp/DiracCases/cases.tsv: one statement a line and what
cmmcomp must do with it (compile, stop because the combination is not
computed yet, or stop with a message that contains a given text). Each
statement goes into main() of a small program that declares the names of
PRELUDE, and cmmcomp compiles it with YANC_LANG=en.

    python3 Scripts/dirac_cases.py <root> <cmmcomp> [--ref <cmmcomp>]

With --ref, every case expected to compile is also compiled by the other
cmmcomp (the one of main, say) and the two assemblies must be identical:
the check that a change of the parser left the code of every old form as
it was. A case the other cmmcomp rejects is counted apart, as new.

Exit status 0 when every case does what the table says, 1 otherwise.
"""
import os
import subprocess
import sys
import tempfile

PRELUDE = """#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 1
#NUIOOU 1

float c; float d; float x;
int   n; int   k;
comp  z;
float a[3]; float b[3]; float e[3]; float u[2];
int   ia[3]; int ib[3];
comp  za[3]; comp zb[3];
float M[3][3]; float N[3][3]; float R[3][2]; float Q[2][3];
int   IM[3][3];
float v[3]; float y[2]; float V[3][3]; float W[3][2];
int   iv[3]; int IV[3][3];

void main()
{
"""

OVEN = "still in the oven"


def cases(path):
    with open(path, encoding="utf-8") as f:
        for num, raw in enumerate(f, 1):
            line = raw.rstrip("\r\n")
            if not line.strip() or line.startswith("//"):
                continue
            if "\t" not in line:
                sys.exit(f"{path}:{num}: no tab between the statement and the expected result")
            stmt, expect = line.split("\t", 1)
            yield num, stmt.strip(), expect.strip()


def compile_case(cmmcomp, work, name, stmt):
    src = os.path.join(work, name + ".cmm")
    with open(src, "w", encoding="utf-8", newline="\n") as f:
        f.write(PRELUDE + "    " + stmt + "\n}\n")
    out = os.path.join(work, name)
    env = dict(os.environ, YANC_LANG="en")
    p = subprocess.run([cmmcomp, "-o", out, src], capture_output=True, env=env)
    text = (p.stdout + p.stderr).decode("utf-8", "replace")
    asm = os.path.join(out, name + ".asm")
    code = None
    if p.returncode == 0 and os.path.exists(asm):
        with open(asm, encoding="utf-8", errors="replace") as f:
            code = f.read()
    return p.returncode, text, code


def main():
    args = sys.argv[1:]
    ref = None
    if "--ref" in args:
        k = args.index("--ref")
        ref = args[k + 1]
        del args[k:k + 2]
    if len(args) != 2:
        sys.exit(__doc__)
    root, cmmcomp = args
    table = os.path.join(root, "Compilers", "CMMComp", "DiracCases", "cases.tsv")

    passed = failed = same = new = 0
    with tempfile.TemporaryDirectory() as work:
        for num, stmt, expect in cases(table):
            name = f"case{num}"
            rc, text, code = compile_case(cmmcomp, work, name, stmt)
            last = text.strip().splitlines()[-1] if text.strip() else ""
            if expect == "ok":
                good = rc == 0
            else:
                want = OVEN if expect == "oven" else expect
                good = rc != 0 and want in text
            if not good:
                failed += 1
                print(f"FAIL line {num}: {stmt}  (expected {expect}; exit {rc}: {last})")
                continue
            passed += 1
            if ref and expect == "ok":
                rrc, _, rcode = compile_case(ref, os.path.join(work), name + "_ref", stmt)
                if rrc != 0:
                    new += 1
                    print(f"new  line {num}: {stmt}  (--ref rejects it)")
                elif rcode.replace(name + "_ref", name) == code:
                    same += 1
                else:
                    failed += 1
                    passed -= 1
                    print(f"FAIL line {num}: {stmt}  (the assembly differs from --ref's)")

    tail = f", {same} with the same assembly as --ref, {new} new" if ref else ""
    print(f"dirac_cases: {passed} passed, {failed} failed{tail}")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
