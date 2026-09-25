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

#endif
