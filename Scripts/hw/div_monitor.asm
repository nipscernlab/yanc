NOP
#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 1
#NUIOOU 1
// The processor of Scripts/hw/tb_div_monitor.sh (#183): an integer division,
// a remainder and a float division, each as the compilers write it (the
// division, the NOPs, the word that reads the result). a and b come from
// port 0, so nothing folds them. With DIV and MOD in the program, F_DIV shares
// their array and takes five words, three NOPs (#151).
@main INN 0
SET a
INN 0
SET b
DIV a
NOP
NOP
NOP
QUO
OUT 0
LOD b
MOD a
NOP
NOP
NOP
REM
OUT 0
LOD 10.0
SET x
LOD 3.0
F_DIV x
NOP
NOP
NOP
F_QUO
F2I
OUT 0
@fim JMP fim
