NOP
#PRNAME cmm_cov_rest
#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 1
#NUIOOU 1
#array vi 1 4
#array vf 2 4
#array mi 1 12
#array mf 2 12
#arrayd si 1 4 4 1 2 3 4
#arrayd sf 2 4 4 1.5 2.5 3.5 4.5
#SHARE par_f_p par_i_p
#SHARE par_c_p_i par_i_p
#SHARE early_p par_i_p
#SHARE mix_c_i par_i_p
#SHARE mix_c par_c_p
#SHARE main_n1 par_i_p
#SHARE main_n2 par_c_p
#SHARE main_x mix_b
#SHARE main_y mix_b
#SHARE main_w mix_b
#SHARE main_w_i mix_a
#SHARE main_k mix_a
#SHARE main_g mix_a
#SHARE __aux_var mix_b
#SHARE __aux_var1 par_i_p
#SHARE __aux_var2 par_c_p
JMP main
@par_i SET par_i_p
RET
@par_f SET par_f_p
RET
@par_c SET_P par_c_p_i
SET par_c_p
P_LOD par_c_p_i
RET
@ret_i_i0 LOD gi
RET
@ret_i_i1 LOD gi
ADD gz
RET
@ret_i_i2 LOD 7
RET
@ret_i_f0 F2I_M gf
RET
@ret_i_f1 LOD gf
F_ADD gfz
F2I
RET
@ret_i_f2 F2I_M 2.5
RET
@ret_i_c0 F2I_M gc
RET
@ret_i_c1 LOD gc
F_MLT gone
P_LOD gc_i
F_MLT gone_i
SF_SU2
P_LOD gc
F_MLT gone_i
P_LOD gc_i
F_MLT gone
SF_ADD
POP
F2I
RET
@ret_i_c2 F2I_M 3.0
RET
@ret_f_i0 I2F_M gi
RET
@ret_f_i1 LOD gi
ADD gz
I2F
RET
@ret_f_i2 LOD 7.0
RET
@ret_f_f0 LOD gf
RET
@ret_f_f1 LOD gf
F_ADD gfz
RET
@ret_f_f2 LOD 2.5
RET
@ret_f_c0 LOD gc
RET
@ret_f_c1 LOD gc
F_MLT gone
P_LOD gc_i
F_MLT gone_i
SF_SU2
P_LOD gc
F_MLT gone_i
P_LOD gc_i
F_MLT gone
SF_ADD
POP
RET
@ret_f_c2 LOD 3.0
RET
@ret_c_i0 I2F_M gi
P_LOD 0.0
RET
@ret_c_i1 LOD gi
ADD gz
I2F
P_LOD 0.0
RET
@ret_c_i2 LOD 7.0
P_LOD 0.0
RET
@ret_c_f0 LOD gf
P_LOD 0.0
RET
@ret_c_f1 LOD gf
F_ADD gfz
P_LOD 0.0
RET
@ret_c_f2 LOD 2.5
P_LOD 0.0
RET
@ret_c_c0 LOD gc
P_LOD gc_i
RET
@ret_c_c1 LOD gc
F_MLT gone
P_LOD gc_i
F_MLT gone_i
SF_SU2
P_LOD gc
F_MLT gone_i
P_LOD gc_i
F_MLT gone
SF_ADD
RET
@ret_c_c2 LOD 3.0
P_LOD 4.0
RET
@early SET early_p
JIZ __Lif1else
RET
@__Lif1else LOD 77
OUT 0
RET
@mix SET_P mix_c_i
SET_P mix_c
SET_P mix_b
SET mix_a
I2F_M mix_a
F_ADD mix_b
F_ADD mix_c
F_ADD mix_c_i
RET
@main LOD 7
SET gi
LOD 0
SET gz
LOD 2.5
SET gf
LOD 0.0
SET gfz
LOD 3.0
SET gc
LOD 4.0
SET gc_i
LOD 1.0
SET gone
LOD 0.0
SET gone_i
LOD 1
SET main_n1
LOD 2
SET main_n2
LOD 0
SET main_z0
LOD 1000
SET main_pa
LOD 0.0
SET main_fz
LOD 1000.0
SET main_pf
LOD 1.0
SET main_one
LOD 0.0
SET main_one_i
LOD 100.0
SET main_pc
SET main_pc_i
LOD gi
SET main_x
OUT 0
LOD gi
ADD gz
SET main_x
OUT 0
LOD 7
SET main_x
OUT 0
F2I_M gf
SET main_x
OUT 0
LOD gf
F_ADD gfz
F2I
SET main_x
OUT 0
F2I_M 2.5
SET main_x
OUT 0
F2I_M gc
SET main_x
OUT 0
LOD gc
F_MLT gone
P_LOD gc_i
F_MLT gone_i
SF_SU2
P_LOD gc
F_MLT gone_i
P_LOD gc_i
F_MLT gone
SF_ADD
POP
F2I
SET main_x
OUT 0
F2I_M 3.0
SET main_x
OUT 0
I2F_M gi
SET main_y
LOD 4.0
F_MLT main_y
F2I
OUT 0
LOD gi
ADD gz
I2F
SET main_y
LOD 4.0
F_MLT main_y
F2I
OUT 0
LOD 7.0
SET main_y
LOD 4.0
F_MLT main_y
F2I
OUT 0
LOD gf
SET main_y
LOD 4.0
F_MLT main_y
F2I
OUT 0
LOD gf
F_ADD gfz
SET main_y
LOD 4.0
F_MLT main_y
F2I
OUT 0
LOD 2.5
SET main_y
LOD 4.0
F_MLT main_y
F2I
OUT 0
LOD gc
SET main_y
LOD 4.0
F_MLT main_y
F2I
OUT 0
LOD gc
F_MLT gone
P_LOD gc_i
F_MLT gone_i
SF_SU2
P_LOD gc
F_MLT gone_i
P_LOD gc_i
F_MLT gone
SF_ADD
POP
SET main_y
LOD 4.0
F_MLT main_y
F2I
OUT 0
LOD 3.0
SET main_y
LOD 4.0
F_MLT main_y
F2I
OUT 0
I2F_M gi
SET main_w
LOD 0.0
SET main_w_i
LOD main_w
SET main_w
LOD main_w_i
SET main_w_i
LOD 4.0
F_MLT main_w
F2I
OUT 0
LOD 4.0
F_MLT main_w_i
F2I
OUT 0
LOD gi
ADD gz
I2F
SET main_w
LOD 0.0
SET main_w_i
LOD main_w
SET main_w
LOD main_w_i
SET main_w_i
LOD 4.0
F_MLT main_w
F2I
OUT 0
LOD 4.0
F_MLT main_w_i
F2I
OUT 0
LOD 7.0
SET main_w
LOD 0.0
SET main_w_i
LOD main_w
SET main_w
LOD main_w_i
SET main_w_i
LOD 4.0
F_MLT main_w
F2I
OUT 0
LOD 4.0
F_MLT main_w_i
F2I
OUT 0
LOD gf
SET main_w
LOD 0.0
SET main_w_i
LOD main_w
SET main_w
LOD main_w_i
SET main_w_i
LOD 4.0
F_MLT main_w
F2I
OUT 0
LOD 4.0
F_MLT main_w_i
F2I
OUT 0
LOD gf
F_ADD gfz
SET main_w
LOD 0.0
SET main_w_i
LOD main_w
SET main_w
LOD main_w_i
SET main_w_i
LOD 4.0
F_MLT main_w
F2I
OUT 0
LOD 4.0
F_MLT main_w_i
F2I
OUT 0
LOD 2.5
SET main_w
LOD 0.0
SET main_w_i
LOD main_w
SET main_w
LOD main_w_i
SET main_w_i
LOD 4.0
F_MLT main_w
F2I
OUT 0
LOD 4.0
F_MLT main_w_i
F2I
OUT 0
LOD gc
SET main_w
LOD gc_i
SET main_w_i
LOD main_w
SET main_w
LOD main_w_i
SET main_w_i
LOD 4.0
F_MLT main_w
F2I
OUT 0
LOD 4.0
F_MLT main_w_i
F2I
OUT 0
LOD gc
F_MLT gone
P_LOD gc_i
F_MLT gone_i
SF_SU2
P_LOD gc
F_MLT gone_i
P_LOD gc_i
F_MLT gone
SF_ADD
SET_P main_w_i
SET main_w
SET main_w
LOD main_w_i
SET main_w_i
LOD 4.0
F_MLT main_w
F2I
OUT 0
LOD 4.0
F_MLT main_w_i
F2I
OUT 0
LOD 3.0
SET main_w
LOD 4.0
SET main_w_i
LOD main_w
SET main_w
LOD main_w_i
SET main_w_i
LOD 4.0
F_MLT main_w
F2I
OUT 0
LOD 4.0
F_MLT main_w_i
F2I
OUT 0
LOD 5
SET main_k
PSH
ADD 1
SET_P main_k
SET main_x
OUT 0
LOD main_k
OUT 0
LOD 1.5
SET main_g
PSH
F_ADD 1.0
SET_P main_g
SET main_y
LOD 4.0
F_MLT main_y
F2I
OUT 0
LOD 4.0
F_MLT main_g
F2I
OUT 0
LOD 5
SET_V vi 1
LOD main_n1
PSH
LDI vi
ADD 1
STI vi
ADD -1
SET main_x
OUT 0
LOD_V vi 1
OUT 0
LOD 1.5
SET_V vf 1
LOD main_n1
PSH
LDI vf
SET __aux_var
F_ADD 1.0
STI vf
LOD __aux_var
SET main_y
LOD 4.0
F_MLT main_y
F2I
OUT 0
LOD_V vf 1
F_MLT 4.0
F2I
OUT 0
LOD 5
SET_V mi 6
LOD  main_n1
MLT  4
ADD  main_n2
PSH
LDI mi
ADD 1
STI mi
ADD -1
SET main_x
OUT 0
LOD_V mi 6
OUT 0
LOD 1.5
SET_V mf 6
LOD  main_n1
MLT  4
ADD  main_n2
PSH
LDI mf
SET __aux_var
F_ADD 1.0
STI mf
LOD __aux_var
SET main_y
LOD 4.0
F_MLT main_y
F2I
OUT 0
LOD_V mf 6
F_MLT 4.0
F2I
OUT 0
LOD 5
SET main_k
LOD main_pa
ADD main_z0
P_LOD main_k
PSH
ADD 1
SET_P main_k
S_ADD
SET main_x
OUT 0
LOD main_k
OUT 0
LOD 1.5
SET main_g
LOD main_pf
F_ADD main_fz
P_LOD main_g
PSH
F_ADD 1.0
SET_P main_g
SF_ADD
SET main_y
LOD 4.0
F_MLT main_y
F2I
OUT 0
LOD 4.0
F_MLT main_g
F2I
OUT 0
LOD 5
SET_V vi 1
LOD main_pa
ADD main_z0
PSH
LOD main_n1
PSH
LDI vi
ADD 1
STI vi
ADD -1
S_ADD
SET main_x
OUT 0
LOD_V vi 1
OUT 0
LOD 1.5
SET_V vf 1
LOD main_pf
F_ADD main_fz
PSH
LOD main_n1
PSH
LDI vf
SET __aux_var
F_ADD 1.0
STI vf
LOD __aux_var
SF_ADD
SET main_y
LOD 4.0
F_MLT main_y
F2I
OUT 0
LOD_V vf 1
F_MLT 4.0
F2I
OUT 0
LOD 5
SET_V mi 6
LOD main_pa
ADD main_z0
PSH
LOD  main_n1
MLT  4
ADD  main_n2
PSH
LDI mi
ADD 1
STI mi
ADD -1
S_ADD
SET main_x
OUT 0
LOD_V mi 6
OUT 0
LOD 1.5
SET_V mf 6
LOD main_pf
F_ADD main_fz
PSH
LOD  main_n1
MLT  4
ADD  main_n2
PSH
LDI mf
SET __aux_var
F_ADD 1.0
STI mf
LOD __aux_var
SF_ADD
SET main_y
LOD 4.0
F_MLT main_y
F2I
OUT 0
LOD_V mf 6
F_MLT 4.0
F2I
OUT 0
LOD gi
CAL par_i
OUT 0
LOD main_pa
ADD main_z0
P_LOD gi
CAL par_i
S_ADD
SET main_x
OUT 0
LOD gi
ADD gz
CAL par_i
OUT 0
LOD main_pa
ADD main_z0
P_LOD gi
ADD gz
CAL par_i
S_ADD
SET main_x
OUT 0
LOD 7
CAL par_i
OUT 0
LOD main_pa
ADD main_z0
P_LOD 7
CAL par_i
S_ADD
SET main_x
OUT 0
LOD gf
F2I
CAL par_i
OUT 0
LOD main_pa
ADD main_z0
P_LOD gf
F2I
CAL par_i
S_ADD
SET main_x
OUT 0
LOD gf
F_ADD gfz
F2I
CAL par_i
OUT 0
LOD main_pa
ADD main_z0
P_LOD gf
F_ADD gfz
F2I
CAL par_i
S_ADD
SET main_x
OUT 0
LOD 2.5
F2I
CAL par_i
OUT 0
LOD main_pa
ADD main_z0
P_LOD 2.5
F2I
CAL par_i
S_ADD
SET main_x
OUT 0
LOD gc
F2I
CAL par_i
OUT 0
LOD main_pa
ADD main_z0
P_LOD gc
F2I
CAL par_i
S_ADD
SET main_x
OUT 0
LOD gc
F_MLT gone
P_LOD gc_i
F_MLT gone_i
SF_SU2
P_LOD gc
F_MLT gone_i
P_LOD gc_i
F_MLT gone
SF_ADD
POP
F2I
CAL par_i
OUT 0
LOD main_pa
ADD main_z0
P_LOD gc
F_MLT gone
P_LOD gc_i
F_MLT gone_i
SF_SU2
P_LOD gc
F_MLT gone_i
P_LOD gc_i
F_MLT gone
SF_ADD
POP
F2I
CAL par_i
S_ADD
SET main_x
OUT 0
LOD 3.0
F2I
CAL par_i
OUT 0
LOD main_pa
ADD main_z0
P_LOD 3.0
F2I
CAL par_i
S_ADD
SET main_x
OUT 0
I2F_M gi
CAL par_f
F_MLT 4.0
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_I2F_M gi
CAL par_f
SF_ADD
SET main_y
LOD 4.0
F_MLT main_y
F2I
OUT 0
LOD gi
ADD gz
I2F
CAL par_f
F_MLT 4.0
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD gi
ADD gz
I2F
CAL par_f
SF_ADD
SET main_y
LOD 4.0
F_MLT main_y
F2I
OUT 0
LOD 7.0
CAL par_f
F_MLT 4.0
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 7.0
CAL par_f
SF_ADD
SET main_y
LOD 4.0
F_MLT main_y
F2I
OUT 0
LOD gf
CAL par_f
F_MLT 4.0
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD gf
CAL par_f
SF_ADD
SET main_y
LOD 4.0
F_MLT main_y
F2I
OUT 0
LOD gf
F_ADD gfz
CAL par_f
F_MLT 4.0
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD gf
F_ADD gfz
CAL par_f
SF_ADD
SET main_y
LOD 4.0
F_MLT main_y
F2I
OUT 0
LOD 2.5
CAL par_f
F_MLT 4.0
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 2.5
CAL par_f
SF_ADD
SET main_y
LOD 4.0
F_MLT main_y
F2I
OUT 0
LOD gc
CAL par_f
F_MLT 4.0
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD gc
CAL par_f
SF_ADD
SET main_y
LOD 4.0
F_MLT main_y
F2I
OUT 0
LOD gc
F_MLT gone
P_LOD gc_i
F_MLT gone_i
SF_SU2
P_LOD gc
F_MLT gone_i
P_LOD gc_i
F_MLT gone
SF_ADD
POP
CAL par_f
F_MLT 4.0
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD gc
F_MLT gone
P_LOD gc_i
F_MLT gone_i
SF_SU2
P_LOD gc
F_MLT gone_i
P_LOD gc_i
F_MLT gone
SF_ADD
POP
CAL par_f
SF_ADD
SET main_y
LOD 4.0
F_MLT main_y
F2I
OUT 0
LOD 3.0
CAL par_f
F_MLT 4.0
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 3.0
CAL par_f
SF_ADD
SET main_y
LOD 4.0
F_MLT main_y
F2I
OUT 0
I2F_M gi
P_LOD 0.0
CAL par_c
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
LOD main_pc
F_MLT main_one
P_LOD main_pc_i
F_MLT main_one_i
SF_SU2
P_LOD main_pc
F_MLT main_one_i
P_LOD main_pc_i
F_MLT main_one
SF_ADD
P_I2F_M gi
P_LOD 0.0
CAL par_c
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
SET main_w
LOD main_w_i
SET main_w_i
LOD 4.0
F_MLT main_w
F2I
OUT 0
LOD 4.0
F_MLT main_w_i
F2I
OUT 0
LOD gi
ADD gz
I2F
P_LOD 0.0
CAL par_c
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
LOD main_pc
F_MLT main_one
P_LOD main_pc_i
F_MLT main_one_i
SF_SU2
P_LOD main_pc
F_MLT main_one_i
P_LOD main_pc_i
F_MLT main_one
SF_ADD
P_LOD gi
ADD gz
I2F
P_LOD 0.0
CAL par_c
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
SET main_w
LOD main_w_i
SET main_w_i
LOD 4.0
F_MLT main_w
F2I
OUT 0
LOD 4.0
F_MLT main_w_i
F2I
OUT 0
LOD 7.0
P_LOD 0.0
CAL par_c
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
LOD main_pc
F_MLT main_one
P_LOD main_pc_i
F_MLT main_one_i
SF_SU2
P_LOD main_pc
F_MLT main_one_i
P_LOD main_pc_i
F_MLT main_one
SF_ADD
P_LOD 7.0
P_LOD 0.0
CAL par_c
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
SET main_w
LOD main_w_i
SET main_w_i
LOD 4.0
F_MLT main_w
F2I
OUT 0
LOD 4.0
F_MLT main_w_i
F2I
OUT 0
LOD gf
P_LOD 0.0
CAL par_c
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
LOD main_pc
F_MLT main_one
P_LOD main_pc_i
F_MLT main_one_i
SF_SU2
P_LOD main_pc
F_MLT main_one_i
P_LOD main_pc_i
F_MLT main_one
SF_ADD
P_LOD gf
P_LOD 0.0
CAL par_c
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
SET main_w
LOD main_w_i
SET main_w_i
LOD 4.0
F_MLT main_w
F2I
OUT 0
LOD 4.0
F_MLT main_w_i
F2I
OUT 0
LOD gf
F_ADD gfz
P_LOD 0.0
CAL par_c
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
LOD main_pc
F_MLT main_one
P_LOD main_pc_i
F_MLT main_one_i
SF_SU2
P_LOD main_pc
F_MLT main_one_i
P_LOD main_pc_i
F_MLT main_one
SF_ADD
P_LOD gf
F_ADD gfz
P_LOD 0.0
CAL par_c
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
SET main_w
LOD main_w_i
SET main_w_i
LOD 4.0
F_MLT main_w
F2I
OUT 0
LOD 4.0
F_MLT main_w_i
F2I
OUT 0
LOD 2.5
P_LOD 0.0
CAL par_c
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
LOD main_pc
F_MLT main_one
P_LOD main_pc_i
F_MLT main_one_i
SF_SU2
P_LOD main_pc
F_MLT main_one_i
P_LOD main_pc_i
F_MLT main_one
SF_ADD
P_LOD 2.5
P_LOD 0.0
CAL par_c
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
SET main_w
LOD main_w_i
SET main_w_i
LOD 4.0
F_MLT main_w
F2I
OUT 0
LOD 4.0
F_MLT main_w_i
F2I
OUT 0
LOD gc
P_LOD gc_i
CAL par_c
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
LOD main_pc
F_MLT main_one
P_LOD main_pc_i
F_MLT main_one_i
SF_SU2
P_LOD main_pc
F_MLT main_one_i
P_LOD main_pc_i
F_MLT main_one
SF_ADD
P_LOD gc
P_LOD gc_i
CAL par_c
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
SET main_w
LOD main_w_i
SET main_w_i
LOD 4.0
F_MLT main_w
F2I
OUT 0
LOD 4.0
F_MLT main_w_i
F2I
OUT 0
LOD gc
F_MLT gone
P_LOD gc_i
F_MLT gone_i
SF_SU2
P_LOD gc
F_MLT gone_i
P_LOD gc_i
F_MLT gone
SF_ADD
CAL par_c
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
LOD main_pc
F_MLT main_one
P_LOD main_pc_i
F_MLT main_one_i
SF_SU2
P_LOD main_pc
F_MLT main_one_i
P_LOD main_pc_i
F_MLT main_one
SF_ADD
P_LOD gc
F_MLT gone
P_LOD gc_i
F_MLT gone_i
SF_SU2
P_LOD gc
F_MLT gone_i
P_LOD gc_i
F_MLT gone
SF_ADD
CAL par_c
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
SET main_w
LOD main_w_i
SET main_w_i
LOD 4.0
F_MLT main_w
F2I
OUT 0
LOD 4.0
F_MLT main_w_i
F2I
OUT 0
LOD 3.0
P_LOD 4.0
CAL par_c
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
LOD main_pc
F_MLT main_one
P_LOD main_pc_i
F_MLT main_one_i
SF_SU2
P_LOD main_pc
F_MLT main_one_i
P_LOD main_pc_i
F_MLT main_one
SF_ADD
P_LOD 3.0
P_LOD 4.0
CAL par_c
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
SET main_w
LOD main_w_i
SET main_w_i
LOD 4.0
F_MLT main_w
F2I
OUT 0
LOD 4.0
F_MLT main_w_i
F2I
OUT 0
CAL ret_i_i0
OUT 0
CAL ret_i_i1
OUT 0
CAL ret_i_i2
OUT 0
CAL ret_i_f0
OUT 0
CAL ret_i_f1
OUT 0
CAL ret_i_f2
OUT 0
CAL ret_i_c0
OUT 0
CAL ret_i_c1
OUT 0
CAL ret_i_c2
OUT 0
CAL ret_f_i0
F_MLT 4.0
F2I
OUT 0
CAL ret_f_i1
F_MLT 4.0
F2I
OUT 0
CAL ret_f_i2
F_MLT 4.0
F2I
OUT 0
CAL ret_f_f0
F_MLT 4.0
F2I
OUT 0
CAL ret_f_f1
F_MLT 4.0
F2I
OUT 0
CAL ret_f_f2
F_MLT 4.0
F2I
OUT 0
CAL ret_f_c0
F_MLT 4.0
F2I
OUT 0
CAL ret_f_c1
F_MLT 4.0
F2I
OUT 0
CAL ret_f_c2
F_MLT 4.0
F2I
OUT 0
CAL ret_c_i0
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
CAL ret_c_i1
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
CAL ret_c_i2
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
CAL ret_c_f0
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
CAL ret_c_f1
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
CAL ret_c_f2
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
CAL ret_c_c0
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
CAL ret_c_c1
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
CAL ret_c_c2
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
LOD 1
CAL early
LOD 0
CAL early
LOD gi
P_LOD gf
F_ADD gfz
P_LOD 3.0
P_LOD 4.0
CAL mix
F_MLT 4.0
F2I
OUT 0
LOD gi
ADD gz
P_LOD 2.5
P_LOD gc
F_MLT gone
P_LOD gc_i
F_MLT gone_i
SF_SU2
P_LOD gc
F_MLT gone_i
P_LOD gc_i
F_MLT gone
SF_ADD
CAL mix
F_MLT 4.0
F2I
OUT 0
LOD 7
SET gi
LOD 2.5
SET gf
LOD 3.0
SET gc
LOD 4.0
SET gc_i
LOD gi
JIZ __Lif2else
LOD 1
OUT 0
JMP __Lif2end
@__Lif2else LOD 0
OUT 0
@__Lif2end LOD 0
SET main_k
@__Lwh1 LOD gi
JIZ __Lwh1end
LOD 1
SET main_k
@__Lwh1end LOD main_k
OUT 0
LIN_M gi
SET main_x
OUT 0
LOD gi
ADD gz
JIZ __Lif3else
LOD 1
OUT 0
JMP __Lif3end
@__Lif3else LOD 0
OUT 0
@__Lif3end LOD 0
SET main_k
@__Lwh2 LOD gi
ADD gz
JIZ __Lwh2end
LOD 1
SET main_k
@__Lwh2end LOD main_k
OUT 0
LOD gi
ADD gz
LIN
SET main_x
OUT 0
LOD 7
JIZ __Lif4else
LOD 1
OUT 0
JMP __Lif4end
@__Lif4else LOD 0
OUT 0
@__Lif4end LOD 0
SET main_k
@__Lwh3 LOD 1
SET main_k
@__Lwh3end LOD main_k
OUT 0
LIN_M 7
SET main_x
OUT 0
LOD 7
SET gi
LOD 2.5
SET gf
LOD 3.0
SET gc
LOD 4.0
SET gc_i
LOD 0
SET gi
LOD 0.0
SET gf
SET gc
LOD 0.0
SET gc_i
LOD gi
JIZ __Lif5else
LOD 1
OUT 0
JMP __Lif5end
@__Lif5else LOD 0
OUT 0
@__Lif5end LOD 0
SET main_k
@__Lwh4 LOD gi
JIZ __Lwh4end
LOD 1
SET main_k
@__Lwh4end LOD main_k
OUT 0
LIN_M gi
SET main_x
OUT 0
LOD gi
ADD gz
JIZ __Lif6else
LOD 1
OUT 0
JMP __Lif6end
@__Lif6else LOD 0
OUT 0
@__Lif6end LOD 0
SET main_k
@__Lwh5 LOD gi
ADD gz
JIZ __Lwh5end
LOD 1
SET main_k
@__Lwh5end LOD main_k
OUT 0
LOD gi
ADD gz
LIN
SET main_x
OUT 0
LOD 0
JIZ __Lif7else
LOD 1
OUT 0
JMP __Lif7end
@__Lif7else LOD 0
OUT 0
@__Lif7end LOD 0
SET main_k
@__Lwh6 LOD 0
JIZ __Lwh6end
LOD 1
SET main_k
@__Lwh6end LOD main_k
OUT 0
LIN_M 0
SET main_x
OUT 0
LOD 7
SET gi
LOD 2.5
SET gf
LOD 3.0
SET gc
LOD 4.0
SET gc_i
LOD 7
SET gi
LOD 2.5
SET gf
LOD 3.0
SET gc
LOD 4.0
SET gc_i
LOD gf
AND 8388607
JIZ __Lif8else
LOD 1
OUT 0
JMP __Lif8end
@__Lif8else LOD 0
OUT 0
@__Lif8end LOD 0
SET main_k
@__Lwh7 LOD gf
AND 8388607
JIZ __Lwh7end
LOD 1
SET main_k
@__Lwh7end LOD main_k
OUT 0
LOD gf
AND 8388607
LIN
SET main_x
OUT 0
LOD gf
F_ADD gfz
AND 8388607
JIZ __Lif9else
LOD 1
OUT 0
JMP __Lif9end
@__Lif9else LOD 0
OUT 0
@__Lif9end LOD 0
SET main_k
@__Lwh8 LOD gf
F_ADD gfz
AND 8388607
JIZ __Lwh8end
LOD 1
SET main_k
@__Lwh8end LOD main_k
OUT 0
LOD gf
F_ADD gfz
AND 8388607
LIN
SET main_x
OUT 0
LOD 2.5
AND 8388607
JIZ __Lif10else
LOD 1
OUT 0
JMP __Lif10end
@__Lif10else LOD 0
OUT 0
@__Lif10end LOD 0
SET main_k
@__Lwh9 LOD 2.5
AND 8388607
JIZ __Lwh9end
LOD 1
SET main_k
@__Lwh9end LOD main_k
OUT 0
LOD 2.5
AND 8388607
LIN
SET main_x
OUT 0
LOD 7
SET gi
LOD 2.5
SET gf
LOD 3.0
SET gc
LOD 4.0
SET gc_i
LOD 0
SET gi
LOD 0.0
SET gf
SET gc
LOD 0.0
SET gc_i
LOD gf
AND 8388607
JIZ __Lif11else
LOD 1
OUT 0
JMP __Lif11end
@__Lif11else LOD 0
OUT 0
@__Lif11end LOD 0
SET main_k
@__Lwh10 LOD gf
AND 8388607
JIZ __Lwh10end
LOD 1
SET main_k
@__Lwh10end LOD main_k
OUT 0
LOD gf
AND 8388607
LIN
SET main_x
OUT 0
LOD gf
F_ADD gfz
AND 8388607
JIZ __Lif12else
LOD 1
OUT 0
JMP __Lif12end
@__Lif12else LOD 0
OUT 0
@__Lif12end LOD 0
SET main_k
@__Lwh11 LOD gf
F_ADD gfz
AND 8388607
JIZ __Lwh11end
LOD 1
SET main_k
@__Lwh11end LOD main_k
OUT 0
LOD gf
F_ADD gfz
AND 8388607
LIN
SET main_x
OUT 0
LOD 0.0
AND 8388607
JIZ __Lif13else
LOD 1
OUT 0
JMP __Lif13end
@__Lif13else LOD 0
OUT 0
@__Lif13end LOD 0
SET main_k
@__Lwh12 LOD 0.0
AND 8388607
JIZ __Lwh12end
LOD 1
SET main_k
@__Lwh12end LOD main_k
OUT 0
LOD 0.0
AND 8388607
LIN
SET main_x
OUT 0
LOD 7
SET gi
LOD 2.5
SET gf
LOD 3.0
SET gc
LOD 4.0
SET gc_i
LOD 7
SET gi
LOD 2.5
SET gf
LOD 3.0
SET gc
LOD 4.0
SET gc_i
LOD gc
ORR gc_i
AND 8388607
JIZ __Lif14else
LOD 1
OUT 0
JMP __Lif14end
@__Lif14else LOD 0
OUT 0
@__Lif14end LOD 0
SET main_k
@__Lwh13 LOD gc
ORR gc_i
AND 8388607
JIZ __Lwh13end
LOD 1
SET main_k
@__Lwh13end LOD main_k
OUT 0
LOD gc
ORR gc_i
AND 8388607
LIN
SET main_x
OUT 0
LOD gc
F_MLT gone
P_LOD gc_i
F_MLT gone_i
SF_SU2
P_LOD gc
F_MLT gone_i
P_LOD gc_i
F_MLT gone
SF_ADD
S_ORR
AND 8388607
JIZ __Lif15else
LOD 1
OUT 0
JMP __Lif15end
@__Lif15else LOD 0
OUT 0
@__Lif15end LOD 0
SET main_k
@__Lwh14 LOD gc
F_MLT gone
P_LOD gc_i
F_MLT gone_i
SF_SU2
P_LOD gc
F_MLT gone_i
P_LOD gc_i
F_MLT gone
SF_ADD
S_ORR
AND 8388607
JIZ __Lwh14end
LOD 1
SET main_k
@__Lwh14end LOD main_k
OUT 0
LOD gc
F_MLT gone
P_LOD gc_i
F_MLT gone_i
SF_SU2
P_LOD gc
F_MLT gone_i
P_LOD gc_i
F_MLT gone
SF_ADD
S_ORR
AND 8388607
LIN
SET main_x
OUT 0
LOD 3.0
ORR 4.0
AND 8388607
JIZ __Lif16else
LOD 1
OUT 0
JMP __Lif16end
@__Lif16else LOD 0
OUT 0
@__Lif16end LOD 0
SET main_k
@__Lwh15 LOD 3.0
ORR 4.0
AND 8388607
JIZ __Lwh15end
LOD 1
SET main_k
@__Lwh15end LOD main_k
OUT 0
LOD 3.0
ORR 4.0
AND 8388607
LIN
SET main_x
OUT 0
LOD 7
SET gi
LOD 2.5
SET gf
LOD 3.0
SET gc
LOD 4.0
SET gc_i
LOD 0
SET gi
LOD 0.0
SET gf
SET gc
LOD 0.0
SET gc_i
LOD gc
ORR gc_i
AND 8388607
JIZ __Lif17else
LOD 1
OUT 0
JMP __Lif17end
@__Lif17else LOD 0
OUT 0
@__Lif17end LOD 0
SET main_k
@__Lwh16 LOD gc
ORR gc_i
AND 8388607
JIZ __Lwh16end
LOD 1
SET main_k
@__Lwh16end LOD main_k
OUT 0
LOD gc
ORR gc_i
AND 8388607
LIN
SET main_x
OUT 0
LOD gc
F_MLT gone
P_LOD gc_i
F_MLT gone_i
SF_SU2
P_LOD gc
F_MLT gone_i
P_LOD gc_i
F_MLT gone
SF_ADD
S_ORR
AND 8388607
JIZ __Lif18else
LOD 1
OUT 0
JMP __Lif18end
@__Lif18else LOD 0
OUT 0
@__Lif18end LOD 0
SET main_k
@__Lwh17 LOD gc
F_MLT gone
P_LOD gc_i
F_MLT gone_i
SF_SU2
P_LOD gc
F_MLT gone_i
P_LOD gc_i
F_MLT gone
SF_ADD
S_ORR
AND 8388607
JIZ __Lwh17end
LOD 1
SET main_k
@__Lwh17end LOD main_k
OUT 0
LOD gc
F_MLT gone
P_LOD gc_i
F_MLT gone_i
SF_SU2
P_LOD gc
F_MLT gone_i
P_LOD gc_i
F_MLT gone
SF_ADD
S_ORR
AND 8388607
LIN
SET main_x
OUT 0
LOD 0.0
ORR 0.0
AND 8388607
JIZ __Lif19else
LOD 1
OUT 0
JMP __Lif19end
@__Lif19else LOD 0
OUT 0
@__Lif19end LOD 0
SET main_k
@__Lwh18 LOD 0.0
ORR 0.0
AND 8388607
JIZ __Lwh18end
LOD 1
SET main_k
@__Lwh18end LOD main_k
OUT 0
LOD 0.0
ORR 0.0
AND 8388607
LIN
SET main_x
OUT 0
LOD 7
SET gi
LOD 2.5
SET gf
LOD 3.0
SET gc
LOD 4.0
SET gc_i
LOD gi
ADD -2
JIZ __sw_body_1_1
ADD -1
JIZ __sw_body_1_2
ADD -4
JIZ __sw_body_1_3
JMP __sw_body_1_4
@__sw_body_1_1 LOD 20
OUT 0
JMP __switch_end_1
@__sw_body_1_2 LOD 30
OUT 0
JMP __switch_end_1
@__sw_body_1_3 LOD 70
OUT 0
JMP __switch_end_1
@__sw_body_1_4 LOD 99
OUT 0
@__switch_end_1 LOD gi
ADD gz
ADD -2
JIZ __sw_body_2_1
ADD -1
JIZ __sw_body_2_2
ADD -4
JIZ __sw_body_2_3
JMP __sw_body_2_4
@__sw_body_2_1 LOD 20
OUT 0
JMP __switch_end_2
@__sw_body_2_2 LOD 30
OUT 0
JMP __switch_end_2
@__sw_body_2_3 LOD 70
OUT 0
JMP __switch_end_2
@__sw_body_2_4 LOD 99
OUT 0
@__switch_end_2 LOD 7
ADD -2
JIZ __sw_body_3_1
ADD -1
JIZ __sw_body_3_2
ADD -4
JIZ __sw_body_3_3
JMP __sw_body_3_4
@__sw_body_3_1 LOD 20
OUT 0
JMP __switch_end_3
@__sw_body_3_2 LOD 30
OUT 0
JMP __switch_end_3
@__sw_body_3_3 LOD 70
OUT 0
JMP __switch_end_3
@__sw_body_3_4 LOD 99
OUT 0
@__switch_end_3 F2I_M gf
ADD -2
JIZ __sw_body_4_1
ADD -1
JIZ __sw_body_4_2
ADD -4
JIZ __sw_body_4_3
JMP __sw_body_4_4
@__sw_body_4_1 LOD 20
OUT 0
JMP __switch_end_4
@__sw_body_4_2 LOD 30
OUT 0
JMP __switch_end_4
@__sw_body_4_3 LOD 70
OUT 0
JMP __switch_end_4
@__sw_body_4_4 LOD 99
OUT 0
@__switch_end_4 LOD gf
F_ADD gfz
F2I
ADD -2
JIZ __sw_body_5_1
ADD -1
JIZ __sw_body_5_2
ADD -4
JIZ __sw_body_5_3
JMP __sw_body_5_4
@__sw_body_5_1 LOD 20
OUT 0
JMP __switch_end_5
@__sw_body_5_2 LOD 30
OUT 0
JMP __switch_end_5
@__sw_body_5_3 LOD 70
OUT 0
JMP __switch_end_5
@__sw_body_5_4 LOD 99
OUT 0
@__switch_end_5 F2I_M 2.5
ADD -2
JIZ __sw_body_6_1
ADD -1
JIZ __sw_body_6_2
ADD -4
JIZ __sw_body_6_3
JMP __sw_body_6_4
@__sw_body_6_1 LOD 20
OUT 0
JMP __switch_end_6
@__sw_body_6_2 LOD 30
OUT 0
JMP __switch_end_6
@__sw_body_6_3 LOD 70
OUT 0
JMP __switch_end_6
@__sw_body_6_4 LOD 99
OUT 0
@__switch_end_6 F2I_M gc
ADD -2
JIZ __sw_body_7_1
ADD -1
JIZ __sw_body_7_2
ADD -4
JIZ __sw_body_7_3
JMP __sw_body_7_4
@__sw_body_7_1 LOD 20
OUT 0
JMP __switch_end_7
@__sw_body_7_2 LOD 30
OUT 0
JMP __switch_end_7
@__sw_body_7_3 LOD 70
OUT 0
JMP __switch_end_7
@__sw_body_7_4 LOD 99
OUT 0
@__switch_end_7 LOD gc
F_MLT gone
P_LOD gc_i
F_MLT gone_i
SF_SU2
P_LOD gc
F_MLT gone_i
P_LOD gc_i
F_MLT gone
SF_ADD
POP
F2I
ADD -2
JIZ __sw_body_8_1
ADD -1
JIZ __sw_body_8_2
ADD -4
JIZ __sw_body_8_3
JMP __sw_body_8_4
@__sw_body_8_1 LOD 20
OUT 0
JMP __switch_end_8
@__sw_body_8_2 LOD 30
OUT 0
JMP __switch_end_8
@__sw_body_8_3 LOD 70
OUT 0
JMP __switch_end_8
@__sw_body_8_4 LOD 99
OUT 0
@__switch_end_8 F2I_M 3.0
ADD -2
JIZ __sw_body_9_1
ADD -1
JIZ __sw_body_9_2
ADD -4
JIZ __sw_body_9_3
JMP __sw_body_9_4
@__sw_body_9_1 LOD 20
OUT 0
JMP __switch_end_9
@__sw_body_9_2 LOD 30
OUT 0
JMP __switch_end_9
@__sw_body_9_3 LOD 70
OUT 0
JMP __switch_end_9
@__sw_body_9_4 LOD 99
OUT 0
@__switch_end_9 LOD gf
F2I
SHL gi
SET main_x
OUT 0
LOD gi
ADD gz
P_LOD gf
F_ADD gfz
F2I
S_SHL
SET main_x
OUT 0
LOD 2.5
F2I
SHR gi
SET main_x
OUT 0
LOD_V si 2
SET_V si 3
LOD_V si 1
SET_V si 2
LOD_V si 0
SET_V si 1
LOD gi
SET si
LOD_V si 0
OUT 0
LOD_V si 1
OUT 0
LOD_V si 2
OUT 0
LOD_V si 3
OUT 0
LOD gi
ADD gz
SET __aux_var
LOD_V si 2
SET_V si 3
LOD_V si 1
SET_V si 2
LOD_V si 0
SET_V si 1
LOD __aux_var
SET si
LOD_V si 0
OUT 0
LOD_V si 1
OUT 0
LOD_V si 2
OUT 0
LOD_V si 3
OUT 0
LOD_V si 2
SET_V si 3
LOD_V si 1
SET_V si 2
LOD_V si 0
SET_V si 1
LOD 7
SET si
LOD_V si 0
OUT 0
LOD_V si 1
OUT 0
LOD_V si 2
OUT 0
LOD_V si 3
OUT 0
F2I_M gf
SET __aux_var
LOD_V si 2
SET_V si 3
LOD_V si 1
SET_V si 2
LOD_V si 0
SET_V si 1
LOD __aux_var
SET si
LOD_V si 0
OUT 0
LOD_V si 1
OUT 0
LOD_V si 2
OUT 0
LOD_V si 3
OUT 0
LOD gf
F_ADD gfz
F2I
SET __aux_var
LOD_V si 2
SET_V si 3
LOD_V si 1
SET_V si 2
LOD_V si 0
SET_V si 1
LOD __aux_var
SET si
LOD_V si 0
OUT 0
LOD_V si 1
OUT 0
LOD_V si 2
OUT 0
LOD_V si 3
OUT 0
F2I_M 2.5
SET __aux_var
LOD_V si 2
SET_V si 3
LOD_V si 1
SET_V si 2
LOD_V si 0
SET_V si 1
LOD __aux_var
SET si
LOD_V si 0
OUT 0
LOD_V si 1
OUT 0
LOD_V si 2
OUT 0
LOD_V si 3
OUT 0
LOD_V sf 2
SET_V sf 3
LOD_V sf 1
SET_V sf 2
LOD_V sf 0
SET_V sf 1
I2F_M gi
SET sf
LOD_V sf 0
F_MLT 4.0
F2I
OUT 0
LOD_V sf 1
F_MLT 4.0
F2I
OUT 0
LOD_V sf 2
F_MLT 4.0
F2I
OUT 0
LOD_V sf 3
F_MLT 4.0
F2I
OUT 0
LOD gi
ADD gz
SET __aux_var
LOD_V sf 2
SET_V sf 3
LOD_V sf 1
SET_V sf 2
LOD_V sf 0
SET_V sf 1
I2F_M __aux_var
SET sf
LOD_V sf 0
F_MLT 4.0
F2I
OUT 0
LOD_V sf 1
F_MLT 4.0
F2I
OUT 0
LOD_V sf 2
F_MLT 4.0
F2I
OUT 0
LOD_V sf 3
F_MLT 4.0
F2I
OUT 0
LOD_V sf 2
SET_V sf 3
LOD_V sf 1
SET_V sf 2
LOD_V sf 0
SET_V sf 1
LOD 7.0
SET sf
LOD_V sf 0
F_MLT 4.0
F2I
OUT 0
LOD_V sf 1
F_MLT 4.0
F2I
OUT 0
LOD_V sf 2
F_MLT 4.0
F2I
OUT 0
LOD_V sf 3
F_MLT 4.0
F2I
OUT 0
LOD_V sf 2
SET_V sf 3
LOD_V sf 1
SET_V sf 2
LOD_V sf 0
SET_V sf 1
LOD gf
SET sf
LOD_V sf 0
F_MLT 4.0
F2I
OUT 0
LOD_V sf 1
F_MLT 4.0
F2I
OUT 0
LOD_V sf 2
F_MLT 4.0
F2I
OUT 0
LOD_V sf 3
F_MLT 4.0
F2I
OUT 0
LOD gf
F_ADD gfz
SET __aux_var
LOD_V sf 2
SET_V sf 3
LOD_V sf 1
SET_V sf 2
LOD_V sf 0
SET_V sf 1
LOD __aux_var
SET sf
LOD_V sf 0
F_MLT 4.0
F2I
OUT 0
LOD_V sf 1
F_MLT 4.0
F2I
OUT 0
LOD_V sf 2
F_MLT 4.0
F2I
OUT 0
LOD_V sf 3
F_MLT 4.0
F2I
OUT 0
LOD_V sf 2
SET_V sf 3
LOD_V sf 1
SET_V sf 2
LOD_V sf 0
SET_V sf 1
LOD 2.5
SET sf
LOD_V sf 0
F_MLT 4.0
F2I
OUT 0
LOD_V sf 1
F_MLT 4.0
F2I
OUT 0
LOD_V sf 2
F_MLT 4.0
F2I
OUT 0
LOD_V sf 3
F_MLT 4.0
F2I
OUT 0
F2I_M gf
OUT 0
LOD gf
F_ADD gfz
F2I
OUT 0
F2I_M 2.5
OUT 0
LOD gi
OUT 0
LOD gi
ADD gz
OUT 0
LOD 7
OUT 0
@fim JMP fim
