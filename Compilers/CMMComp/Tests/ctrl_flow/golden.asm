NOP
#PRNAME ctrl_flow
#NUBITS 16
#NUIOIN 1
#NUIOOU 1
#NBMANT 10
#NBEXPO 5
#NUGAIN 128
#SHARE main_ic main_wc
#SHARE main_outer main_wc
#SHARE main_bc main_wc
@main @__Lwh1 LOD 1
JIZ __Lif1else
LOD 11
OUT 0
@__Lif1else LOD 0
JIZ __Lif2else
LOD 12
OUT 0
@__Lif2else LOD 1
JIZ __Lif3else
LOD 21
OUT 0
JMP __Lif3end
@__Lif3else LOD 22
OUT 0
@__Lif3end LOD 0
JIZ __Lif4else
LOD 31
OUT 0
JMP __Lif4end
@__Lif4else LOD 32
OUT 0
@__Lif4end LOD 1
JIZ __Lif5else
LOD 1
JIZ __Lif6else
LOD 41
OUT 0
@__Lif6else @__Lif5else LOD 0
JIZ __Lif7else
LOD 51
OUT 0
JMP __Lif7end
@__Lif7else LOD 1
JIZ __Lif8else
LOD 52
OUT 0
@__Lif8else @__Lif7end LOD 1
JIZ __Lif9else
LOD 3
SET main_wc
@__Lwh2 LOD main_wc
JIZ __Lwh2end
LOD 60
ADD main_wc
OUT 0
NEG_M 1
ADD main_wc
SET main_wc
JMP __Lwh2
@__Lwh2end @__Lif9else LOD 2
SET main_ic
@__Lwh3 LOD main_ic
JIZ __Lwh3end
LOD main_ic
JIZ __Lif10else
LOD 70
ADD main_ic
OUT 0
@__Lif10else NEG_M 1
ADD main_ic
SET main_ic
JMP __Lwh3
@__Lwh3end LOD 2
SET main_outer
@__Lwh4 LOD main_outer
JIZ __Lwh4end
LOD 2
SET main_inner
@__Lwh5 LOD main_inner
JIZ __Lwh5end
LOD main_outer
MLT 10
ADD 80
ADD main_inner
OUT 0
NEG_M 1
ADD main_inner
SET main_inner
JMP __Lwh5
@__Lwh5end NEG_M 1
ADD main_outer
SET main_outer
JMP __Lwh4
@__Lwh4end LOD 5
SET main_bc
@__Lwh6 LOD main_bc
JIZ __Lwh6end
LOD 3
XOR main_bc
JIZ __Lwh6end
LOD 100
ADD main_bc
OUT 0
NEG_M 1
ADD main_bc
SET main_bc
JMP __Lwh6
@__Lwh6end LOD 0
OUT 0
JMP __Lwh1
@__Lwh1end @fim JMP fim
