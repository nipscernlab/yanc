// Negative fixture for asmcomp (regress step 4b2): a program of a single
// instruction gives an instruction memory of one word, whose address has 0
// bits, and core.v does not elaborate (#190). asmcomp refuses it.
// Hand-written: cmmcomp and cppcomp always start with a NOP at address 0.
#PRNAME one_instruction
#NUBITS 16
#NUIOIN 1
#NUIOOU 1
@fim JMP fim
