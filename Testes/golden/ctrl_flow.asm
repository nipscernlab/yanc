NOP
#PRNAME ctrl_flow
#NUBITS 16
#NDSTAC 8
#SDEPTH 8
#NUIOIN 1
#NUIOOU 1
#NBMANT 10
#NBEXPO 5
#NUGAIN 128
@main @Lwh1 LOD 1
JIZ Lwh1end
LOD 1
JIZ Lif1else
LOD 11
OUT 0
@Lif1else LOD 0
JIZ Lif2else
LOD 12
OUT 0
@Lif2else LOD 1
JIZ Lif3else
LOD 21
OUT 0
JMP Lif3end
@Lif3else LOD 22
OUT 0
@Lif3end LOD 0
JIZ Lif4else
LOD 31
OUT 0
JMP Lif4end
@Lif4else LOD 32
OUT 0
@Lif4end LOD 1
JIZ Lif5else
LOD 1
JIZ Lif6else
LOD 41
OUT 0
@Lif6else @Lif5else LOD 0
JIZ Lif7else
LOD 51
OUT 0
JMP Lif7end
@Lif7else LOD 1
JIZ Lif8else
LOD 52
OUT 0
@Lif8else @Lif7end LOD 1
JIZ Lif9else
LOD 3
SET main_wc
@Lwh2 LOD main_wc
JIZ Lwh2end
LOD 60
ADD main_wc
OUT 0
NEG_M 1
ADD main_wc
SET main_wc
JMP Lwh2
@Lwh2end @Lif9else LOD 2
SET main_ic
@Lwh3 LOD main_ic
JIZ Lwh3end
LOD main_ic
JIZ Lif10else
LOD 70
ADD main_ic
OUT 0
@Lif10else NEG_M 1
ADD main_ic
SET main_ic
JMP Lwh3
@Lwh3end LOD 2
SET main_outer
@Lwh4 LOD main_outer
JIZ Lwh4end
LOD 2
SET main_inner
@Lwh5 LOD main_inner
JIZ Lwh5end
LOD main_outer
MLT 10
ADD 80
ADD main_inner
OUT 0
NEG_M 1
ADD main_inner
SET main_inner
JMP Lwh5
@Lwh5end NEG_M 1
ADD main_outer
SET main_outer
JMP Lwh4
@Lwh4end LOD 5
SET main_bc
@Lwh6 LOD main_bc
JIZ Lwh6end
LOD 3
EQU main_bc
JIZ Lif11else
JMP Lwh6end
@Lif11else LOD 100
ADD main_bc
OUT 0
NEG_M 1
ADD main_bc
SET main_bc
JMP Lwh6
@Lwh6end LOD 0
OUT 0
JMP Lwh1
@Lwh1end @fim JMP fim
