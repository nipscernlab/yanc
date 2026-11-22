// Negative fixture for asmcomp (regress step 4b2): a division only starts its
// divider, and its read comes a fixed number of words later (#113). A slot
// between the two may hold an instruction that leaves the read on time, a
// SET or a PSH, but not a jump: here JMP fim leaves the sequence, QUO never
// runs and the quotient is lost. Hand-written: the compilers' asm_divslot
// only moves a SET or a PSH into a slot.
#PRNAME div_slot
#NUBITS 16
#NUIOIN 1
#NUIOOU 1
@main NOP
LOD 7
SET a
LOD 21
DIV a
JMP fim
NOP
NOP
QUO
OUT 0
@fim JMP fim
