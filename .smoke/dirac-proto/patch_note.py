# Adds the loop prototype to docs/design/dirac-expressions.md (2026-10-25/26).
p = 'C:/nipscern/yanc-dirac/docs/design/dirac-expressions.md'
s = open(p, encoding='utf-8', newline='').read().replace('\r\n', '\n')
R = []

R.append(("""Status: design, 2026-10-25. No code yet. Decision (Luciano, 2026-10-25): the""",
"""Status: step (a) of section 5 done (commit 20112ab, branch
`lucianomaf/dirac-parser`: the grammar, the tree and the shapes, the code
still unrolled); step (b), the loops, redesigned in section 6 and
prototyped, not in the repository. Decision (Luciano, 2026-10-25): the"""))

R.append(("""### 3.4 Code generation

Three ways were weighed:""",
"""### 3.4 Code generation

**Superseded by section 6** (Luciano, 2026-10-25): the loops are to take
the fewest instructions possible, with small hardware if needed, so L1
below (the generic walker, 12 to 20 instructions an element) is replaced by
a generator of its own for the Dirac loops. The rest of this section (the
requirements on scalars, aliasing, the temporary area) still holds.

Three ways were weighed:"""))

R.append(("""2. Code generation L1: lowering to the loop nodes of the tree; the
   `exec_*` generators go, except `exec_shift`.""",
"""2. Code generation L1: lowering to the loop nodes of the tree; the
   `exec_*` generators go, except `exec_shift`. **Reopened the same day:
   section 6.**"""))

