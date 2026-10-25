"""Hand-written .asm for step 4b (array-unit.md): DOT and DOTB on random SAPHO floats,
and the words it must print, from the bit-level model of MAC A (mac.py).

Usage: python dot_gen.py <mac.py dir> <out dir> <FROUND> <seed>
Writes <out>/dot_t/dot_t.asm (no compiler involved) and <out>/expect.txt.
The arrays are #arrayd of type int holding the raw float words, so the
memory gets the exact bits (a float #arrayd would round decimal text)."""
import sys, os, random
sys.path.insert(0, sys.argv[1])
import mac

out, FR, seed = sys.argv[2], int(sys.argv[3]), int(sys.argv[4])
NB, M, E = 32, 23, 8
f = mac.fmt(M, E)
W = 2 * M + 8
rng = random.Random(seed)

K = 40
a = [mac.rnd_word(f, rng, -2, 2, True) for _ in range(K)]
b = [mac.rnd_word(f, rng, -2, 2, True) for _ in range(K)]
a[7] = mac.zero(f)                                  # a zero term, skipped by the MAC

def dot(xs, ys):
    (V, L), bad = mac.var_lazy(f, mac.products(f, xs, ys), W, FR == 2)
    assert not bad
    if V == 0:
        return mac.zero(f)
    return mac.enc(f, 1 if V < 0 else 0, L, abs(V))

def s32(w):
    return w - (1 << 32) if w >= 1 << 31 else w

exp = []
asm = [
    "#PRNAME dot_t", "#NUBITS 32", "#NBMANT 23", "#NBEXPO 8",
    "#NUIOIN 1", "#NUIOOU 1", f"#FROUND {FR}",
    f"#arrayd a 1 {K} {K} " + " ".join(str(w) for w in a),
    f"#arrayd b 1 {K} {K} " + " ".join(str(w) for w in b),
]
# 1. the whole vectors, N a constant
asm += ["LEA a", "P_LEA b", f"DOT {K}", "OUT 0"]; exp.append(s32(dot(a, b)))
# 2. the stack is balanced after DOT: 7 + 5
asm += ["LOD 7", "P_LOD 5", "S_ADD", "OUT 0"]; exp.append(12)
# 3. pointers into the middle: a[3..13] . b[10..20], N in a variable
asm += ["LOD 11", "SET n", "LEA a", "ADD 3", "PSH", "LEA b", "ADD 10", "DOT n", "OUT 0"]
exp.append(s32(dot(a[3:14], b[10:21])))
# 4. N = 0: the zero word
asm += ["LEA a", "P_LEA b", "DOT 0", "OUT 0"]; exp.append(s32(mac.zero(f)))
# 5. one term
asm += ["LEA a", "P_LEA b", "DOT 1", "OUT 0"]; exp.append(s32(dot(a[:1], b[:1])))
# 6. the core still reads the array memory after DOT: a[5] by LDI
asm += ["LOD 5", "LDI a", "OUT 0"]; exp.append(s32(a[5]))
# 7. a DOT right after another, its result used by the ALU: <a|b> + <b|a> as float
asm += ["LEA a", "P_LEA b", f"DOT {K}", "PSH", "LEA b", "P_LEA a", f"DOT {K}", "SF_ADD", "OUT 0"]
exp.append(None)                                    # checked apart: twice case 1, by the float adder
# DOTB b: N in the acc (P_LOD N pushes pointer a), b the array named
# 8. the whole vectors
asm += ["LEA a", f"P_LOD {K}", "DOTB b", "OUT 0"]; exp.append(s32(dot(a, b)))
# 9. pointer a into the middle, N in a variable (11, set in case 3)
asm += ["LEA a", "ADD 3", "P_LOD n", "DOTB b", "OUT 0"]; exp.append(s32(dot(a[3:14], b[:11])))
# 10. N = 0 and 11. N = 1
asm += ["LEA a", "P_LOD 0", "DOTB b", "OUT 0"]; exp.append(s32(mac.zero(f)))
asm += ["LEA a", "P_LOD 1", "DOTB b", "OUT 0"]; exp.append(s32(dot(a[:1], b[:1])))
# 12. the stack is balanced after DOTB
asm += ["LOD 7", "P_LOD 5", "S_ADD", "OUT 0"]; exp.append(12)
# 13. the result reaches the data memory (SET right after DOT, and after DOTB)
asm += ["LEA a", "P_LEA b", f"DOT {K}", "SET r", "LOD 0", "LOD r", "OUT 0"]; exp.append(s32(dot(a, b)))
asm += ["LEA a", f"P_LOD {K}", "DOTB b", "SET r", "LOD 0", "LOD r", "OUT 0"]; exp.append(s32(dot(a, b)))
# 14. DOTB right after DOT, the two results summed by the ALU: as case 7
asm += ["LEA a", "P_LEA b", f"DOT {K}", "PSH", "LEA b", f"P_LOD {K}", "DOTB a", "SF_ADD", "OUT 0"]
exp.append(None)
asm += ["@fim JMP fim"]

d = os.path.join(out, "dot_t")
os.makedirs(d, exist_ok=True)
open(os.path.join(d, "dot_t.asm"), "w", newline="\n").write("\n".join(asm) + "\n")
open(os.path.join(out, "expect.txt"), "w", newline="\n").write("\n".join("-" if e is None else str(e) for e in exp) + "\n")
print(f"{len(asm)} lines, {len(exp)} outputs")
