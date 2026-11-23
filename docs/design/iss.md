# The instruction-set simulator and the random differential test (#32)

The goldens hold what one example happens to exercise, and a golden blessed
with a wrong result holds it as firmly as a right one. The ALU already has a
check that does not depend on any golden: `Scripts/hw/tb_ula.v` drives every
operation and compares the result with a reference derived from what the
operation means. Issue #32 asked for the same for the rest of the processor:
the decoder, the jumps, the calls, the stacks, the memory, the I/O and the
interrupt.

## What was built

Three pieces, in `Scripts/iss/`:

- **`sapho_iss.c`, the instruction-set simulator.** It runs the program
  asmcomp built, from the two memory images and the parameters of the
  generated top (`<proc>.v`), and prints one line per event, cycle by cycle:
  the instruction address fetched (`F`), every data-memory write (`W`),
  input read (`I`) and output (`O`), and the depth of the data stack (`D`)
  and of the instruction stack (`R`) when it changes.
- **`tb_iss.v`, the RTL side.** A testbench that runs the generated processor
  and prints the same lines from the top's simulation wires (`pc_sim_val`,
  `mem_wr`, `mem_addr_wr`), its I/O and the stacks' counters (`fl_cnt`). Both sides read the input ports from
  the same function of the read count and the port, so a program can read
  any number of words.
- **`rand_asm.py`, the generator.** A seed fixes a whole program: the format
  (32/23/8, 23/16/6 or 16/10/5), `#FROUND`, `#NUGAIN`, the data-stack depth
  (10 words, or 32, where the stack reads its top from a register, #170), and
  a main program of straight-line runs, ifs, if/elses, counted loops, calls
  and divisions, with up to four subroutines.

`iss_diff.sh` puts them together: for each seed it writes the program,
builds it with `yanc`, runs it on the ISS and under Icarus, and compares the
two traces line by line. Half the seeds are `#ITRAD` programs, run with the
interrupt pin raised in one or two cycles the seed picks; every other one of
those aims its first pulse at a cycle with `INC` in decode or, if there is
none, at one where the data stack holds words, where a pulse shows the most. `iss_mutants.sh`
checks the check: it breaks a copy of `SAPHO/` in one place at a time and
expects `iss_diff.sh` to fail.

## Decisions

**The ISS is written from the instruction table, not from the Verilog.**
What each instruction does comes from the columns of
`Compilers/common/isa.tsv` (accumulator, stack, operand, flow, I/O). What the
ALU computes is the meaning `tb_ula.v` checks the ALU against, its `ref_*`
functions ported to C. Nothing is copied from `instr_dec.v` or `core.v`, so a
decoder row or a core that does something else shows up as a difference.
This is also what the table's header asked for: its effect columns were "a
careful reading, not a measurement" until something ran them.

**The comparison is cycle by cycle, not only of the results.** The processor
runs one instruction a cycle, so the ISS can follow its timing: the jump
decided in decode, with no slot after it; the word at address 0 fetched on
the reset edges and run in cycle 0; `OUT` seen a cycle after its decode
(io_ctrl registers it); the write of `INC` a cycle after its decode (#203);
`QUO`, `REM` and `F_QUO` reading the division started STG-1 cycles before
(#113). Comparing the fetch address of every cycle catches a wrong jump at
once, where the results alone would show it many cycles later or not at all.

**What a program does not show, the trace does.** A word left on a stack,
or a stack not emptied by an interrupt, changes nothing a balanced program
reads; so the stacks' depths are in the trace. A result left in the
accumulator at the end of a block is lost to the load that starts the next
one; so the generator stores it there, most of the time.

**The interrupt as core.v gives it.** In the cycle the pin is high, the
instruction in decode runs, the fetch goes to `ITRADD` and both stacks empty,
with the push or pop of that instruction (#188); an `INC` in decode loses its
write. The ISS does the same, and the generator starts the main program with
a load, so a restart never reads an accumulator the restart left.

**Only where the processor is defined.** The generator types every value, int
or float, and an operation only takes the type it is for, so a float at
`#FROUND` 1 or 2 is always normalized or the canonical zero, the set
`tb_ula.v` checks. An `F_DIV` at `#FROUND` 0 divides by a normalized
constant. Array indices stay inside the arrays, the stacks inside their
depths, and every block leaves the data stack as it found it, so the two
paths of an if meet with the same stack. Each division is written as the
compilers write it, the division, its slots and the read, with random single
instructions in the slots. Where a program breaks one of these, the ISS stops
with an error (a stack overflow, an address past the memory, a read with no
division before it) instead of guessing.

**The simulator is built by the test, not by the Makefile.** It is a test
harness, like cmmcomp's unit test in `Scripts/regress.sh`: `iss_diff.sh`
compiles it with gcc, and it does not ship.

## What was measured (2026-11-23)

- Seeds 1 to 40, the ones `Scripts/regress.sh` runs: all 40 the same on the
  ISS and the RTL, 1 min 42 s for the 40 (a Windows 11 laptop, Icarus under
  MSYS2). Those programs run 297 to 1053 cycles without the interrupt.
- The first 60 seeds use every mnemonic of `isa.tsv` but `TOM` and `CAD`,
  and every format with every `#FROUND` level.
- `iss_mutants.sh`, 13 mutants, 12 seeds each, before the stack depths, the
  stored results and the aimed pulses: 10 caught, 3 survived. "The
  interrupt keeps the data stack" and "the interrupt keeps the INC write"
  changed nothing the trace had (a word left on the stack, never read; a
  pulse that never fell on an INC); "F_SU1 runs as F_SU2" changed results
  the next load threw away. After the three changes above, the three are
  caught: the first two by seed 2 from cycle 65 (its pulse at cycle 64, on
  an INC), the third by seed 4 from cycle 356. The whole battery again
  after the changes: 13 caught, none survived, each by seed 1, 2 or 4.

## Found on the way

asmcomp drops a line whose mnemonic it does not know, without an error or a
warning. An early version of the generator wrote `P_XPO_M`, which the
instruction set does not have; the program assembled, the line was missing
from `trad_opcode.txt`, and "the processor is good to go". The generator now
checks every mnemonic it writes against `isa.tsv`. asmcomp itself is not
changed here.

## Not done

- **Verilator.** The issue asks for the RTL under Icarus and Verilator.
  `tb_iss.v` verilates (Verilator 5.050, `--binary --timing`), but on the
  machine it was tried on the link fails on the C++ library, the problem the regression goes
  around with a PowerShell step for its C++ phase. Only Icarus runs.
- **`TOM` and `CAD`.** They need a partner processor; neither the ISS nor
  the generator has one.
- **What the processor leaves undefined:** a stack past its depth, an
  address past the memory, a float that is not normalized at `#FROUND` 1 or
  2, an `F_DIV` at level 0 by a divisor that is not normalized. These are
  avoided, not checked.
