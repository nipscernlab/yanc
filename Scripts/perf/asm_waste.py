# asm_waste.py <tests dir>: scan every <test>/golden.asm (e.g. Compilers/CMMComp/Tests)
# for instruction sequences a lean compiler would not emit. Patterns never cross a label (a jump target may
# arrive with a different acc), except the jump-threading ones.
import sys, re, glob, collections, os

root = sys.argv[1]
COMM = {"S_ADD": "ADD", "S_MLT": "MLT", "SF_ADD": "F_ADD", "SF_MLT": "F_MLT",
        "S_AND": "AND", "S_ORR": "ORR", "S_XOR": "XOR", "S_EQU": "EQU",
        "S_LAN": "LAN", "S_LOR": "LOR"}

def load(path):
    out = []   # (label or None, mnemonic, operand, lineno)
    for n, ln in enumerate(open(path, encoding="utf-8", errors="replace"), 1):
        ln = ln.split("//")[0].strip()
        if not ln or ln.startswith("#"):
            continue
        lab = None
        m = re.match(r"@(\S+)\s*(.*)", ln)
        if m:
            lab, ln = m.group(1), m.group(2).strip()
            if not ln or ln.startswith("#"):
                out.append((lab, "NOP", "", n)); continue
        p = ln.split(None, 1)
        out.append((lab, p[0], p[1].strip() if len(p) > 1 else "", n))
    return out

hits = collections.defaultdict(list)
reads = collections.Counter(); writes = collections.Counter()
for path in sorted(glob.glob(f"{root}/*/golden.asm")):
    name = os.path.basename(os.path.dirname(path))
    ins = load(path)
    labels = {l: k for k, (l, *_ ) in enumerate(ins) if l}
    for k in range(len(ins)):
        lab, op, arg, n = ins[k]
        if op in ("SET", "SET_P"): writes[(name, arg)] += 1
        elif arg and not arg[0].isdigit() and not arg.startswith("-"): reads[(name, arg)] += 1
        if k + 1 >= len(ins): continue
        l2, op2, a2, _ = ins[k + 1]
        where = f"{name}:{n}"
        if l2 is None:
            if op == "SET" and op2 == "LOD" and a2 == arg: hits["SET x; LOD x (reload of acc)"].append(where)
            if op == "LOD" and op2 == "SET" and a2 == arg: hits["LOD x; SET x (store back)"].append(where)
            if op in ("LOD",) and op2 == "LOD": hits["LOD; LOD (first load dead)"].append(where)
            if op == "PSH" and op2 == "POP": hits["PSH; POP"].append(where)
            if op == "PSH" and op2 == "LOD": hits["PSH; LOD (unfused P_LOD)"].append(where)
            if op == "P_LOD" and op2 in COMM: hits[f"P_LOD x; {op2} (-> {COMM[op2]} x)"].append(where)
            if op == "SET" and op2 == "SET" and a2 == arg: hits["SET x; SET x"].append(where)
            if op in ("ADD",) and arg == "0": hits["ADD 0"].append(where)
            if op in ("MLT",) and arg == "1": hits["MLT 1"].append(where)
        if op == "JMP" and l2 == arg: hits["JMP to the next instruction"].append(where)
        if op in ("JMP", "RET") and l2 is None and op2 != "NOP":
            hits["unreachable after JMP/RET"].append(where)
        if op in ("JMP", "JIZ") and arg in labels:
            t = ins[labels[arg]]
            if t[1] == "JMP": hits[f"{op} to a JMP (thread)"].append(where)
# words written and never read (per program), ignoring outputs/ports
for (name, v), c in writes.items():
    if reads[(name, v)] == 0: hits["variable written, never read"].append(f"{name}:{v}")

for k, v in sorted(hits.items(), key=lambda kv: -len(kv[1])):
    print(f"{len(v):5d}  {k}")
    print("       " + ", ".join(v[:8]) + (" ..." if len(v) > 8 else ""))
