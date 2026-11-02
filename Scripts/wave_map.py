#!/usr/bin/env python3
"""Check that the waveform puts each instruction on the source line that produced it.

Usage:
  wave_map.py <output folder> <name> [<messages file>]   check, then print the map
  wave_map.py <output folder> <name> --vcd <file.vcd>    check a simulation's tracks
  wave_map.py <output folder> <name> --selftest          the rules must catch broken maps

The waveform of a SAPHO program shows, cycle by cycle, the source line, the
instruction and the variables. Nothing reads them from the .asm: they come
from tables the compilers write beside it, and asmcomp completes them.

  trad_opcode.txt     address -> instruction (asmcomp, after its NOPs)
  pc_<name>_mem.txt   address -> source line, 20-bit two's complement
                      (cmmcomp, shifted by asmcomp when it adds a NOP)
  src_lines.txt       source line -> text (-1 INTERNAL, -2 the call to main,
                      -3 END)
  prog_log.txt        the variables, num_ins (how many addresses have a
                      source line), srcdir, lang

1. The tables agree with each other: trad_opcode.txt numbers the addresses
   0, 1, 2, ... and has as many as <name>_inst.mif has words, and they are
   the instructions of the .asm in order; pc_<name>_mem.txt has num_ins
   lines of 20 binary digits; every source line it names is in
   src_lines.txt. The addresses after num_ins are the library macros cmmcomp
   appends, which have no source line.

2. Each instruction is on a line that can have produced it. Nothing else
   says which line that is, so these rules hold what is true of every
   correct map (a "statement" is the line with the lines before it that it
   continues, the ones ending in , ( or an operator):
     function   the line is in the function whose body holds the address
                (or outside every function, before the first one);
     variable   an instruction whose operand is a variable of the program
                is on a statement that names that variable;
     call       CAL f is on a statement that calls f;
     io         an instruction that reads an input or writes an output
                (isa.tsv's io column) is on a statement that calls something;
     order      in straight-line code (no label, jump or call between) the
                line does not go back, except to the head of a while, for,
                do, if, else or switch, or within one statement;
     together   in straight-line code a statement's instructions are not
                split by another statement's, with the same exceptions;
     jump       a jump to a loop's, if's, switch's, pow's, floor's or ceil's
                label is on that construct's line, or on a break, continue,
                if or switch (a case whose body is only a jump).
   --selftest breaks the map of a test the way a compiler bug would (one
   kind of instruction, or the first or last instruction of every line,
   moved to the line before or after; the map shifted by one address) and
   fails when the rules do not notice: a rule loosened by mistake shows up.

3. With --vcd, the waveform's two tracks show the same address: at every
   change, the source-line track (linetabs) is the line pc_<name>_mem.txt
   gives the address on the instruction track (valr2). The two are two
   registers behind the PC each (asmcomp's hdl.c); a change to one delay
   and not the other puts every instruction on the line of its neighbour.

Printed after the checks: the messages (when a file of them is given, with
the folder of the run replaced by $DIR), the variables, then each source
line followed by the instructions the waveform shows on it. The regression
keeps that as golden_map.txt, so a change to any of it shows up for review.
"""
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
CONT = tuple(",(+-*/&|=<>")                      # a line ending so continues
HEAD = re.compile(r"\b(while|for|do|if|else|switch)\b")
JUMP_FAMILY = [("__Lwh", "while|for|do"), ("__Lif", "if|else"), ("__Lpow", "pow"),
               ("__Lflr", "floor"), ("__Lcel", "ceil"),
               ("__switch", "switch|case|default"), ("__sw", "switch|case|default")]


def fail(msg):
    print(f"wave_map: {msg}", file=sys.stderr)
    sys.exit(1)


def lines(path):
    try:
        with open(path, encoding="utf-8", errors="replace") as f:
            return [ln.rstrip("\r\n") for ln in f]
    except OSError:
        fail(f"cannot read {path}")


def signed20(bits):
    v = int(bits, 2)
    return v - (1 << 20) if v >= 1 << 19 else v


