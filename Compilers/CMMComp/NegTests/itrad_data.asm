// Negative fixture for asmcomp (regress step 4b2): the itr pulse empties both
// stacks (#188), so the data stack must be empty right before the #ITRAD
// point on every path that reaches it. Here P_LOD leaves the 1 on the data
// stack when the program falls into the point; the S_ADD after it would pop a
// word the pulse dropped. Hand-written: cmmcomp puts #PRACA between
// statements, where its data stack is empty.
#PRNAME itrad_data
#NUBITS 16
#NUIOIN 1
#NUIOOU 1
@main LOD 1
P_LOD 2
#ITRAD
S_ADD
OUT 0
@fim JMP fim
