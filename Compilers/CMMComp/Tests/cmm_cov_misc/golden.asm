NOP
#PRNAME cmm_cov_misc
#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 1
#NUIOOU 1
#arrayd fa3 2 3 3 1.0 2.0 3.0
#arrayd ia3 1 3 3 1 2 3
#array fv3 2 3
#array iv3 1 3
#arrayd fa1 2 1 1 2.5
#arrayd ia1 1 1 1 3
#array f1 2 1
#array i1 1 1
#array vc 3 4
#array vc_i 4 4
#SHARE main_x locals_w_i
#SHARE main_k locals_w
#SHARE main_s locals_w_i
#SHARE __switch_exp main_m5
#SHARE main_w_i main_m5
#SHARE main_w locals_w_i
#SHARE __dhf1_main main_m5
#SHARE __dk0_main locals_w_i
#SHARE main_y main_m5
JMP main
@locals #arrays locals_li 1 3 "li.txt"
#arrays locals_lf 2 3 "lf.txt"
#arrays locals_lc 3 2 "lc.txt"
#arrays locals_lc_i 4 2 "lc.txt"
#arrays locals_lmi 1 6 "mi.txt"
#arrays locals_lmf 2 6 "mf.txt"
#arrays locals_lmc 3 4 "mc.txt"
#arrays locals_lmc_i 4 4 "mc.txt"
LOD_V locals_li 2
OUT 0
LOD_V locals_lf 2
F_MLT 4.0
F2I
OUT 0
LOD_V locals_lc 1
P_LOD_V locals_lc_i 1
SET_P locals_w_i
SET locals_w
LOD 4.0
F_MLT locals_w
F2I
OUT 0
LOD 4.0
F_MLT locals_w_i
F2I
OUT 0
LOD_V locals_lmi 5
OUT 0
LOD_V locals_lmf 3
F_MLT 4.0
F2I
OUT 0
LOD_V locals_lmc 3
P_LOD_V locals_lmc_i 3
SET_P locals_w_i
SET locals_w
LOD 4.0
F_MLT locals_w
F2I
OUT 0
LOD 4.0
F_MLT locals_w_i
F2I
OUT 0
RET
@noarg LOD 5
RET
@main LOD 2
SET main_n2
LOD 5
SET main_m5
LOD 0
SET main_z0
LOD 1000
SET main_pa
LOD 2.5
SET main_fs
LOD 0.0
SET main_fz
LOD 1000.0
SET main_pf
LOD 3.0
SET main_cv
LOD 4.0
SET main_cv_i
LOD 1.0
SET main_one
LOD 0.0
SET main_one_i
CAL locals
LOD main_pa
ADD main_z0
PSH
CAL noarg
S_ADD
SET main_x
OUT 0
LOD 0
SET main_k
SET main_s
@__Lwh1 LOD main_k
ADD 1
SET main_k
LOD 4
XOR main_k
JIZ __Lwh1end
LOD main_s
ADD main_k
SET main_s
JMP __Lwh1
@__Lwh1end LOD main_s
OUT 0
LOD 0
SET main_k
SET main_s
@__Lwh2 LOD main_k
EQU 5
LIN
JIZ __Lwh2end
LOD main_k
ADD 1
SET main_k
EQU 3
JIZ __Lwh2
LOD main_s
ADD 10
SET main_s
JMP __Lwh2
@__Lwh2end LOD main_s
OUT 0
LOD 0
SET main_s
SET main_k
@__Lwh3 LOD 2
XOR main_k
JIZ __Lwh3cont
LOD main_s
ADD main_k
SET main_s
@__Lwh3cont LOD main_k
ADD 1
SET main_k
LES 4
JIZ __Lwh3
@__Lwh3end LOD main_s
OUT 0
LOD 0
SET main_k
SET main_s
@__Lwh4 LOD main_k
ADD 1
SET main_k
LOD 2
XOR main_k
JIZ __Lwh4cont
LOD main_s
ADD main_k
SET main_s
@__Lwh4cont LOD main_k
GRE 4
JIZ __Lwh4end
JMP __Lwh4
@__Lwh4end LOD main_s
OUT 0
LOD 3
SET main_k
LOD 0
SET main_s
@__Lwh5 LOD main_k
JIZ __Lwh5end
LOD main_s
ADD main_k
SET main_s
NEG_M 1
ADD main_k
SET main_k
JMP __Lwh5
@__Lwh5end LOD main_s
OUT 0
LOD 0
SET main_k
SET main_s
@__Lwh6 LOD main_k
EQU 5
LIN
JIZ __Lwh6end
LOD main_k
ADD 1
SET main_k
EQU 3
JIZ __Lif5else
LOD main_s
ADD 100
SET main_s
JMP __Lwh6
@__Lif5else LOD main_s
ADD main_k
SET main_s
JMP __Lwh6
@__Lwh6end LOD main_s
OUT 0
LOD 0
SET main_s
SET main_k
@__Lwh7 LOD main_k
EQU 2
JIZ __Lif6else
LOD main_s
ADD 100
SET main_s
JMP __Lwh7cont
@__Lif6else LOD main_s
ADD main_k
SET main_s
@__Lwh7cont LOD main_k
ADD 1
SET main_k
LES 4
JIZ __Lwh7
@__Lwh7end LOD main_s
OUT 0
LOD 0
SET main_s
SET main_k
@__Lwh8 LOD main_m5
LES main_k
JIZ __Lwh8end
LOD main_k
EQU 2
JIZ __Lif7else
LOD main_s
ADD 100
SET main_s
JMP __Lwh8cont
@__Lif7else LOD main_s
ADD main_k
SET main_s
@__Lwh8cont LOD main_k
ADD 1
SET main_k
JMP __Lwh8
@__Lwh8end LOD main_s
OUT 0
LOD 0
SET main_k
SET main_s
@__Lwh9 LOD main_k
ADD 1
SET main_k
EQU 2
JIZ __Lif8else
LOD main_s
ADD 100
SET main_s
JMP __Lwh9cont
@__Lif8else LOD main_s
ADD main_k
SET main_s
@__Lwh9cont LOD main_k
GRE 4
JIZ __Lwh9end
JMP __Lwh9
@__Lwh9end LOD main_s
OUT 0
LOD 0
SET main_s
LOD main_n2
ADD -1
JIZ __sw_body_1_1
ADD -2
JIZ __sw_body_1_2
JMP __switch_end_1
@__sw_body_1_1 LOD 10
SET main_s
JMP __switch_end_1
@__sw_body_1_2 LOD 30
SET main_s
@__switch_end_1 LOD main_s
OUT 0
LOD 0
SET main_s
LOD main_n2
SET __switch_exp
I2F_M __switch_exp
EQU 1.5
JIZ __sw_disp_2_1
JMP __sw_body_2_1
@__sw_disp_2_1 I2F_M __switch_exp
EQU 2.0
JIZ __switch_end_2
JMP __sw_body_2_2
@__sw_body_2_1 LOD 15
SET main_s
JMP __switch_end_2
@__sw_body_2_2 LOD 20
SET main_s
@__switch_end_2 LOD main_s
OUT 0
LOD 0
SET main_s
LOD main_n2
SET __switch_exp
I2F_M __switch_exp
EQU 1.5
JIZ __sw_disp_3_1
JMP __sw_body_3_1
@__sw_disp_3_1 I2F_M __switch_exp
EQU 3.0
JIZ __switch_end_3
JMP __sw_body_3_2
@__sw_body_3_1 LOD 15
SET main_s
JMP __switch_end_3
@__sw_body_3_2 LOD 30
SET main_s
@__switch_end_3 LOD main_s
OUT 0
LOD main_cv
F_MLT main_one
P_LOD main_cv_i
F_MLT main_one_i
SF_SU2
P_LOD main_cv
F_MLT main_one_i
P_LOD main_cv_i
F_MLT main_one
SF_ADD
SET_V vc_i 2
POP
SET_V vc 2
LOD_V vc 2
P_LOD_V vc_i 2
SET_P main_w_i
SET main_w
LOD 4.0
F_MLT main_w
F2I
OUT 0
LOD 4.0
F_MLT main_w_i
F2I
OUT 0
LOD main_n2
ADD main_z0
I2F
SET __dhf1_main
LOD -3
SET __dk0_main
@__Ld0 PSH
LDI_V fa3 3
F_MLT __dhf1_main
STI_V fv3 3
LOD __dk0_main
ADD 1
SET __dk0_main
JIZ __Ld0e
JMP __Ld0
@__Ld0e LOD_V fv3 0
F_MLT 4.0
F2I
OUT 0
LOD_V fv3 2
F_MLT 4.0
F2I
OUT 0
LOD -3
SET __dk0_main
@__Ld1 PSH
LDI_V ia3 3
I2F
STI_V fv3 3
LOD __dk0_main
ADD 1
SET __dk0_main
JIZ __Ld1e
JMP __Ld1
@__Ld1e LOD_V fv3 1
F_MLT 4.0
F2I
OUT 0
LOD -3
SET __dk0_main
@__Ld2 PSH
LDI_V fa3 3
F2I
STI_V iv3 3
LOD __dk0_main
ADD 1
SET __dk0_main
JIZ __Ld2e
JMP __Ld2
@__Ld2e LOD_V iv3 2
OUT 0
LOD fa1
SET_V f1 0
LOD_V f1 0
F_MLT 4.0
F2I
OUT 0
LOD ia1
I2F
SET_V f1 0
LOD_V f1 0
F_MLT 4.0
F2I
OUT 0
LOD fa1
F2I
SET_V i1 0
LOD_V i1 0
OUT 0
LOD main_pf
F_ADD main_fz
P_I2F_M ia3
F_MLT fa3
P_LOD -2
SET __dk0_main
@__Ld3 LDI_V fa3 3
P_LOD __dk0_main
LDI_V ia3 3
I2F
SF_MLT
SF_ADD
P_LOD __dk0_main
ADD 1
SET __dk0_main
JIZ __Ld3e
JMP __Ld3
@__Ld3e POP
SF_ADD
SET main_y
LOD 4.0
F_MLT main_y
F2I
OUT 0
F_NEG_M 3.0
PF_NEG_M 0.0
SET_P main_w_i
SET main_w
LOD 4.0
F_MLT main_w
F2I
OUT 0
LOD 4.0
F_MLT main_w_i
F2I
OUT 0
LOD 0.0
SET main_w
LOD -2.0
SET main_w_i
LOD 4.0
F_MLT main_w
F2I
OUT 0
LOD 4.0
F_MLT main_w_i
F2I
OUT 0
LOD main_fs
F_ADD main_fz
F2I
SHL main_n2
SET main_x
OUT 0
LOD main_n2
ADD main_z0
P_LOD main_fs
F2I
S_SHL
SET main_x
OUT 0
LOD main_n2
OUT 0
F2I_M main_fs
OUT 0
F2I_M main_fs
OUT 0
LOD main_n2
OUT 0
LOD_V ia3 2
P_LOD main_pa
ADD main_z0
S_ADD
SET main_x
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD_V fa3 1
SF_ADD
SET main_y
LOD 4.0
F_MLT main_y
F2I
OUT 0
LOD main_cv
F_MLT main_one
P_LOD main_cv_i
F_MLT main_one_i
SF_SU2
P_LOD_V vc 2
SF_ADD
P_LOD main_cv
F_MLT main_one_i
P_LOD main_cv_i
F_MLT main_one
SF_ADD
P_LOD_V vc_i 2
SF_ADD
SET_P main_w_i
SET main_w
LOD 4.0
F_MLT main_w
F2I
OUT 0
LOD 4.0
F_MLT main_w_i
F2I
OUT 0
@fim JMP fim
