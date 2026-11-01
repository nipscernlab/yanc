// ****************************************************************************
// asm_share -- scalars whose lives never overlap share one data word ---------
// ****************************************************************************
//
// Reads a finished .asm, works out for every instruction which variables still
// hold a value that will be read later (liveness, on the whole program), and
// gives variables that are never alive at the same time one data word: for
// each name that moves into another's word it writes "#SHARE <name> <home>"
// ahead of the code, and asmcomp gives <name> the address of <home>.
// The code itself is not touched -- every instruction keeps its own names, so
// the listing reads as written, the pc_*_mem.txt line map stays valid, and
// asmcomp still shows each variable in the waveform on its own (it follows a
// shared variable by the instructions that write it, not by its address).
// History: CHANGELOG v5.5 (C+-) and v5.6 (C++).
//
// The program is analysed as one flow graph: a RET returns to the instruction
// after every CAL in the program, and #ITRAD adds an edge from every
// instruction to the interrupt point (the interrupt is a forced jump there).
// Names that are touched through an address (arrays, _V offsets, LEA,
// indirect loads and stores) never share. A variable that is read
// before any write keeps its own initial value: it is the name its group
// keeps (<home>). On anything the pass does not understand (an unknown mnemonic, a
// jump to a label that is not there) it leaves the file untouched.
// ****************************************************************************

#ifndef ASM_SHARE_H
#define ASM_SHARE_H

// Rewrites asm_path in place. Returns the number of data words saved (0 when
// nothing could be shared or the file was left untouched), -1 on an I/O error.
int asm_share(const char *asm_path);

// 1 when name is an instruction of the assembly (the table above, held to
// isa.tsv by check_isa.py): the assembler reads such a word as the
// instruction, so neither compiler lets a user name become one.
int asm_is_mnemonic(const char *name);

// ****************************************************************************
// asm_reach -- code no path gets to is not in the program, nor its hardware --
// ****************************************************************************
//
// asmcomp builds an operator for every opcode the .asm holds, so a function
// nobody calls (or a library header's inline function) would still pay for
// its divider. This pass walks the program from where the PC can start --
// address 0, the #ITRAD interrupt point, the #TOAQUI mark and @fim -- along
// fall-through, JMP, both ways of JIZ and CAL (then the instruction after
// it), and drops every instruction it never reaches, and every #array /
// #arrays that only those used. The ISA has no indirect jump, so the walk is
// exact: C++'s function pointers and virtual calls are chains of direct CALs.
//
// A front end may put straight-line code that exists only for some routines
// between "#IFLIVE <label> [<label> ...]" and "#ENDLIVE" (cppcomp's heap
// set-up in main, for malloc; its recursion stack, for the recursive
// functions): the block stays only if one of the labels is reached. The
// marker lines always leave the file.
//
// A JMP or JIZ to an unconditional JMP first goes straight to where that one
// goes. A JMP whose target is the instruction that follows it anyway (a break
// at the end of a loop body, an if whose else is empty) leaves too. Neither
// goes through or drops a JMP that #ITRAD or #TOAQUI marks.
//
// pc_path (pc_<proc>_mem.txt, one line per instruction of the program part,
// may be NULL) loses the lines of the dropped instructions in lockstep. A
// label that a kept jump needs moves to the next kept instruction. On anything
// the pass does not understand the code stays whole (only the markers leave).
//
// Run it BEFORE asm_share, so dead code does not take part in the liveness.
// Returns how many dropped instructions the pc map covered (the caller's
// instruction count drops by that much), -1 on an I/O error.
int asm_reach(const char *asm_path, const char *pc_path);

// ****************************************************************************
// asm_unread -- a variable written and never read is not in the program ------
// ****************************************************************************
//
// A scalar that some SET or SET_P writes and no instruction reads leaves the
// program: its SETs go (their labels move to the next instruction), a SET_P
// becomes the POP it also is, and with no instruction naming it the variable
// is not in the processor, its memory or the waveform either. A name an
// address touches (an array, a _V offset, LEA, an indirect load or store) or
// a directive names is read. The two halves of a comp, x and x_i, go together
// or not at all: asmcomp joins them into comp_x for the waveform. On anything
// the pass does not understand it leaves the file untouched. keep (may be
// NULL) names the variables the user asked to keep anyway, for the waveform
// (cmmcomp's #WAVE): a name it returns nonzero for counts as read.
//
// Run it after asm_reach and before asm_share. pc_path (pc_<proc>_mem.txt,
// may be NULL) loses the lines of the dropped SETs in lockstep. gone (may be
// NULL) is called with the name of each variable that left, a comp by its
// real half. Returns how many dropped instructions the pc map covered (the
// caller's instruction count drops by that much), -1 on an I/O error.
int asm_unread(const char *asm_path, const char *pc_path, void (*gone)(const char *name),
               int (*keep)(const char *name));

// ****************************************************************************
// asm_callarg -- no argument for a function that never reads it --------------
// ****************************************************************************
//
// A function whose first instruction writes the acc without reading it (LOD,
// a load from memory, ...) never sees the argument its caller left in the
// acc: once asm_unread has taken out the SET of a parameter nothing reads, the
// instructions before each CAL of that function that change only the acc
// compute a value nobody uses, and they leave (their labels move to the next
// instruction). Only for a function no JMP or JIZ reaches and nothing runs
// into from the instruction above it. An instruction with any other effect (a
// push, a pop, I/O, a call, a link, a divider) stays, and so does everything
// before it. On a #ITRAD, or on anything the pass does not understand, it
// leaves the file untouched.
//
// Run it after asm_unread. pc_path (may be NULL) loses the lines of the
// dropped instructions. Returns how many dropped instructions the pc map
// covered, -1 on an I/O error.
int asm_callarg(const char *asm_path, const char *pc_path);