def asm_program(path):
    """The instructions of the .asm in order ('MNEMONIC operand'), and the
    labels on each."""
    ins, labels, carry = [], [], []
    for ln in lines(path):
        ln = ln.split("//")[0].strip()
        while ln.startswith("@"):                 # one or more labels
            parts = ln.split(None, 1)
            carry.append(parts[0][1:])
            ln = parts[1].strip() if len(parts) > 1 else ""
        if not ln or ln.startswith("#"):          # a label alone is on the next instruction
            continue
        labels.append(carry)
        carry = []
        ins.append(" ".join(ln.split()))
    return ins, labels


def strip_comments(text):
    """The source lines with // and /* */ comments blanked."""
    out, block = [], False
    for ln in text:
        res, i = "", 0
        while i < len(ln):
            if block:
                j = ln.find("*/", i)
                if j < 0:
                    break
                block, i = False, j + 2
            elif ln.startswith("/*", i):
                block, i = True, i + 2
            elif ln.startswith("//", i):
                break
            else:
                res += ln[i]
                i += 1
        out.append(res)
    return out


def functions(code):
    """name -> (first line, last line) of every function, from its header to
    the brace that closes its body."""
    rng, depth, cur, start = {}, 0, None, 0
    for n in sorted(k for k in code if k > 0):
        ln = code[n]
        if depth == 0 and cur is None:
            m = re.match(r"\s*(?:void|int|float|comp)\s+(\w+)\s*\(", ln)
            if m:
                cur, start = m.group(1), n
        for ch in ln:
            if ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0 and cur is not None:
                    rng[cur] = (start, n)
                    cur = None
    return rng


