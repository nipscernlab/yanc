// Negative fixture for asmcomp (regress step 4b2): INC k writes k the cycle
// after its decode, on the data memory's one write port (#203), so the
// instruction right after it must not write a data word. Here SET y does, and
// one of the two writes would be lost. Hand-written: the compilers' asm_inc
// never makes an INC that a write follows.
#PRNAME inc_write
#NUBITS 16
#NUIOIN 1
#NUIOOU 1
@main NOP
LOD 0
SET k
INC k
SET y
OUT 0
@fim JMP fim