// ****************************************************************************
// asm_divk -- a division by a constant, or the product by its reciprocal ----
// ****************************************************************************
//
// cmmcomp emits a float division by a literal that is not a power of two in
// both forms: the division between #DIVK and #MULK, the product by the
// reciprocal between #MULK and #ENDK. The product is one word and needs no
// divider, but rounds the reciprocal: it is kept only when no float division
// outside the blocks (by a variable, of a comp, in a library routine) puts
// the divider in the processor anyway (Luciano, 2026-10-31, issue #69).
// The markers leave; pc_path loses the lines of the dropped instructions.
// Run it after asm_reach (code no path reaches needs no divider) and before
// asm_divseq. Returns how many dropped instructions the pc map covered, -1
// on an I/O error.
int asm_divk(const char *asm_path, const char *pc_path);

// ****************************************************************************
// asm_divseq -- a division takes as many words as its divider has stages -----
// ****************************************************************************
//
// The dividers (SAPHO/ula.v) are cut by registers into STG stages: the
// result of a division started at cycle t is at the end of the array at
// t+STG-1, while the ALU executes the division's last word. So every DIV /
// S_DIV becomes `DIV x; NOP; NOP; NOP; QUO` (ula_idiv, 5 stages), MOD / S_MOD
// `...; REM` and F_DIV / SF_DIV `F_DIV x; NOP; NOP; F_QUO` (ula_fdiv, 4
// stages) (QUO, REM, F_QUO: aliases of the division's own opcode, no
// operand; the read is always the memory form, so a stack form is not popped
// twice). A division already followed by its NOPs and its read is left
// alone. Labels stay on the line they were on, so a jump still lands on the
// division, never inside the sequence.
//
// pc_path (pc_<proc>_mem.txt, may be NULL) repeats a division's source line
// for its new words. Run it after asm_reach (dead code needs no
// sequence) and before asm_share. Returns how many words were added inside
// the part of the program the pc map covers (the caller's instruction count
// grows by that much), -1 on an I/O error.
int asm_divseq(const char *asm_path, const char *pc_path);

// The words of a division: the STG of its divider in SAPHO/ula.v (ula_idiv,
// ula_fdiv); change both together.
#define ASM_DIV_WORDS_INT 5                       // DIV / MOD
#define ASM_DIV_WORDS_FLT 4                       // F_DIV

const char *asm_div_read (const char *mn);        // QUO / REM / F_QUO, NULL if mn is no division
int         asm_div_words(const char *mn);        // its words (above), 0 if mn is no division

// ****************************************************************************
// asm_normnop -- a NOP between a normalised operation and a JIZ/LDI/ILI ------
// ****************************************************************************
//
// JIZ (the jump decision) and LDI/ILI (the read index) take the ALU output of
// the instruction right before them, and read it before the float normaliser
// (SAPHO/ula.v, out_fast): the normaliser is the longest path, and leaving it
// out of the jump and of the read address raises the clock. So a JIZ, LDI or
// ILI right after an operation whose ALU result goes through the normaliser
// (asm_is_norm) gets a NOP in between: the NOP's ALU output is the
// accumulator, the normalised result. A label on that line stays there: a
// jump to it comes from a jump, never from the normalised operation.
//
// asmcomp runs it on the .asm it assembles (the compilers' and hand-written
// ones alike), before its first pass. pc_path (pc_<proc>_mem.txt, may be
// NULL or missing) repeats the reader's source line for the new word. Returns
// how many NOPs went in (the .asm is rewritten only then), -1 on an I/O error.
int asm_normnop(const char *asm_path, const char *pc_path);
int asm_is_norm(const char *mn);                  // its ALU result goes through the normaliser

// ****************************************************************************
// asm_depth -- each stack as deep as the program goes, not a fixed 128 -------
// ****************************************************************************
//
// Works out from the .asm how deep the data stack (#NDSTAC) and the return-
// address stack (#SDEPTH, one word per CAL) get. A routine is the code a CAL
// target reaches until its RETs, plus address 0 and the #ITRAD point. Each is
// walked once, after the routines it calls: the data-stack depth must be the
// same on every path into an instruction (isa.tsv's stack column: push / pop),
// a callee's net effect is its depth at RET (negative: it pops its
// arguments), and a call adds the callee's peak to the caller's depth there.
// The interrupt can arrive anywhere, so its peaks add to the program's.
// Each depth written is the peak + 1 (the simulation's overflow flag fires
// when the pointer reaches DEPTH), at least 2.
//
// asmcomp calls it for a stack the .asm does not declare (#SDEPTH / #NDSTAC
// left out, in any front end or by hand); a declared depth stays. Recursion,
// an unknown instruction, or two paths that disagree on the depth: no depth,
// an Info line says why, and asmcomp keeps its default (128).
// Returns 1 with *sdepth / *ddepth set, 0 when they could not be worked out
// (both 0), -1 on an I/O error.
int asm_depth(const char *asm_path, int *sdepth, int *ddepth);

#endif