class Program:
    """The tables of one compiled program, checked to agree with each other."""

    def __init__(self, d, name):
        self.d, self.name = d, name
        self.trad = []
        for k, ln in enumerate(lines(os.path.join(d, "trad_opcode.txt"))):
            parts = ln.split(None, 1)
            if not parts or parts[0] != str(k):
                fail(f"trad_opcode.txt line {k + 1}: expected address {k}, got '{ln}'")
            self.trad.append(" ".join(parts[1].split()) if len(parts) > 1 else "")

        words = [ln for ln in lines(os.path.join(d, f"{name}_inst.mif")) if ln.strip()]
        if len(words) != len(self.trad):
            fail(f"{name}_inst.mif has {len(words)} words, trad_opcode.txt {len(self.trad)} addresses")

        self.num_ins, self.variables, self.var_of = None, [], {}
        for ln in lines(os.path.join(d, "prog_log.txt")):
            p = ln.split()
            if len(p) == 2 and p[0] == "num_ins":
                self.num_ins = int(p[1])
            elif p and p[0] in ("srcdir",):
                continue                          # the folder of the run
            else:
                self.variables.append(ln)
                if len(p) == 3 and p[0] != "lang":
                    f, v = p[0], p[1]
                    asm = v if f == "global" else f"{f}_{v}"
                    if v.endswith("_i") and asm[:-2] in self.var_of:
                        continue                  # the imaginary half of a comp
                    self.var_of[asm] = v
                    self.var_of[asm + "_i"] = v
        if self.num_ins is None:
            fail("prog_log.txt has no num_ins")

        pcmem = lines(os.path.join(d, f"pc_{name}_mem.txt"))
        if len(pcmem) != self.num_ins:
            fail(f"pc_{name}_mem.txt has {len(pcmem)} lines, prog_log.txt num_ins {self.num_ins}")
        if self.num_ins > len(self.trad):
            fail(f"num_ins {self.num_ins} is more than the {len(self.trad)} addresses")
        for k, ln in enumerate(pcmem):
            if not re.fullmatch(r"[01]{20}", ln):
                fail(f"pc_{name}_mem.txt line {k + 1}: '{ln}' is not 20 binary digits")
        self.line = [signed20(ln) for ln in pcmem]

        self.text = {}
        for ln in lines(os.path.join(d, "src_lines.txt")):
            p = ln.split(" ", 1)
            try:
                self.text[int(p[0])] = p[1] if len(p) > 1 else ""
            except ValueError:
                fail(f"src_lines.txt: '{ln}' does not start with a line number")
        for k, s in enumerate(self.line):
            if s not in self.text:
                fail(f"address {k} is on source line {s}, which src_lines.txt lacks")

        asm, self.labels = asm_program(os.path.join(d, f"{name}.asm"))
        if len(asm) != len(self.trad):
            fail(f"{name}.asm has {len(asm)} instructions, trad_opcode.txt {len(self.trad)} addresses")
        for k, ins in enumerate(asm):
            if ins != self.trad[k]:
                fail(f"address {k}: the .asm has '{ins}', trad_opcode.txt '{self.trad[k]}'")

        pos = sorted(k for k in self.text if k > 0)
        stripped = strip_comments([self.text[k] for k in pos])
        self.code = dict(self.text)
        self.code.update(zip(pos, stripped))
        self.funcs = functions(self.code)

    def start(self, s):
        """the first line of the statement line s ends"""
        while s > 1 and self.code.get(s - 1, "").rstrip()[-1:] in CONT:
            s -= 1
        return s

    def stmt(self, s):
        return " ".join(self.code.get(k, "") for k in range(self.start(s), s + 1))

    def violations(self, line=None):
        """The rules of part 2 that the map breaks: [(rule, address, why)]."""
        line = self.line if line is None else line
        io = isa_io()
        bad = []
        cur_f, prev, seen = None, None, set()
        for a in range(self.num_ins):
            for lb in self.labels[a]:
                if lb in self.funcs:
                    cur_f = lb
            if self.labels[a]:
                prev, seen = None, set()
            s = line[a]
            ins = self.trad[a].split()
            mn, arg = ins[0], (ins[1] if len(ins) > 1 else "")
            if s > 0:
                st = self.stmt(s)
                in_f = next((f for f, (b, e) in self.funcs.items() if b <= s <= e), None)
                if in_f != cur_f:
                    bad.append(("function", a, f"the address is in {cur_f or 'no function'}, line {s} in {in_f or 'no function'}"))
                if arg in self.var_of and not re.search(r"\b" + re.escape(self.var_of[arg]) + r"\b", st):
                    bad.append(("variable", a, f"line {s} does not name '{self.var_of[arg]}'"))
                if mn == "CAL" and arg in self.funcs and not re.search(r"\b" + re.escape(arg) + r"\s*\(", st):
                    bad.append(("call", a, f"line {s} does not call {arg}"))
                if mn in io and not re.search(r"\b\w+\s*\(", st):
                    bad.append(("io", a, f"line {s} calls nothing"))
                back_ok = HEAD.search(self.code.get(s, ""))
                if prev is not None and prev > 0 and s < prev and not back_ok and self.start(prev) > s:
                    bad.append(("order", a, f"line {s} after line {prev}"))
                if s in seen and prev != s and not back_ok and self.start(prev) > s:
                    bad.append(("together", a, f"line {s} again after line {prev}"))
                if mn in ("JMP", "JIZ"):
                    for pre, kw in JUMP_FAMILY:
                        if arg.startswith(pre):
                            if not re.search(r"\b(" + kw + r"|break|continue|if|switch|case|default)\b", st):
                                bad.append(("jump", a, f"line {s} is not a {kw.split('|')[0]}"))
                            break
            prev = s
            seen.add(s)
            if mn in ("JMP", "JIZ", "RET", "CAL"):
                prev, seen = None, set()
        return bad

    def show(self, a, s):
        return f"address {a} '{self.trad[a]}' on line {s}: {self.code.get(s, '?').strip()[:60]}"


_io = None


def isa_io():
    """the mnemonics isa.tsv marks as reading an input or writing an output"""
    global _io
    if _io is None:
        _io = set()
        path = os.path.join(HERE, "..", "Compilers", "common", "isa.tsv")
        for ln in lines(path):
            p = ln.split("\t")
            if ln.startswith("#") or len(p) < 8:
                continue
            if p[7] != "-":
                _io.add(p[0])
    return _io


def check_rules(prog):
    bad = prog.violations()
    if bad:
        for rule, a, why in bad[:10]:
            print(f"wave_map: {rule}: {prog.show(a, prog.line[a])} ({why})", file=sys.stderr)
        fail(f"{len(bad)} instructions on a line that cannot have produced them")


def neighbour(line):
    used = sorted({s for s in line if s > 0})
    pos = {s: i for i, s in enumerate(used)}
    return lambda s, step: used[pos[s] + step] if 0 <= pos[s] + step < len(used) else s


