#!/usr/bin/env python3
"""No statement of a C± test leaves a word on the data stack.

For every Compilers/CMMComp/Tests/<t>/golden.asm, the depth of the data stack
is worked out at each instruction, along every path, from the effect of each
mnemonic in Compilers/common/isa.tsv (its stack and flow columns), as
asm_depth in Compilers/common/asm_share.c does to size the stacks; a call
adds the net effect of the routine it calls. Then:

- in the routine that starts at address 0 (main), the stack is empty at every
  output instruction (the io column says out: the end of an out() or a toma()
  statement) and at @fim, the end of the program;
- in a function, the depth at every output instruction is the depth after its
  prologue (the SET / SET_P that take its parameters off the stack);
- two paths reach one instruction with one depth, and a routine's RETs leave
  one depth.

A word left on the stack by any statement shows at the next output: the
generated tests of Scripts/cov_tests.py end every statement with one.

usage: python3 Scripts/stack_balance.py [repo-root]       every test's golden.asm
       python3 Scripts/stack_balance.py [repo-root] x.asm  those files only
"""
import glob
import os
import re
import sys

args = [a for a in sys.argv[1:] if not a.endswith(".asm")]
files = [a for a in sys.argv[1:] if a.endswith(".asm")]
root = args[0] if args else os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
isa = {}
for line in open(os.path.join(root, "Compilers", "common", "isa.tsv"), encoding="utf-8"):
    if line.startswith("#") or not line.strip():
        continue
    f = line.rstrip("\n").split("\t")
    isa[f[0]] = {"stack": f[4], "flow": f[6], "io": f[7]}


def parse(path):
    """the instructions of an .asm: (mnemonic, operand, source line), and label -> index"""
    ins, lab = [], {}
    for n, ln in enumerate(open(path, encoding="utf-8", errors="replace"), 1):
        ln = ln.split("//")[0].strip()
        while ln.startswith("@"):
            p = ln.split(None, 1)
            lab[p[0][1:]] = len(ins)
            ln = p[1].strip() if len(p) > 1 else ""
        if not ln or ln.startswith("#"):
            continue
        p = ln.split(None, 1)
        if p[0] not in isa:
            raise SystemExit("stack_balance: %s:%d: '%s' is not in isa.tsv" % (path, n, p[0]))
        ins.append((p[0], p[1].strip() if len(p) > 1 else "", n))
    return ins, lab


def check(path):
    ins, lab = parse(path)
    errors = []
    routines = {}                                    # entry -> (depth per instruction, net)

    def walk(entry, stack_of_calls=()):
        if entry in routines:
            return routines[entry]
        if entry in stack_of_calls:
            raise SystemExit("stack_balance: %s: a routine calls itself back" % path)
        dep, work, net = {entry: 0}, [entry], None
        while work:
            i = work.pop()
            d = dep[i]
            m, opnd, _ = ins[i]
            x = isa[m]
            d += 1 if x["stack"] == "push" else -1 if x["stack"] == "pop" else 0
            nxt = []
            if x["flow"] == "jmp":
                nxt = [(lab[opnd], d)]
            elif x["flow"] == "jz":
                nxt = [(lab[opnd], d), (i + 1, d)]
            elif x["flow"] == "call":
                _, cnet = walk(lab[opnd], stack_of_calls + (entry,))
                nxt = [(i + 1, d + cnet)]
            elif x["flow"] == "ret":
                if net is None:
                    net = d
                elif net != d:
                    errors.append("line %d: two RETs leave different stack depths" % ins[i][2])
            else:
                nxt = [(i + 1, d)]
            for k, v in nxt:
                if k >= len(ins):
                    continue                         # the end of the code
                if k not in dep:
                    dep[k] = v; work.append(k)
                elif dep[k] != v:
                    errors.append("line %d: two paths reach it with stack depths %d and %d"
                                  % (ins[k][2], dep[k], v))
        routines[entry] = (dep, net or 0)
        return routines[entry]

    main_dep, _ = walk(0)
    for i, d in main_dep.items():
        m, opnd, n = ins[i]
        if isa[m]["io"] == "out" and d != 0:
            errors.append("line %d: %s %s with %d word(s) left on the stack" % (n, m, opnd, d))
    if "fim" in lab and lab["fim"] in main_dep and main_dep[lab["fim"]] != 0:
        errors.append("@fim with %d word(s) left on the stack" % main_dep[lab["fim"]])
    for entry, (dep, _) in routines.items():
        if entry == 0:
            continue
        k = entry                                     # the prologue: the parameters' SET / SET_P
        while k < len(ins) and ins[k][0] in ("SET", "SET_P") and k in dep:
            k += 1
        base = dep.get(k)
        for i, d in dep.items():
            m, opnd, n = ins[i]
            if isa[m]["io"] == "out" and base is not None and d != base:
                errors.append("line %d: %s %s, in a function, %d word(s) off its stack"
                              % (n, m, opnd, d - base))
    return errors


bad = 0
tests = files or sorted(glob.glob(os.path.join(root, "Compilers", "CMMComp", "Tests", "*", "golden.asm")))
for path in tests:
    errs = check(path)
    if errs:
        bad += 1
        name = os.path.basename(os.path.dirname(path))
        for e in sorted(set(errs))[:5]:
            print("stack_balance: %s: %s" % (name, e))
print("stack_balance: %d tests, %d leave a word on the stack" % (len(tests), bad))
sys.exit(1 if bad else 0)
