// Negative fixture for asmcomp (regress step 4b2): P_XPO_M is not in the
// instruction set (XPO_M is). asmcomp read the word as an operand nobody
// waited for and dropped the line, so the program ran without it.
#PRNAME unknown_mnemonic
#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 1
#NUIOOU 1
@main F_INN 0
SET fv4
P_XPO_M fv4
S_ADD
OUT 0
@fim JMP fim
