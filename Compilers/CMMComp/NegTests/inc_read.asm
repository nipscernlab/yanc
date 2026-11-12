// Negative fixture for asmcomp (regress step 4b2): INC k writes k the cycle
// after its decode (#203); the read of the instruction right after leaves on
// that same edge and would see the old k. Here ADD k reads it. Hand-written:
// the compilers' asm_inc drops a LOD k there and makes a P_LOD k a PSH, and
// keeps any other read of k from following an INC.
#PRNAME inc_read
#NUBITS 16
#NUIOIN 1
#NUIOOU 1
@main NOP
LOD 0
SET k
INC k
ADD k
OUT 0
@fim JMP fim