R.append(("""## 6. For the documentation (item 21 in TODO.md)""",
"""## 6. Loops with small hardware (prototype, 2026-10-25 and 26)

Luciano asked for the fewest instructions in the loops, adding
instructions if needed, within these limits (2026-10-25): **no memory of
its own** (no second port, not the array unit of PR #19: that is the
coprocessor mode, which a parameter will choose instead of this), only
**small hardware** (registers, counters, multiplexers), every block in a
`generate` and built only when the program uses it, the **logic added
measured**, and **no path longer than today's path through the float
adder**: the clock must not drop.

Everything here is a prototype outside the repository, in
`.smoke/dirac-proto/` of this worktree (ignored by git): a modified copy of
`SAPHO/` (`proto/SAPHO`, made by `proto/patch_core.py`), a bench
(`proto/bench.py`) and the fit scripts (`proto/map.sh`, `proto/fit.sh`,
`proto/sta.sh`). No compiler emits the new instructions yet: the bench
writes them by hand.

### 6.1 What the pipeline allows (fact)

Read in `SAPHO/core.v`, `SAPHO/instr_dec.v` and `SAPHO/processor.v`:

- every instruction takes one clock, a jump or `JIZ` included: the jump is
  decided in the decode and the fetch takes the target in the same cycle
  (`prefetch`, `core.v` 95-157);
- the data memory has one read port and one write port, with their own
  addresses (`mem_addr_rd`, `mem_addr_wr`);
- an instruction is decoded in cycle t (its read address leaves then), the
  word read arrives in t+1, the ALU works in t+1 (the ALU operation is
  registered in `instr_dec`) and the acc is written at the end of t+1;
- a write happens in the decode cycle and stores `ula_out`, which in that
  cycle is the result of the **previous** instruction: that is how `SET`
  works;
- `LDI base` reads `mem[base + ula_fast]`; `STI base` writes the acc to
  `mem[base + stack top]` and pops (`mem_ctrl`, `core.v` 448-481).

One ALU means one operation a clock. With one read port, an element of an
inner product costs at least three instructions (load, multiply, add),
which is what today's unrolled code takes. The write port is idle in most
instructions: that is the room left.

### 6.2 Software only, measured first

Before the hardware, the best loops with today's instructions, written by
hand, simulated, the same bits as the compiler's (inner product of 16, 6
turns; `y # |M|v⟩` of 16x16, 3 turns):

| code | instructions an element | words | clocks a turn |
|---|---|---|---|
| inner product, the compiler's `for` loop | 12 | 57 | 199 |
| sum on the stack, negative index, `JIZ`+`JMP` | 10 | 55 | 168 |
| the same, unrolled by 5 | 6.8 | 79 | 120 |
| `\\|M\\|v⟩`, unrolled today | 3 | 826 | 941 |
| `\\|M\\|v⟩`, rows in a loop, columns unrolled (`LDI M+j`) | 4.6 | 137 | 1358 |

The negative index (from -N to -1, the base at the end of the array) lets
`JIZ` close the loop; the column unrolling makes the code grow with N, not
N². Both need an assembler alias `LDI_V`/`STI_V` (base plus a constant), no
hardware.

### 6.3 The design

Three blocks, each in a `generate` on a parameter asmcomp would set from
the program (`NPTR`, `RPT`):

1. **Four pointers P0 to P3**, each as wide as a data address. With
   `NPTR > 0`, the top bit of a memory instruction's operand selects the
   pointer mode: `[1:0]` k, the pointer of the instruction's own access,
   `[3:2]` its change after the access (01 +1, 10 -1). asmcomp widens the
   operand by one bit, so no other operand reaches that bit (an invariant
   of the assembler, not checked by the hardware). `SETP k a` (opcode 110)
   loads pointer k with the address a, an immediate (every Dirac pointer
   starts at a constant address).
2. **A parallel store of the previous result**: in pointer mode, `[4]` also
   stores `ula_out` (the result of the previous instruction, what `SET`
   stores) through pointer `[6:5]` j, changed by `[8:7]`. It uses the idle
   write port. The compiler pipelines the loop: the load of element i
   stores the result of element i-1, with one instruction before and one
   after the loop.
3. **A zero-overhead loop**: `RPT L n` (opcode 111) runs the L instructions
   after it n times with no jump instruction. Three registers (the first
   address of the body, the address after it, the turns left) and a flag;
   when the PC is about to fetch the address after the body with turns
   left, the fetch takes the first address. One level; the outer loop of
   a matrix stays in software. No jump inside the body (a contract of the
   compiler). The operand holds L in 3 bits and n below it.

Opcodes 108 and 109 stay for `DOT`/`DOB` of PR #19. Every select reads
registers and the instruction word, never the ALU: the pointer feeds the
**base** input of the `LDI`/`STI` adder, whose other input (`ula_fast`)
comes much later, and the `RPT` choice is ready before the decode decides
a jump (`pc_load` stays the last select of the fetch address).

### 6.4 Measured in simulation

`proto/bench.py`: each form compiled by cmmcomp as today (the reference),
the unrolled stretch replaced by the hand-written loop, both simulated on
the same inputs (2 turns, N = 16), the output compared bit for bit, the
clocks of the stretch counted by a probe on the fetch address. **All 11
give the same bits.** Today's element order is kept, the transposed outer
product included, so that the bits can be compared.

| form | words today -> prototype | clocks today -> prototype | the loop body |
|---|---|---|---|
| `⟨a\\|b⟩` | 48 -> 9 | 48 -> 51 | `P_LOD @0+; F_MLT @1+; SF_ADD` |
| `\\|M\\|v⟩` | 768 -> 18 | 768 -> 884 | the same, a row a turn of a software loop |
| `c\\|v⟩` | 48 -> 8 | 48 -> 36 | `LOD @0+ >1+; F_MLT c` |
| `\\|w⟩ + e\\|K⟩` (in place) | 64 -> 11 | 64 -> 53 | `LOD @0+ >2+; F_MLT e; F_ADD @1+` |
| `\\|P\\| - \\|K⟩⟨Px\\|`, `F_NEG; F_ADD` | 1280 -> 24 | 1280 -> 1189 | 4 a element |
| `\\|P\\| - \\|K⟩⟨Px\\|`, `F_SU2` | 1280 -> 22 | 1280 -> 933 | `LOD @0+ >2+; F_MLT t; F_SU2 @1+` |
| `c\\|P\\|` | 768 -> 8 | 768 -> 516 | `LOD @0+ >1+; F_MLT c` |
| `\\|a⟩⟨b\\|` | 768 -> 19 | 768 -> 676 | `LOD @0+ >2+; F_MLT t` |
| `\\|0⟩` | 17 -> 4 | 17 -> 19 | `SET @0+` |
| `c -> \\|a⟩` | 32 -> 8 | 32 -> 21 | `LOD @0- >1-` |
| `out(0, c\\|a⟩)` | 64 -> 6 | 64 -> 66 | `LOD @0+; F_MLT c; F2I; OUT 0` |

`F_SU2` is memory minus acc (`ula.v` 1626-1627: `F_SU1` turns the sign of
the memory input, `F_SU2` that of the acc); `F_SU1` gave other bits. With
`F_SU2` the bits are those of today's `F_NEG; F_ADD`, at `#FROUND 0`; not
checked at levels 1 and 2. `|M|v⟩` is 15 % slower than unrolled: 7
instructions of software loop a row; a second level of `RPT` would remove
them (not designed). The prototype core with `NPTR = 0` and `RPT = 0`
gives the output and the clocks of the original (checked on `|M|v⟩`), and
`sapho_all` with the blocks on and unused gives its golden output.

### 6.5 Measured in Quartus

`sapho_all` (every block in, `#FROUND 2`) against `sapho_ptr`, the same
program with the operand widened from 11 to 12 bits, `NPTR = 4`, `RPT = 1`,
on the DE10-Nano (5CSEBA6U23I7), `fmax.sh`, seed 1, 2026-10-25:

| | `sapho_all` | `sapho_ptr` | difference |
|---|---|---|---|
| Fmax | 32.11 MHz | 31.82 MHz | -0.9 % |
| logic (fit) | 3545 ALMs | 3643 ALMs | +98 (+2.8 %) |
| logic (synthesis estimate) | 3378 ALMs | 3467 ALMs | +89 (+2.6 %) |
| registers | 1193 | 1253 | +60 |
| program memory bits | 21,054 | 22,109 | +1055 (one bit a word) |

The worst path is the same chain in both: the acc or the ALU input, the
exponent alignment, the float adder, the normaliser, the ALU output mux,
the stack; 30.88 ns of data path in `sapho_all`, 30.76 ns in `sapho_ptr`.
None of the 3000 worst paths of `sapho_ptr` (down to -20.6 ns of slack at
the 10 ns asked) touches the new blocks. Asked of `quartus_sta` on the same
fit, the worst paths through them, at 10 ns:

| path | slack | about |
|---|---|---|
| out of the pointers, to the data memory address | +1.7 ns | 8 ns |
| into the pointers (from the instruction memory) | -2.0 ns | 12 ns |
| out of the `RPT` registers, to the instruction address | +1.6 ns | 8 ns |
| into the `RPT` turn counter | -14.7 ns | 25 ns |

The last one came from the counter counting a turn only without a jump
(`~pc_load`), which pulled the `JIZ` decision, made from the ALU, into the
counter. The contract already forbids a jump in the body, so the condition
was dropped (2026-10-26): the 11 forms still give the same bits; **its
path was not timed again**.

The -0.9 % is one seed each: `fmax.sh` warns that a fit can be off by
several per cent from one seed to another. Not yet shown either way.

### 6.6 Open, in order

1. Two more seeds of each (`proto/fit.sh` with `SEED=2`, `SEED=3`, a fit
   about 50 minutes, one at a time) with the counter fixed, and the path
   into the `RPT` counter timed again (`proto/sta.sh`).
2. Luciano decides: these instructions, and then decision 2 of section 5
   becomes a generator of its own for the Dirac loops.
3. In the repository, if yes: `SAPHO/core.v` and `processor.v` from the
   prototype (the diff to him first, as for every HDL change), the rows of
   `isa.tsv` and of the copies `check_isa.py` holds to it, asmcomp (the
   mnemonics `SETP` and `RPT`, the pointer syntax, the widened operand,
   `NPTR`/`RPT` from the program), the manual's instruction reference.
4. Open in the design: a second `RPT` level for the rows of a matrix;
   `comp` (two words an element); whether the pointers also serve
   ordinary C± arrays in loops (`for` with `a[k]`), which would speed up
   every program, not only the Dirac notation.

## 7. For the documentation (item 21 in TODO.md)"""))

for a, b in R:
    assert s.count(a) == 1, a[:60]
    s = s.replace(a, b)
open(p, 'w', encoding='utf-8', newline='').write(s.replace('\n', '\r\n'))
print('ok')
