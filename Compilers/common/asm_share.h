// ****************************************************************************
// asm_share -- scalars whose lives never overlap share one data word ---------
// ****************************************************************************
//
// Reads a finished .asm, works out for every instruction which variables still
// hold a value that will be read later (liveness, on the whole program), and
// gives variables that are never alive at the same time one data word: for
// each name that moves into another's word it writes "#SHARE <name> <home>"
// ahead of the code, and appcomp/asmcomp give <name> the address of <home>.
// The code itself is not touched -- every instruction keeps its own names, so
// the listing reads as written, the pc_*_mem.txt line map stays valid, and
// asmcomp still shows each variable in the waveform on its own (it follows a
// shared variable by the instructions that write it, not by its address).
// TODO.md item 13.
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
// asm_divseq -- a division takes three words: `<div> x; NOP; <read>` --------
// ****************************************************************************
//
// The dividers (SAPHO/ula.v) are cut by two registers: the result of a
// division started at cycle t is at the end of the array at t+2, while the
// ALU executes the third word. So every DIV / S_DIV becomes `DIV x; NOP; QUO`,
// MOD / S_MOD `...; NOP; REM` and F_DIV / SF_DIV `...; NOP; F_QUO` (QUO, REM,
// F_QUO: aliases of the division's own opcode, no operand; the read is always
// the memory form, so a stack form is not popped twice). A division already
// followed by NOP and its read is left alone. Labels stay on the line they
// were on, so a jump still lands on the division, never inside the sequence.
//
// pc_path (pc_<proc>_mem.txt, may be NULL) repeats a division's source line
// for its two new words. Run it after asm_reach (dead code needs no
// sequence) and before asm_share. Returns how many words were added inside
// the part of the program the pc map covers (the caller's instruction count
// grows by that much), -1 on an I/O error.
int asm_divseq(const char *asm_path, const char *pc_path);

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