def selftest(prog):
    """Break the map as a compiler bug would; each break must be noticed."""
    line, n = prog.line, prog.num_ins
    g = neighbour(line)
    cases = []
    for step, way in ((1, "the line after"), (-1, "the line before")):
        for which in ("first", "last"):
            m = list(line)
            for a, s in enumerate(line):
                if s <= 0:
                    continue
                edge = (a == 0 or line[a - 1] != s) if which == "first" else (a + 1 == n or line[a + 1] != s)
                if edge:
                    m[a] = g(s, step)
            cases.append((f"the {which} instruction of every line on {way}", m))
        for mn in ("LOD", "SET", "OUT", "JMP", "JIZ", "CAL"):
            m = [g(s, step) if s > 0 and prog.trad[a].split()[0] == mn else s for a, s in enumerate(line)]
            cases.append((f"every {mn} on {way}", m))
    mid = n // 2
    cases.append(("the map shifted by one address", line[:mid] + [line[mid - 1]] + line[mid:-1]))
    missed = [what for what, m in cases if m != line and not prog.violations(m)]
    tried = sum(1 for _, m in cases if m != line)
    if missed:
        for what in missed:
            print(f"wave_map: selftest: not noticed: {what}", file=sys.stderr)
        fail(f"the rules missed {len(missed)} of {tried} broken maps")
    print(f"wave_map: selftest: {tried} broken maps, all noticed")


def check_vcd(prog, path):
    """linetabs is pc_<name>_mem.txt at the address valr2 shows, at every change."""
    ids, scope, want = {}, [], ("valr2", "linetabs")
    val, checked = {}, 0
    with open(path, encoding="utf-8", errors="replace") as f:
        for ln in f:
            if ln.startswith("$scope"):
                scope.append(ln.split()[2])
            elif ln.startswith("$upscope"):
                scope.pop()
            elif ln.startswith("$var"):
                p = ln.split()
                if p[4] in want and p[4] not in [v for v in ids.values()]:
                    ids[p[3]] = p[4]
            elif ln.startswith("$enddefinitions"):
                if sorted(ids.values()) != sorted(want):
                    fail(f"{path} lacks {', '.join(w for w in want if w not in ids.values())}")
                break
        for ln in f:
            if ln.startswith("#"):
                if len(val) == 2:
                    a, s = val["valr2"], val["linetabs"]
                    if a < prog.num_ins:
                        checked += 1
                        if s != prog.line[a]:
                            fail(f"at time {t}: the waveform shows line {s} for address {a} "
                                 f"('{prog.trad[a]}'), which is on line {prog.line[a]}")
                t = ln[1:].strip()
            elif ln[0] == "b":
                bits, i = ln[1:].split()
                if i in ids:
                    if "x" in bits or "z" in bits:
                        val.pop(ids[i], None)
                        continue
                    v = int(bits, 2)
                    if ids[i] == "linetabs" and len(bits) == 20 and bits[0] == "1":
                        v -= 1 << 20
                    val[ids[i]] = v
    if checked == 0:
        fail(f"{path}: no cycle to check")
    print(f"wave_map: the waveform's tracks agree at {checked} changes")


def main():
    sys.stdout.reconfigure(encoding="utf-8")     # the same map whatever the console
    args = sys.argv[1:]
    if len(args) < 2:
        fail("usage: wave_map.py <output folder> <name> [<messages file> | --vcd <file> | --selftest]")
    prog = Program(args[0], args[1])
    check_rules(prog)
    if args[2:3] == ["--selftest"]:
        selftest(prog)
        return
    if args[2:3] == ["--vcd"]:
        check_vcd(prog, args[3])
        return

    if len(args) == 3:
        print("## messages")
        for ln in lines(args[2]):
            print(ln.replace(os.path.abspath(args[0]), "$DIR"))
    print("## variables")
    for ln in prog.variables:
        print(ln)
    print("## listing")
    cur = None
    for k, ins in enumerate(prog.trad):
        s = prog.line[k] if k < prog.num_ins else None
        if k == prog.num_ins:
            print("// library macros, no source line")
        elif s != cur:
            print(f"// {s}: {prog.text[s].strip()}")
        cur = s
        print(f"{k} {ins}")


if __name__ == "__main__":
    main()
