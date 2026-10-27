# Names in the `.asm`: one owner per name (2026-10-09)

Commits 96fcdaf (cppcomp) and d8f4eeb (cmmcomp), in `[Unreleased]` of the
CHANGELOG. This note records what a user can no longer write and why, so the
manual can say it (issue #40).

## The problem

Both compilers turn every source name into a word of the `.asm`, and the
assembler has a single table for data and one for labels. Nothing checked
that the `.asm` spelling of a user name was free. Measured in simulation,
code up to 607bff4:

| Source | What happened |
|---|---|
| `int ADD;` | C+- dropped the `SET ADD`; C++ printed 0 |
| global `main_x` and local `x` of `main` | C++: one word for both, silently. C+- refused, with a misleading message |
| global `aux_var` or `pow_b` (C+-) | overwritten by the post-increment or by `pow()` |
| global `Lwh1` (C+-) | the loop label: no output at all |
| global `udm_r` (C++) | took the remainder of every unsigned division |
| `__fp` (C++) | the frame pointer |

## The rules

1. **A name whose `.asm` spelling is an instruction is refused.** The list
   comes from the ISA table in `Compilers/common/asm_share.c`
   (`asm_is_mnemonic()`), which `Scripts/check_isa.py` keeps equal to the
   assembler, so a new instruction is reserved without touching the
   compilers. The check is on the `.asm` spelling: a local `ADD` of `main`
   is `main_ADD` and is legal; a local `ADD` of a function `S` is `S_ADD`,
   an instruction, and is refused.
2. **Every name the compiler makes starts with `__`**: loop and switch
   labels, `aux_var`, the stdlib's scratch words, the `float_*` routines,
   cppcomp's runtime routines and temporaries. C+- refuses a user name
   starting with `__`; C++ refuses any name with `__` in it, as the C++
   standard reserves those.
3. **A name has one owner.** A name whose `.asm` spelling already belongs to
   a global, to a local of another function, to a function, or to the
   imaginary part of a `comp` (`z` keeps it in `z_i`) is refused, and the
   message names both owners. C++ also refuses two overloads that map to
   the same `.asm` name.

Reserved words: a function may not be called `fim` (the program ends at the
label `@fim`, which Aurora reads), `global` (marks the globals in the
simulation log) or `num_ins` (a key of that log). A *variable* named `fim`
stays legal: data and labels are separate tables in asmcomp.

## Evidence

Negative fixtures, one per case, in `Compilers/CMMComp/NegTests/names_*.cmm`
and `Compilers/CPPComp/NegTests/names_*.cpp` (the regress checks each is
refused with the right message). Positive: `cmm_names` printed
`7 1969225728 1983905800 4 5 9` with the old compiler, `7 2 10 4 5 9` now;
`test90` printed `udm_r = 1` before. The 71 C+- golden `.asm` that changed
differ only by the `__` prefix. Full regress on 2026-10-10: 202 of 203; the one
failure was the `sapho_all` block check, which did not know `LINKAS`
(fixed in 3a96072, the check rerun alone passes).

## For the documentation (issue #40)

The user-facing text says: a name may not be spelt like an instruction of
the processor once the compiler prefixes it (a local `x` of `f` becomes
`f_x`); names starting with `__` belong to the compiler (C++: any name
containing `__`); a `comp z` also owns `z_i`; two names that become the same
`.asm` word are an error, and the message says which two; a function may not
be called `fim`, `global` or `num_ins`. Old programs that broke one of these
rules compiled into a wrong program; they now stop with an error naming the
line.
