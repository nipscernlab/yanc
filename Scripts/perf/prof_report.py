# prof_report.py <program dir> <name>: cycles per function, per source line and
# per address, from prof.txt (PC -> cycles) written by Scripts/perf/cycles.sh prof.
import sys, re, collections
d, v = sys.argv[1], sys.argv[2]
asm = open(f"{d}/proc/Software/{v}.asm").read().splitlines()

# address -> (instruction text, enclosing function label)
ins, func, cur = [], [], "?"
for ln in asm:
    ln = ln.strip()
    if not ln or ln.startswith("#"):
        continue
    m = re.match(r"@(\S+)\s+(.*)", ln)
    if m:
        lab, ln = m.group(1), m.group(2)
        if not re.match(r"L[a-z_]*\d+$", lab):   # branch targets are not functions
            cur = lab
        ln = f"@{lab} {ln}"
    ins.append(ln); func.append(cur)

src = {}
for ln in open(f"{d}/trad_cmm.txt", encoding="utf-8", errors="replace"):
    k, _, t = ln.rstrip("\n").partition(" ")
    src[int(k)] = t
line = []
for b in open(f"{d}/pc_{v}_mem.txt"):
    x = int(b.strip(), 2)
    line.append(x - (1 << 20) if x >= 1 << 19 else x)

prof = {}
for ln in open(f"{d}/prof.txt"):
    a, c = ln.split(); prof[int(a)] = int(c)
tot = sum(prof.values())

def pct(c): return f"{c:8d} {100*c/tot:5.1f}%"
print(f"instructions {len(ins)}, cycles {tot}\n")

byf = collections.Counter()
for a, c in prof.items():
    byf[func[a] if a < len(func) else "?"] += c
print("== by function")
for f, c in byf.most_common(20): print(pct(c), f)

byl = collections.Counter()
for a, c in prof.items():
    byl[line[a] if a < len(line) else -99] += c
print("\n== by source line")
for l, c in byl.most_common(25): print(pct(c), f"{l:5d}", src.get(l, "")[:90])

print("\n== hottest addresses")
for a, c in sorted(prof.items(), key=lambda kv: -kv[1])[:40]:
    print(pct(c), f"{a:5d}", f"{func[a]:28s}" if a < len(func) else "", ins[a] if a < len(ins) else "")
