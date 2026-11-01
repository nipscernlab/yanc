NOP
#PRNAME cmm_cov_lib
#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 1
#NUIOOU 1
#SHARE __csqrt_im main_y
#SHARE main_w_i main_y
#SHARE __aux_var main_y
#SHARE __aux_var1 main_w
#SHARE __fase_re main_y
#SHARE __fase_t main_y
#SHARE __clog_im main_y
#SHARE __csin_eb __csin_shb
#SHARE __csin_emb main_y
#SHARE __csin_b main_y
#SHARE __ccos_eb __ccos_shb
#SHARE __ccos_emb main_y
#SHARE __ccos_b main_y
#SHARE __ctan_eb main_w
#SHARE __ctan_emb __aux_var2
#SHARE __ctan_chb main_y
#SHARE __ctan_shb main_w
#SHARE __ctan_d main_y
#SHARE __ctan_b __ctan_tb
#SHARE __catan_bp main_y
#SHARE __catan_num main_y
#SHARE __catan_bm main_w
#SHARE __catan_im main_y
#SHARE __catan_a __fase_im
#SHARE __tanh_e main_y
#SHARE __tanh_n main_w
#SHARE __floor_x main_ca
#SHARE __floor_t main_ca_i
#SHARE __ceil_x main_ca
#SHARE __ceil_t main_ca_i
#SHARE __round_x main_ca
#SHARE __pow_x main_w
#SHARE __pow_r main_y
#SHARE __pow_c __aux_var2
#SHARE __pow_b __pow_y
#SHARE __sqrt_num main_y
#SHARE __atan_x main_y
#SHARE __atan_ax __aux_var2
#SHARE __atan_big main_w
#SHARE __atan_t __aux_var2
#SHARE __atan_p __aux_var2
#SHARE __sin_x main_w
#SHARE __sin_q main_y
#SHARE __sin_k main_y
#SHARE __sin_r main_w
#SHARE __sin_w __aux_var2
#SHARE __sin_p main_w
#SHARE __tan_x main_y
#SHARE __tan_q main_w
#SHARE __tan_r main_y
#SHARE __tan_a __aux_var2
#SHARE __tan_fold main_w
#SHARE __tan_u __aux_var2
#SHARE __tan_w __atan_w
#SHARE __tan_p __aux_var2
#SHARE __exp_x main_y
#SHARE __exp_q main_w
#SHARE __exp_n main_w
#SHARE __exp_r main_y
#SHARE __log_x main_w
#SHARE __log_e main_y
#SHARE __log_ne __aux_var2
#SHARE __log_m __aux_var2
#SHARE __log_t2 main_w
#SHARE __log_t1 __aux_var2
#SHARE __log_u main_w
#SHARE __log_w __aux_var2
#SHARE __log_lm main_w
@main LOD 0
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
LOD 2
SET main_ia
LOD 2.25
SET main_fa
LOD 3.0
SET main_ca
LOD 4.0
SET main_ca_i
I2F_M main_ia
CAL __float_sqrt
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_I2F_M main_ia
CAL __float_sqrt
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ia
ADD main_z0
I2F
CAL __float_sqrt
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_ia
ADD main_z0
I2F
CAL __float_sqrt
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 2.0
CAL __float_sqrt
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 2.0
CAL __float_sqrt
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
CAL __float_sqrt
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
CAL __float_sqrt
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
F_ADD main_fz
CAL __float_sqrt
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
F_ADD main_fz
CAL __float_sqrt
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 2.25
CAL __float_sqrt
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 2.25
CAL __float_sqrt
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ca
F_MLT main_ca
P_LOD main_ca_i
F_MLT main_ca_i
SF_ADD
CAL __float_sqrt
SET __csqrt_r
F_ADD main_ca
F_MLT 0.5
CAL __float_sqrt
SET __csqrt_re
LOD __csqrt_r
F_SU1 main_ca
F_MLT 0.5
CAL __float_sqrt
F_SGN main_ca_i
SET __csqrt_im
LOD __csqrt_re
P_LOD __csqrt_im
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_ca
F_MLT main_ca
P_LOD main_ca_i
F_MLT main_ca_i
SF_ADD
CAL __float_sqrt
SET __csqrt_r
F_ADD main_ca
F_MLT 0.5
CAL __float_sqrt
SET __csqrt_re
LOD __csqrt_r
F_SU1 main_ca
F_MLT 0.5
CAL __float_sqrt
F_SGN main_ca_i
SET __csqrt_im
LOD __csqrt_re
P_LOD __csqrt_im
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD main_ca
F_MLT main_one
P_LOD main_ca_i
F_MLT main_one_i
SF_SU2
P_LOD main_ca
F_MLT main_one_i
P_LOD main_ca_i
F_MLT main_one
SF_ADD
SET_P __csqrt_b
SET   __csqrt_a
F_MLT __csqrt_a
P_LOD __csqrt_b
F_MLT __csqrt_b
SF_ADD
CAL __float_sqrt
SET __csqrt_r
F_ADD __csqrt_a
F_MLT 0.5
CAL __float_sqrt
SET __csqrt_re
LOD __csqrt_r
F_SU1 __csqrt_a
F_MLT 0.5
CAL __float_sqrt
F_SGN __csqrt_b
SET __csqrt_im
LOD __csqrt_re
P_LOD __csqrt_im
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_ca
F_MLT main_one
P_LOD main_ca_i
F_MLT main_one_i
SF_SU2
P_LOD main_ca
F_MLT main_one_i
P_LOD main_ca_i
F_MLT main_one
SF_ADD
SET_P __csqrt_b
SET   __csqrt_a
F_MLT __csqrt_a
P_LOD __csqrt_b
F_MLT __csqrt_b
SF_ADD
CAL __float_sqrt
SET __csqrt_r
F_ADD __csqrt_a
F_MLT 0.5
CAL __float_sqrt
SET __csqrt_re
LOD __csqrt_r
F_SU1 __csqrt_a
F_MLT 0.5
CAL __float_sqrt
F_SGN __csqrt_b
SET __csqrt_im
LOD __csqrt_re
P_LOD __csqrt_im
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD 3.0
F_MLT 3.0
P_LOD 4.0
F_MLT 4.0
SF_ADD
CAL __float_sqrt
SET __csqrt_r
F_ADD 3.0
F_MLT 0.5
CAL __float_sqrt
SET __csqrt_re
LOD __csqrt_r
F_SU1 3.0
F_MLT 0.5
CAL __float_sqrt
F_SGN 4.0
SET __csqrt_im
LOD __csqrt_re
P_LOD __csqrt_im
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
F_MLT 3.0
P_LOD 4.0
F_MLT 4.0
SF_ADD
CAL __float_sqrt
SET __csqrt_r
F_ADD 3.0
F_MLT 0.5
CAL __float_sqrt
SET __csqrt_re
LOD __csqrt_r
F_SU1 3.0
F_MLT 0.5
CAL __float_sqrt
F_SGN 4.0
SET __csqrt_im
LOD __csqrt_re
P_LOD __csqrt_im
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD 1
SET main_ia
LOD 0.5
SET main_fa
SET main_ca
LOD 0.25
SET main_ca_i
I2F_M main_ia
CAL __float_exp
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_I2F_M main_ia
CAL __float_exp
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ia
ADD main_z0
I2F
CAL __float_exp
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_ia
ADD main_z0
I2F
CAL __float_exp
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 1.0
CAL __float_exp
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 1.0
CAL __float_exp
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
CAL __float_exp
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
CAL __float_exp
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
F_ADD main_fz
CAL __float_exp
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
F_ADD main_fz
CAL __float_exp
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 0.5
CAL __float_exp
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 0.5
CAL __float_exp
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ca
CAL __float_exp
SET __cexp_ea
LOD main_ca_i
CAL __float_sin
F_MLT __cexp_ea
SET __cexp_im
LOD main_ca_i
F_SU2 1.570796327
CAL __float_sin
F_MLT __cexp_ea
P_LOD __cexp_im
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_ca
CAL __float_exp
SET __cexp_ea
LOD main_ca_i
CAL __float_sin
F_MLT __cexp_ea
SET __cexp_im
LOD main_ca_i
F_SU2 1.570796327
CAL __float_sin
F_MLT __cexp_ea
P_LOD __cexp_im
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD main_ca
F_MLT main_one
P_LOD main_ca_i
F_MLT main_one_i
SF_SU2
P_LOD main_ca
F_MLT main_one_i
P_LOD main_ca_i
F_MLT main_one
SF_ADD
SET_P __cexp_b
CAL __float_exp
SET __cexp_ea
LOD __cexp_b
CAL __float_sin
F_MLT __cexp_ea
SET __cexp_im
LOD __cexp_b
F_SU2 1.570796327
CAL __float_sin
F_MLT __cexp_ea
P_LOD __cexp_im
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_ca
F_MLT main_one
P_LOD main_ca_i
F_MLT main_one_i
SF_SU2
P_LOD main_ca
F_MLT main_one_i
P_LOD main_ca_i
F_MLT main_one
SF_ADD
SET_P __cexp_b
CAL __float_exp
SET __cexp_ea
LOD __cexp_b
CAL __float_sin
F_MLT __cexp_ea
SET __cexp_im
LOD __cexp_b
F_SU2 1.570796327
CAL __float_sin
F_MLT __cexp_ea
P_LOD __cexp_im
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD 0.5
CAL __float_exp
SET __cexp_ea
LOD 0.25
CAL __float_sin
F_MLT __cexp_ea
SET __cexp_im
LOD 0.25
F_SU2 1.570796327
CAL __float_sin
F_MLT __cexp_ea
P_LOD __cexp_im
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD 0.5
CAL __float_exp
SET __cexp_ea
LOD 0.25
CAL __float_sin
F_MLT __cexp_ea
SET __cexp_im
LOD 0.25
F_SU2 1.570796327
CAL __float_sin
F_MLT __cexp_ea
P_LOD __cexp_im
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD 3
SET main_ia
LOD 2.5
SET main_fa
LOD 1.0
SET main_ca
SET main_ca_i
I2F_M main_ia
CAL __float_log
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_I2F_M main_ia
CAL __float_log
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ia
ADD main_z0
I2F
CAL __float_log
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_ia
ADD main_z0
I2F
CAL __float_log
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 3.0
CAL __float_log
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 3.0
CAL __float_log
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
CAL __float_log
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
CAL __float_log
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
F_ADD main_fz
CAL __float_log
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
F_ADD main_fz
CAL __float_log
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 2.5
CAL __float_log
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 2.5
CAL __float_log
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ca
F_MLT main_ca
P_LOD main_ca_i
F_MLT main_ca_i
SF_ADD
CAL __float_log
F_MLT 0.5
SET __clog_re
LOD main_ca
P_LOD main_ca_i
SET_P __fase_im
SET __fase_re
LOD 0.0
F_LES __fase_re
JIZ __Lfa1a
LOD __fase_re
F_DIV __fase_im
NOP
NOP
F_QUO
CAL __float_atan
SET __fase_t
LOD 0.0
F_LES __fase_im
JIZ __Lfa1b
LOD __fase_t
F_ADD -3.14159265359
JMP __Lfa1z
@__Lfa1b LOD __fase_t
F_ADD 3.14159265359
JMP __Lfa1z
@__Lfa1a LOD __fase_re
F_LES 0.0
JIZ __Lfa1c
LOD __fase_re
F_DIV __fase_im
NOP
NOP
F_QUO
CAL __float_atan
JMP __Lfa1z
@__Lfa1c LOD __fase_im
F_LES 0.0
JIZ __Lfa1d
LOD 1.57079632679
JMP __Lfa1z
@__Lfa1d LOD 0.0
F_LES __fase_im
JIZ __Lfa1e
LOD -1.57079632679
JMP __Lfa1z
@__Lfa1e LOD 0.0
@__Lfa1z SET __clog_im
LOD __clog_re
P_LOD __clog_im
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_ca
F_MLT main_ca
P_LOD main_ca_i
F_MLT main_ca_i
SF_ADD
CAL __float_log
F_MLT 0.5
SET __clog_re
LOD main_ca
P_LOD main_ca_i
SET_P __fase_im
SET __fase_re
LOD 0.0
F_LES __fase_re
JIZ __Lfa2a
LOD __fase_re
F_DIV __fase_im
NOP
NOP
F_QUO
CAL __float_atan
SET __fase_t
LOD 0.0
F_LES __fase_im
JIZ __Lfa2b
LOD __fase_t
F_ADD -3.14159265359
JMP __Lfa2z
@__Lfa2b LOD __fase_t
F_ADD 3.14159265359
JMP __Lfa2z
@__Lfa2a LOD __fase_re
F_LES 0.0
JIZ __Lfa2c
LOD __fase_re
F_DIV __fase_im
NOP
NOP
F_QUO
CAL __float_atan
JMP __Lfa2z
@__Lfa2c LOD __fase_im
F_LES 0.0
JIZ __Lfa2d
LOD 1.57079632679
JMP __Lfa2z
@__Lfa2d LOD 0.0
F_LES __fase_im
JIZ __Lfa2e
LOD -1.57079632679
JMP __Lfa2z
@__Lfa2e LOD 0.0
@__Lfa2z SET __clog_im
LOD __clog_re
P_LOD __clog_im
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD main_ca
F_MLT main_one
P_LOD main_ca_i
F_MLT main_one_i
SF_SU2
P_LOD main_ca
F_MLT main_one_i
P_LOD main_ca_i
F_MLT main_one
SF_ADD
SET_P __clog_b
SET   __clog_a
F_MLT __clog_a
P_LOD __clog_b
F_MLT __clog_b
SF_ADD
CAL __float_log
F_MLT 0.5
SET __clog_re
LOD __clog_a
P_LOD __clog_b
SET_P __fase_im
SET __fase_re
LOD 0.0
F_LES __fase_re
JIZ __Lfa3a
LOD __fase_re
F_DIV __fase_im
NOP
NOP
F_QUO
CAL __float_atan
SET __fase_t
LOD 0.0
F_LES __fase_im
JIZ __Lfa3b
LOD __fase_t
F_ADD -3.14159265359
JMP __Lfa3z
@__Lfa3b LOD __fase_t
F_ADD 3.14159265359
JMP __Lfa3z
@__Lfa3a LOD __fase_re
F_LES 0.0
JIZ __Lfa3c
LOD __fase_re
F_DIV __fase_im
NOP
NOP
F_QUO
CAL __float_atan
JMP __Lfa3z
@__Lfa3c LOD __fase_im
F_LES 0.0
JIZ __Lfa3d
LOD 1.57079632679
JMP __Lfa3z
@__Lfa3d LOD 0.0
F_LES __fase_im
JIZ __Lfa3e
LOD -1.57079632679
JMP __Lfa3z
@__Lfa3e LOD 0.0
@__Lfa3z SET __clog_im
LOD __clog_re
P_LOD __clog_im
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_ca
F_MLT main_one
P_LOD main_ca_i
F_MLT main_one_i
SF_SU2
P_LOD main_ca
F_MLT main_one_i
P_LOD main_ca_i
F_MLT main_one
SF_ADD
SET_P __clog_b
SET   __clog_a
F_MLT __clog_a
P_LOD __clog_b
F_MLT __clog_b
SF_ADD
CAL __float_log
F_MLT 0.5
SET __clog_re
LOD __clog_a
P_LOD __clog_b
SET_P __fase_im
SET __fase_re
LOD 0.0
F_LES __fase_re
JIZ __Lfa4a
LOD __fase_re
F_DIV __fase_im
NOP
NOP
F_QUO
CAL __float_atan
SET __fase_t
LOD 0.0
F_LES __fase_im
JIZ __Lfa4b
LOD __fase_t
F_ADD -3.14159265359
JMP __Lfa4z
@__Lfa4b LOD __fase_t
F_ADD 3.14159265359
JMP __Lfa4z
@__Lfa4a LOD __fase_re
F_LES 0.0
JIZ __Lfa4c
LOD __fase_re
F_DIV __fase_im
NOP
NOP
F_QUO
CAL __float_atan
JMP __Lfa4z
@__Lfa4c LOD __fase_im
F_LES 0.0
JIZ __Lfa4d
LOD 1.57079632679
JMP __Lfa4z
@__Lfa4d LOD 0.0
F_LES __fase_im
JIZ __Lfa4e
LOD -1.57079632679
JMP __Lfa4z
@__Lfa4e LOD 0.0
@__Lfa4z SET __clog_im
LOD __clog_re
P_LOD __clog_im
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD 1.0
P_LOD 1.0
SF_ADD
CAL __float_log
F_MLT 0.5
SET __clog_re
LOD 1.0
P_LOD 1.0
SET_P __fase_im
SET __fase_re
LOD 0.0
F_LES __fase_re
JIZ __Lfa5a
LOD __fase_re
F_DIV __fase_im
NOP
NOP
F_QUO
CAL __float_atan
SET __fase_t
LOD 0.0
F_LES __fase_im
JIZ __Lfa5b
LOD __fase_t
F_ADD -3.14159265359
JMP __Lfa5z
@__Lfa5b LOD __fase_t
F_ADD 3.14159265359
JMP __Lfa5z
@__Lfa5a LOD __fase_re
F_LES 0.0
JIZ __Lfa5c
LOD __fase_re
F_DIV __fase_im
NOP
NOP
F_QUO
CAL __float_atan
JMP __Lfa5z
@__Lfa5c LOD __fase_im
F_LES 0.0
JIZ __Lfa5d
LOD 1.57079632679
JMP __Lfa5z
@__Lfa5d LOD 0.0
F_LES __fase_im
JIZ __Lfa5e
LOD -1.57079632679
JMP __Lfa5z
@__Lfa5e LOD 0.0
@__Lfa5z SET __clog_im
LOD __clog_re
P_LOD __clog_im
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD 1.0
P_LOD 1.0
SF_ADD
CAL __float_log
F_MLT 0.5
SET __clog_re
LOD 1.0
P_LOD 1.0
SET_P __fase_im
SET __fase_re
LOD 0.0
F_LES __fase_re
JIZ __Lfa6a
LOD __fase_re
F_DIV __fase_im
NOP
NOP
F_QUO
CAL __float_atan
SET __fase_t
LOD 0.0
F_LES __fase_im
JIZ __Lfa6b
LOD __fase_t
F_ADD -3.14159265359
JMP __Lfa6z
@__Lfa6b LOD __fase_t
F_ADD 3.14159265359
JMP __Lfa6z
@__Lfa6a LOD __fase_re
F_LES 0.0
JIZ __Lfa6c
LOD __fase_re
F_DIV __fase_im
NOP
NOP
F_QUO
CAL __float_atan
JMP __Lfa6z
@__Lfa6c LOD __fase_im
F_LES 0.0
JIZ __Lfa6d
LOD 1.57079632679
JMP __Lfa6z
@__Lfa6d LOD 0.0
F_LES __fase_im
JIZ __Lfa6e
LOD -1.57079632679
JMP __Lfa6z
@__Lfa6e LOD 0.0
@__Lfa6z SET __clog_im
LOD __clog_re
P_LOD __clog_im
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD 1
SET main_ia
LOD 0.5
SET main_fa
SET main_ca
LOD 0.25
SET main_ca_i
I2F_M main_ia
CAL __float_sin
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_I2F_M main_ia
CAL __float_sin
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ia
ADD main_z0
I2F
CAL __float_sin
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_ia
ADD main_z0
I2F
CAL __float_sin
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 1.0
CAL __float_sin
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 1.0
CAL __float_sin
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
CAL __float_sin
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
CAL __float_sin
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
F_ADD main_fz
CAL __float_sin
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
F_ADD main_fz
CAL __float_sin
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 0.5
CAL __float_sin
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 0.5
CAL __float_sin
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ca_i
CAL __float_exp
SET __csin_eb
F_DIV 1.0
NOP
NOP
F_QUO
SET __csin_emb
F_ADD __csin_eb
F_MLT 0.5
SET __csin_chb
LOD __csin_eb
F_SU1 __csin_emb
F_MLT 0.5
SET __csin_shb
LOD main_ca
F_SU2 1.570796327
CAL __float_sin
SET __csin_ca
LOD main_ca
CAL __float_sin
F_MLT __csin_chb
P_LOD __csin_ca
F_MLT __csin_shb
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_ca_i
CAL __float_exp
SET __csin_eb
F_DIV 1.0
NOP
NOP
F_QUO
SET __csin_emb
F_ADD __csin_eb
F_MLT 0.5
SET __csin_chb
LOD __csin_eb
F_SU1 __csin_emb
F_MLT 0.5
SET __csin_shb
LOD main_ca
F_SU2 1.570796327
CAL __float_sin
SET __csin_ca
LOD main_ca
CAL __float_sin
F_MLT __csin_chb
P_LOD __csin_ca
F_MLT __csin_shb
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD main_ca
F_MLT main_one
P_LOD main_ca_i
F_MLT main_one_i
SF_SU2
P_LOD main_ca
F_MLT main_one_i
P_LOD main_ca_i
F_MLT main_one
SF_ADD
SET_P __csin_b
SET   __csin_a
LOD __csin_b
CAL __float_exp
SET __csin_eb
F_DIV 1.0
NOP
NOP
F_QUO
SET __csin_emb
F_ADD __csin_eb
F_MLT 0.5
SET __csin_chb
LOD __csin_eb
F_SU1 __csin_emb
F_MLT 0.5
SET __csin_shb
LOD __csin_a
F_SU2 1.570796327
CAL __float_sin
SET __csin_ca
LOD __csin_a
CAL __float_sin
F_MLT __csin_chb
P_LOD __csin_ca
F_MLT __csin_shb
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_ca
F_MLT main_one
P_LOD main_ca_i
F_MLT main_one_i
SF_SU2
P_LOD main_ca
F_MLT main_one_i
P_LOD main_ca_i
F_MLT main_one
SF_ADD
SET_P __csin_b
SET   __csin_a
LOD __csin_b
CAL __float_exp
SET __csin_eb
F_DIV 1.0
NOP
NOP
F_QUO
SET __csin_emb
F_ADD __csin_eb
F_MLT 0.5
SET __csin_chb
LOD __csin_eb
F_SU1 __csin_emb
F_MLT 0.5
SET __csin_shb
LOD __csin_a
F_SU2 1.570796327
CAL __float_sin
SET __csin_ca
LOD __csin_a
CAL __float_sin
F_MLT __csin_chb
P_LOD __csin_ca
F_MLT __csin_shb
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD 0.25
CAL __float_exp
SET __csin_eb
F_DIV 1.0
NOP
NOP
F_QUO
SET __csin_emb
F_ADD __csin_eb
F_MLT 0.5
SET __csin_chb
LOD __csin_eb
F_SU1 __csin_emb
F_MLT 0.5
SET __csin_shb
LOD 0.5
F_SU2 1.570796327
CAL __float_sin
SET __csin_ca
LOD 0.5
CAL __float_sin
F_MLT __csin_chb
P_LOD __csin_ca
F_MLT __csin_shb
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD 0.25
CAL __float_exp
SET __csin_eb
F_DIV 1.0
NOP
NOP
F_QUO
SET __csin_emb
F_ADD __csin_eb
F_MLT 0.5
SET __csin_chb
LOD __csin_eb
F_SU1 __csin_emb
F_MLT 0.5
SET __csin_shb
LOD 0.5
F_SU2 1.570796327
CAL __float_sin
SET __csin_ca
LOD 0.5
CAL __float_sin
F_MLT __csin_chb
P_LOD __csin_ca
F_MLT __csin_shb
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD 1
SET main_ia
LOD 0.5
SET main_fa
SET main_ca
LOD 0.25
SET main_ca_i
I2F_M main_ia
F_SU2 1.570796327
CAL __float_sin
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_I2F_M main_ia
F_SU2 1.570796327
CAL __float_sin
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ia
ADD main_z0
I2F
F_SU2 1.570796327
CAL __float_sin
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_ia
ADD main_z0
I2F
F_SU2 1.570796327
CAL __float_sin
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 1.0
F_SU2 1.570796327
CAL __float_sin
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 1.0
F_SU2 1.570796327
CAL __float_sin
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
F_SU2 1.570796327
CAL __float_sin
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
F_SU2 1.570796327
CAL __float_sin
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
F_ADD main_fz
F_SU2 1.570796327
CAL __float_sin
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
F_ADD main_fz
F_SU2 1.570796327
CAL __float_sin
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 0.5
F_SU2 1.570796327
CAL __float_sin
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 0.5
F_SU2 1.570796327
CAL __float_sin
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ca_i
CAL __float_exp
SET __ccos_eb
F_DIV 1.0
NOP
NOP
F_QUO
SET __ccos_emb
F_ADD __ccos_eb
F_MLT 0.5
SET __ccos_chb
LOD __ccos_eb
F_SU1 __ccos_emb
F_MLT 0.5
SET __ccos_shb
LOD main_ca
CAL __float_sin
SET __ccos_sa
LOD main_ca
F_SU2 1.570796327
CAL __float_sin
F_MLT __ccos_chb
P_LOD __ccos_sa
F_MLT __ccos_shb
F_NEG
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_ca_i
CAL __float_exp
SET __ccos_eb
F_DIV 1.0
NOP
NOP
F_QUO
SET __ccos_emb
F_ADD __ccos_eb
F_MLT 0.5
SET __ccos_chb
LOD __ccos_eb
F_SU1 __ccos_emb
F_MLT 0.5
SET __ccos_shb
LOD main_ca
CAL __float_sin
SET __ccos_sa
LOD main_ca
F_SU2 1.570796327
CAL __float_sin
F_MLT __ccos_chb
P_LOD __ccos_sa
F_MLT __ccos_shb
F_NEG
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD main_ca
F_MLT main_one
P_LOD main_ca_i
F_MLT main_one_i
SF_SU2
P_LOD main_ca
F_MLT main_one_i
P_LOD main_ca_i
F_MLT main_one
SF_ADD
SET_P __ccos_b
SET   __ccos_a
LOD __ccos_b
CAL __float_exp
SET __ccos_eb
F_DIV 1.0
NOP
NOP
F_QUO
SET __ccos_emb
F_ADD __ccos_eb
F_MLT 0.5
SET __ccos_chb
LOD __ccos_eb
F_SU1 __ccos_emb
F_MLT 0.5
SET __ccos_shb
LOD __ccos_a
CAL __float_sin
SET __ccos_sa
LOD __ccos_a
F_SU2 1.570796327
CAL __float_sin
F_MLT __ccos_chb
P_LOD __ccos_sa
F_MLT __ccos_shb
F_NEG
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_ca
F_MLT main_one
P_LOD main_ca_i
F_MLT main_one_i
SF_SU2
P_LOD main_ca
F_MLT main_one_i
P_LOD main_ca_i
F_MLT main_one
SF_ADD
SET_P __ccos_b
SET   __ccos_a
LOD __ccos_b
CAL __float_exp
SET __ccos_eb
F_DIV 1.0
NOP
NOP
F_QUO
SET __ccos_emb
F_ADD __ccos_eb
F_MLT 0.5
SET __ccos_chb
LOD __ccos_eb
F_SU1 __ccos_emb
F_MLT 0.5
SET __ccos_shb
LOD __ccos_a
CAL __float_sin
SET __ccos_sa
LOD __ccos_a
F_SU2 1.570796327
CAL __float_sin
F_MLT __ccos_chb
P_LOD __ccos_sa
F_MLT __ccos_shb
F_NEG
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD 0.25
CAL __float_exp
SET __ccos_eb
F_DIV 1.0
NOP
NOP
F_QUO
SET __ccos_emb
F_ADD __ccos_eb
F_MLT 0.5
SET __ccos_chb
LOD __ccos_eb
F_SU1 __ccos_emb
F_MLT 0.5
SET __ccos_shb
LOD 0.5
CAL __float_sin
SET __ccos_sa
LOD 0.5
F_SU2 1.570796327
CAL __float_sin
F_MLT __ccos_chb
P_LOD __ccos_sa
F_MLT __ccos_shb
F_NEG
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD 0.25
CAL __float_exp
SET __ccos_eb
F_DIV 1.0
NOP
NOP
F_QUO
SET __ccos_emb
F_ADD __ccos_eb
F_MLT 0.5
SET __ccos_chb
LOD __ccos_eb
F_SU1 __ccos_emb
F_MLT 0.5
SET __ccos_shb
LOD 0.5
CAL __float_sin
SET __ccos_sa
LOD 0.5
F_SU2 1.570796327
CAL __float_sin
F_MLT __ccos_chb
P_LOD __ccos_sa
F_MLT __ccos_shb
F_NEG
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD 1
SET main_ia
LOD 0.5
SET main_fa
SET main_ca
LOD 0.25
SET main_ca_i
I2F_M main_ia
CAL __float_tan
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_I2F_M main_ia
CAL __float_tan
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ia
ADD main_z0
I2F
CAL __float_tan
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_ia
ADD main_z0
I2F
CAL __float_tan
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 1.0
CAL __float_tan
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 1.0
CAL __float_tan
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
CAL __float_tan
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
CAL __float_tan
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
F_ADD main_fz
CAL __float_tan
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
F_ADD main_fz
CAL __float_tan
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 0.5
CAL __float_tan
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 0.5
CAL __float_tan
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ca
F_MLT 2.0
SET __ctan_ta
LOD main_ca_i
F_MLT 2.0
SET __ctan_tb
LOD __ctan_ta
CAL __float_sin
SET __ctan_s2a
LOD __ctan_ta
F_SU2 1.570796327
CAL __float_sin
SET __ctan_c2a
LOD __ctan_tb
CAL __float_exp
SET __ctan_eb
F_DIV 1.0
NOP
NOP
F_QUO
SET __ctan_emb
F_ADD __ctan_eb
F_MLT 0.5
SET __ctan_chb
LOD __ctan_eb
F_SU1 __ctan_emb
F_MLT 0.5
SET __ctan_shb
LOD __ctan_c2a
F_ADD __ctan_chb
SET __ctan_d
F_DIV __ctan_s2a
NOP
NOP
F_QUO
P_LOD __ctan_d
F_DIV __ctan_shb
NOP
NOP
F_QUO
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_ca
F_MLT 2.0
SET __ctan_ta
LOD main_ca_i
F_MLT 2.0
SET __ctan_tb
LOD __ctan_ta
CAL __float_sin
SET __ctan_s2a
LOD __ctan_ta
F_SU2 1.570796327
CAL __float_sin
SET __ctan_c2a
LOD __ctan_tb
CAL __float_exp
SET __ctan_eb
F_DIV 1.0
NOP
NOP
F_QUO
SET __ctan_emb
F_ADD __ctan_eb
F_MLT 0.5
SET __ctan_chb
LOD __ctan_eb
F_SU1 __ctan_emb
F_MLT 0.5
SET __ctan_shb
LOD __ctan_c2a
F_ADD __ctan_chb
SET __ctan_d
F_DIV __ctan_s2a
NOP
NOP
F_QUO
P_LOD __ctan_d
F_DIV __ctan_shb
NOP
NOP
F_QUO
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD main_ca
F_MLT main_one
P_LOD main_ca_i
F_MLT main_one_i
SF_SU2
P_LOD main_ca
F_MLT main_one_i
P_LOD main_ca_i
F_MLT main_one
SF_ADD
SET_P __ctan_b
F_MLT 2.0
SET __ctan_ta
LOD __ctan_b
F_MLT 2.0
SET __ctan_tb
LOD __ctan_ta
CAL __float_sin
SET __ctan_s2a
LOD __ctan_ta
F_SU2 1.570796327
CAL __float_sin
SET __ctan_c2a
LOD __ctan_tb
CAL __float_exp
SET __ctan_eb
F_DIV 1.0
NOP
NOP
F_QUO
SET __ctan_emb
F_ADD __ctan_eb
F_MLT 0.5
SET __ctan_chb
LOD __ctan_eb
F_SU1 __ctan_emb
F_MLT 0.5
SET __ctan_shb
LOD __ctan_c2a
F_ADD __ctan_chb
SET __ctan_d
F_DIV __ctan_s2a
NOP
NOP
F_QUO
P_LOD __ctan_d
F_DIV __ctan_shb
NOP
NOP
F_QUO
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_ca
F_MLT main_one
P_LOD main_ca_i
F_MLT main_one_i
SF_SU2
P_LOD main_ca
F_MLT main_one_i
P_LOD main_ca_i
F_MLT main_one
SF_ADD
SET_P __ctan_b
F_MLT 2.0
SET __ctan_ta
LOD __ctan_b
F_MLT 2.0
SET __ctan_tb
LOD __ctan_ta
CAL __float_sin
SET __ctan_s2a
LOD __ctan_ta
F_SU2 1.570796327
CAL __float_sin
SET __ctan_c2a
LOD __ctan_tb
CAL __float_exp
SET __ctan_eb
F_DIV 1.0
NOP
NOP
F_QUO
SET __ctan_emb
F_ADD __ctan_eb
F_MLT 0.5
SET __ctan_chb
LOD __ctan_eb
F_SU1 __ctan_emb
F_MLT 0.5
SET __ctan_shb
LOD __ctan_c2a
F_ADD __ctan_chb
SET __ctan_d
F_DIV __ctan_s2a
NOP
NOP
F_QUO
P_LOD __ctan_d
F_DIV __ctan_shb
NOP
NOP
F_QUO
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD 0.5
F_MLT 2.0
SET __ctan_ta
LOD 0.25
F_MLT 2.0
SET __ctan_tb
LOD __ctan_ta
CAL __float_sin
SET __ctan_s2a
LOD __ctan_ta
F_SU2 1.570796327
CAL __float_sin
SET __ctan_c2a
LOD __ctan_tb
CAL __float_exp
SET __ctan_eb
F_DIV 1.0
NOP
NOP
F_QUO
SET __ctan_emb
F_ADD __ctan_eb
F_MLT 0.5
SET __ctan_chb
LOD __ctan_eb
F_SU1 __ctan_emb
F_MLT 0.5
SET __ctan_shb
LOD __ctan_c2a
F_ADD __ctan_chb
SET __ctan_d
F_DIV __ctan_s2a
NOP
NOP
F_QUO
P_LOD __ctan_d
F_DIV __ctan_shb
NOP
NOP
F_QUO
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD 0.5
F_MLT 2.0
SET __ctan_ta
LOD 0.25
F_MLT 2.0
SET __ctan_tb
LOD __ctan_ta
CAL __float_sin
SET __ctan_s2a
LOD __ctan_ta
F_SU2 1.570796327
CAL __float_sin
SET __ctan_c2a
LOD __ctan_tb
CAL __float_exp
SET __ctan_eb
F_DIV 1.0
NOP
NOP
F_QUO
SET __ctan_emb
F_ADD __ctan_eb
F_MLT 0.5
SET __ctan_chb
LOD __ctan_eb
F_SU1 __ctan_emb
F_MLT 0.5
SET __ctan_shb
LOD __ctan_c2a
F_ADD __ctan_chb
SET __ctan_d
F_DIV __ctan_s2a
NOP
NOP
F_QUO
P_LOD __ctan_d
F_DIV __ctan_shb
NOP
NOP
F_QUO
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD 2
SET main_ia
LOD 0.5
SET main_fa
SET main_ca
LOD 0.25
SET main_ca_i
I2F_M main_ia
CAL __float_atan
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_I2F_M main_ia
CAL __float_atan
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ia
ADD main_z0
I2F
CAL __float_atan
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_ia
ADD main_z0
I2F
CAL __float_atan
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 2.0
CAL __float_atan
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 2.0
CAL __float_atan
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
CAL __float_atan
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
CAL __float_atan
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
F_ADD main_fz
CAL __float_atan
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
F_ADD main_fz
CAL __float_atan
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 0.5
CAL __float_atan
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 0.5
CAL __float_atan
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ca
F_MLT main_ca
SET __catan_a2
LOD main_ca_i
F_MLT main_ca_i
F_ADD __catan_a2
F_SU2 1.0
P_LOD main_ca
F_MLT 2.0
SET_P __fase_im
SET __fase_re
LOD 0.0
F_LES __fase_re
JIZ __Lfa7a
LOD __fase_re
F_DIV __fase_im
NOP
NOP
F_QUO
CAL __float_atan
SET __fase_t
LOD 0.0
F_LES __fase_im
JIZ __Lfa7b
LOD __fase_t
F_ADD -3.14159265359
JMP __Lfa7z
@__Lfa7b LOD __fase_t
F_ADD 3.14159265359
JMP __Lfa7z
@__Lfa7a LOD __fase_re
F_LES 0.0
JIZ __Lfa7c
LOD __fase_re
F_DIV __fase_im
NOP
NOP
F_QUO
CAL __float_atan
JMP __Lfa7z
@__Lfa7c LOD __fase_im
F_LES 0.0
JIZ __Lfa7d
LOD 1.57079632679
JMP __Lfa7z
@__Lfa7d LOD 0.0
F_LES __fase_im
JIZ __Lfa7e
LOD -1.57079632679
JMP __Lfa7z
@__Lfa7e LOD 0.0
@__Lfa7z F_MLT 0.5
SET __catan_re
LOD main_ca_i
F_ADD 1.0
SET __catan_bp
F_MLT __catan_bp
F_ADD __catan_a2
SET __catan_num
LOD main_ca_i
F_ADD -1.0
SET __catan_bm
F_MLT __catan_bm
F_ADD __catan_a2
F_DIV __catan_num
NOP
NOP
F_QUO
CAL __float_log
F_MLT 0.25
SET __catan_im
LOD __catan_re
P_LOD __catan_im
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_ca
F_MLT main_ca
SET __catan_a2
LOD main_ca_i
F_MLT main_ca_i
F_ADD __catan_a2
F_SU2 1.0
P_LOD main_ca
F_MLT 2.0
SET_P __fase_im
SET __fase_re
LOD 0.0
F_LES __fase_re
JIZ __Lfa8a
LOD __fase_re
F_DIV __fase_im
NOP
NOP
F_QUO
CAL __float_atan
SET __fase_t
LOD 0.0
F_LES __fase_im
JIZ __Lfa8b
LOD __fase_t
F_ADD -3.14159265359
JMP __Lfa8z
@__Lfa8b LOD __fase_t
F_ADD 3.14159265359
JMP __Lfa8z
@__Lfa8a LOD __fase_re
F_LES 0.0
JIZ __Lfa8c
LOD __fase_re
F_DIV __fase_im
NOP
NOP
F_QUO
CAL __float_atan
JMP __Lfa8z
@__Lfa8c LOD __fase_im
F_LES 0.0
JIZ __Lfa8d
LOD 1.57079632679
JMP __Lfa8z
@__Lfa8d LOD 0.0
F_LES __fase_im
JIZ __Lfa8e
LOD -1.57079632679
JMP __Lfa8z
@__Lfa8e LOD 0.0
@__Lfa8z F_MLT 0.5
SET __catan_re
LOD main_ca_i
F_ADD 1.0
SET __catan_bp
F_MLT __catan_bp
F_ADD __catan_a2
SET __catan_num
LOD main_ca_i
F_ADD -1.0
SET __catan_bm
F_MLT __catan_bm
F_ADD __catan_a2
F_DIV __catan_num
NOP
NOP
F_QUO
CAL __float_log
F_MLT 0.25
SET __catan_im
LOD __catan_re
P_LOD __catan_im
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD main_ca
F_MLT main_one
P_LOD main_ca_i
F_MLT main_one_i
SF_SU2
P_LOD main_ca
F_MLT main_one_i
P_LOD main_ca_i
F_MLT main_one
SF_ADD
SET_P __catan_b
SET   __catan_a
F_MLT __catan_a
SET __catan_a2
LOD __catan_b
F_MLT __catan_b
F_ADD __catan_a2
F_SU2 1.0
P_LOD __catan_a
F_MLT 2.0
SET_P __fase_im
SET __fase_re
LOD 0.0
F_LES __fase_re
JIZ __Lfa9a
LOD __fase_re
F_DIV __fase_im
NOP
NOP
F_QUO
CAL __float_atan
SET __fase_t
LOD 0.0
F_LES __fase_im
JIZ __Lfa9b
LOD __fase_t
F_ADD -3.14159265359
JMP __Lfa9z
@__Lfa9b LOD __fase_t
F_ADD 3.14159265359
JMP __Lfa9z
@__Lfa9a LOD __fase_re
F_LES 0.0
JIZ __Lfa9c
LOD __fase_re
F_DIV __fase_im
NOP
NOP
F_QUO
CAL __float_atan
JMP __Lfa9z
@__Lfa9c LOD __fase_im
F_LES 0.0
JIZ __Lfa9d
LOD 1.57079632679
JMP __Lfa9z
@__Lfa9d LOD 0.0
F_LES __fase_im
JIZ __Lfa9e
LOD -1.57079632679
JMP __Lfa9z
@__Lfa9e LOD 0.0
@__Lfa9z F_MLT 0.5
SET __catan_re
LOD __catan_b
F_ADD 1.0
SET __catan_bp
F_MLT __catan_bp
F_ADD __catan_a2
SET __catan_num
LOD __catan_b
F_ADD -1.0
SET __catan_bm
F_MLT __catan_bm
F_ADD __catan_a2
F_DIV __catan_num
NOP
NOP
F_QUO
CAL __float_log
F_MLT 0.25
SET __catan_im
LOD __catan_re
P_LOD __catan_im
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_ca
F_MLT main_one
P_LOD main_ca_i
F_MLT main_one_i
SF_SU2
P_LOD main_ca
F_MLT main_one_i
P_LOD main_ca_i
F_MLT main_one
SF_ADD
SET_P __catan_b
SET   __catan_a
F_MLT __catan_a
SET __catan_a2
LOD __catan_b
F_MLT __catan_b
F_ADD __catan_a2
F_SU2 1.0
P_LOD __catan_a
F_MLT 2.0
SET_P __fase_im
SET __fase_re
LOD 0.0
F_LES __fase_re
JIZ __Lfa10a
LOD __fase_re
F_DIV __fase_im
NOP
NOP
F_QUO
CAL __float_atan
SET __fase_t
LOD 0.0
F_LES __fase_im
JIZ __Lfa10b
LOD __fase_t
F_ADD -3.14159265359
JMP __Lfa10z
@__Lfa10b LOD __fase_t
F_ADD 3.14159265359
JMP __Lfa10z
@__Lfa10a LOD __fase_re
F_LES 0.0
JIZ __Lfa10c
LOD __fase_re
F_DIV __fase_im
NOP
NOP
F_QUO
CAL __float_atan
JMP __Lfa10z
@__Lfa10c LOD __fase_im
F_LES 0.0
JIZ __Lfa10d
LOD 1.57079632679
JMP __Lfa10z
@__Lfa10d LOD 0.0
F_LES __fase_im
JIZ __Lfa10e
LOD -1.57079632679
JMP __Lfa10z
@__Lfa10e LOD 0.0
@__Lfa10z F_MLT 0.5
SET __catan_re
LOD __catan_b
F_ADD 1.0
SET __catan_bp
F_MLT __catan_bp
F_ADD __catan_a2
SET __catan_num
LOD __catan_b
F_ADD -1.0
SET __catan_bm
F_MLT __catan_bm
F_ADD __catan_a2
F_DIV __catan_num
NOP
NOP
F_QUO
CAL __float_log
F_MLT 0.25
SET __catan_im
LOD __catan_re
P_LOD __catan_im
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD 0.5
F_MLT 0.5
SET __catan_a2
LOD 0.25
F_MLT 0.25
F_ADD __catan_a2
F_SU2 1.0
P_LOD 0.5
F_MLT 2.0
SET_P __fase_im
SET __fase_re
LOD 0.0
F_LES __fase_re
JIZ __Lfa11a
LOD __fase_re
F_DIV __fase_im
NOP
NOP
F_QUO
CAL __float_atan
SET __fase_t
LOD 0.0
F_LES __fase_im
JIZ __Lfa11b
LOD __fase_t
F_ADD -3.14159265359
JMP __Lfa11z
@__Lfa11b LOD __fase_t
F_ADD 3.14159265359
JMP __Lfa11z
@__Lfa11a LOD __fase_re
F_LES 0.0
JIZ __Lfa11c
LOD __fase_re
F_DIV __fase_im
NOP
NOP
F_QUO
CAL __float_atan
JMP __Lfa11z
@__Lfa11c LOD __fase_im
F_LES 0.0
JIZ __Lfa11d
LOD 1.57079632679
JMP __Lfa11z
@__Lfa11d LOD 0.0
F_LES __fase_im
JIZ __Lfa11e
LOD -1.57079632679
JMP __Lfa11z
@__Lfa11e LOD 0.0
@__Lfa11z F_MLT 0.5
SET __catan_re
LOD 0.25
F_ADD 1.0
SET __catan_bp
F_MLT __catan_bp
F_ADD __catan_a2
SET __catan_num
LOD 0.25
F_ADD -1.0
SET __catan_bm
F_MLT __catan_bm
F_ADD __catan_a2
F_DIV __catan_num
NOP
NOP
F_QUO
CAL __float_log
F_MLT 0.25
SET __catan_im
LOD __catan_re
P_LOD __catan_im
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD 0.5
F_MLT 0.5
SET __catan_a2
LOD 0.25
F_MLT 0.25
F_ADD __catan_a2
F_SU2 1.0
P_LOD 0.5
F_MLT 2.0
SET_P __fase_im
SET __fase_re
LOD 0.0
F_LES __fase_re
JIZ __Lfa12a
LOD __fase_re
F_DIV __fase_im
NOP
NOP
F_QUO
CAL __float_atan
SET __fase_t
LOD 0.0
F_LES __fase_im
JIZ __Lfa12b
LOD __fase_t
F_ADD -3.14159265359
JMP __Lfa12z
@__Lfa12b LOD __fase_t
F_ADD 3.14159265359
JMP __Lfa12z
@__Lfa12a LOD __fase_re
F_LES 0.0
JIZ __Lfa12c
LOD __fase_re
F_DIV __fase_im
NOP
NOP
F_QUO
CAL __float_atan
JMP __Lfa12z
@__Lfa12c LOD __fase_im
F_LES 0.0
JIZ __Lfa12d
LOD 1.57079632679
JMP __Lfa12z
@__Lfa12d LOD 0.0
F_LES __fase_im
JIZ __Lfa12e
LOD -1.57079632679
JMP __Lfa12z
@__Lfa12e LOD 0.0
@__Lfa12z F_MLT 0.5
SET __catan_re
LOD 0.25
F_ADD 1.0
SET __catan_bp
F_MLT __catan_bp
F_ADD __catan_a2
SET __catan_num
LOD 0.25
F_ADD -1.0
SET __catan_bm
F_MLT __catan_bm
F_ADD __catan_a2
F_DIV __catan_num
NOP
NOP
F_QUO
CAL __float_log
F_MLT 0.25
SET __catan_im
LOD __catan_re
P_LOD __catan_im
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD 1
SET main_ia
LOD 0.5
SET main_fa
LOD 0.0
SET main_ca
SET main_ca_i
I2F_M main_ia
SET __sinh_x
CAL __float_exp
SET __sinh_t
F_NEG_M __sinh_x
CAL __float_exp
F_SU2 __sinh_t
F_MLT 0.5
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_I2F_M main_ia
SET __sinh_x
CAL __float_exp
SET __sinh_t
F_NEG_M __sinh_x
CAL __float_exp
F_SU2 __sinh_t
F_MLT 0.5
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ia
ADD main_z0
I2F
SET __sinh_x
CAL __float_exp
SET __sinh_t
F_NEG_M __sinh_x
CAL __float_exp
F_SU2 __sinh_t
F_MLT 0.5
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_ia
ADD main_z0
I2F
SET __sinh_x
CAL __float_exp
SET __sinh_t
F_NEG_M __sinh_x
CAL __float_exp
F_SU2 __sinh_t
F_MLT 0.5
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 1.0
SET __sinh_x
CAL __float_exp
SET __sinh_t
F_NEG_M __sinh_x
CAL __float_exp
F_SU2 __sinh_t
F_MLT 0.5
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 1.0
SET __sinh_x
CAL __float_exp
SET __sinh_t
F_NEG_M __sinh_x
CAL __float_exp
F_SU2 __sinh_t
F_MLT 0.5
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
SET __sinh_x
CAL __float_exp
SET __sinh_t
F_NEG_M __sinh_x
CAL __float_exp
F_SU2 __sinh_t
F_MLT 0.5
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
SET __sinh_x
CAL __float_exp
SET __sinh_t
F_NEG_M __sinh_x
CAL __float_exp
F_SU2 __sinh_t
F_MLT 0.5
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
F_ADD main_fz
SET __sinh_x
CAL __float_exp
SET __sinh_t
F_NEG_M __sinh_x
CAL __float_exp
F_SU2 __sinh_t
F_MLT 0.5
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
F_ADD main_fz
SET __sinh_x
CAL __float_exp
SET __sinh_t
F_NEG_M __sinh_x
CAL __float_exp
F_SU2 __sinh_t
F_MLT 0.5
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 0.5
SET __sinh_x
CAL __float_exp
SET __sinh_t
F_NEG_M __sinh_x
CAL __float_exp
F_SU2 __sinh_t
F_MLT 0.5
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 0.5
SET __sinh_x
CAL __float_exp
SET __sinh_t
F_NEG_M __sinh_x
CAL __float_exp
F_SU2 __sinh_t
F_MLT 0.5
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 1
SET main_ia
LOD 0.5
SET main_fa
LOD 0.0
SET main_ca
SET main_ca_i
I2F_M main_ia
SET __cosh_x
CAL __float_exp
SET __cosh_t
F_NEG_M __cosh_x
CAL __float_exp
F_ADD __cosh_t
F_MLT 0.5
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_I2F_M main_ia
SET __cosh_x
CAL __float_exp
SET __cosh_t
F_NEG_M __cosh_x
CAL __float_exp
F_ADD __cosh_t
F_MLT 0.5
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ia
ADD main_z0
I2F
SET __cosh_x
CAL __float_exp
SET __cosh_t
F_NEG_M __cosh_x
CAL __float_exp
F_ADD __cosh_t
F_MLT 0.5
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_ia
ADD main_z0
I2F
SET __cosh_x
CAL __float_exp
SET __cosh_t
F_NEG_M __cosh_x
CAL __float_exp
F_ADD __cosh_t
F_MLT 0.5
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 1.0
SET __cosh_x
CAL __float_exp
SET __cosh_t
F_NEG_M __cosh_x
CAL __float_exp
F_ADD __cosh_t
F_MLT 0.5
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 1.0
SET __cosh_x
CAL __float_exp
SET __cosh_t
F_NEG_M __cosh_x
CAL __float_exp
F_ADD __cosh_t
F_MLT 0.5
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
SET __cosh_x
CAL __float_exp
SET __cosh_t
F_NEG_M __cosh_x
CAL __float_exp
F_ADD __cosh_t
F_MLT 0.5
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
SET __cosh_x
CAL __float_exp
SET __cosh_t
F_NEG_M __cosh_x
CAL __float_exp
F_ADD __cosh_t
F_MLT 0.5
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
F_ADD main_fz
SET __cosh_x
CAL __float_exp
SET __cosh_t
F_NEG_M __cosh_x
CAL __float_exp
F_ADD __cosh_t
F_MLT 0.5
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
F_ADD main_fz
SET __cosh_x
CAL __float_exp
SET __cosh_t
F_NEG_M __cosh_x
CAL __float_exp
F_ADD __cosh_t
F_MLT 0.5
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 0.5
SET __cosh_x
CAL __float_exp
SET __cosh_t
F_NEG_M __cosh_x
CAL __float_exp
F_ADD __cosh_t
F_MLT 0.5
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 0.5
SET __cosh_x
CAL __float_exp
SET __cosh_t
F_NEG_M __cosh_x
CAL __float_exp
F_ADD __cosh_t
F_MLT 0.5
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 1
SET main_ia
LOD 0.5
SET main_fa
LOD 0.0
SET main_ca
SET main_ca_i
I2F_M main_ia
F_MLT 2.0
CAL __float_exp
SET __tanh_e
F_ADD -1.0
SET __tanh_n
LOD __tanh_e
F_ADD 1.0
F_DIV __tanh_n
NOP
NOP
F_QUO
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_I2F_M main_ia
F_MLT 2.0
CAL __float_exp
SET __tanh_e
F_ADD -1.0
SET __tanh_n
LOD __tanh_e
F_ADD 1.0
F_DIV __tanh_n
NOP
NOP
F_QUO
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ia
ADD main_z0
I2F
F_MLT 2.0
CAL __float_exp
SET __tanh_e
F_ADD -1.0
SET __tanh_n
LOD __tanh_e
F_ADD 1.0
F_DIV __tanh_n
NOP
NOP
F_QUO
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_ia
ADD main_z0
I2F
F_MLT 2.0
CAL __float_exp
SET __tanh_e
F_ADD -1.0
SET __tanh_n
LOD __tanh_e
F_ADD 1.0
F_DIV __tanh_n
NOP
NOP
F_QUO
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 1.0
F_MLT 2.0
CAL __float_exp
SET __tanh_e
F_ADD -1.0
SET __tanh_n
LOD __tanh_e
F_ADD 1.0
F_DIV __tanh_n
NOP
NOP
F_QUO
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 1.0
F_MLT 2.0
CAL __float_exp
SET __tanh_e
F_ADD -1.0
SET __tanh_n
LOD __tanh_e
F_ADD 1.0
F_DIV __tanh_n
NOP
NOP
F_QUO
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
F_MLT 2.0
CAL __float_exp
SET __tanh_e
F_ADD -1.0
SET __tanh_n
LOD __tanh_e
F_ADD 1.0
F_DIV __tanh_n
NOP
NOP
F_QUO
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
F_MLT 2.0
CAL __float_exp
SET __tanh_e
F_ADD -1.0
SET __tanh_n
LOD __tanh_e
F_ADD 1.0
F_DIV __tanh_n
NOP
NOP
F_QUO
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
F_ADD main_fz
F_MLT 2.0
CAL __float_exp
SET __tanh_e
F_ADD -1.0
SET __tanh_n
LOD __tanh_e
F_ADD 1.0
F_DIV __tanh_n
NOP
NOP
F_QUO
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
F_ADD main_fz
F_MLT 2.0
CAL __float_exp
SET __tanh_e
F_ADD -1.0
SET __tanh_n
LOD __tanh_e
F_ADD 1.0
F_DIV __tanh_n
NOP
NOP
F_QUO
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 0.5
F_MLT 2.0
CAL __float_exp
SET __tanh_e
F_ADD -1.0
SET __tanh_n
LOD __tanh_e
F_ADD 1.0
F_DIV __tanh_n
NOP
NOP
F_QUO
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 0.5
F_MLT 2.0
CAL __float_exp
SET __tanh_e
F_ADD -1.0
SET __tanh_n
LOD __tanh_e
F_ADD 1.0
F_DIV __tanh_n
NOP
NOP
F_QUO
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 0
SET main_ia
LOD 0.0
SET main_fa
LOD 3.0
SET main_ca
LOD 4.0
SET main_ca_i
LOD 0.0
F_LES main_ca
JIZ __Lfa13a
LOD main_ca
F_DIV main_ca_i
NOP
NOP
F_QUO
CAL __float_atan
SET __fase_t
LOD 0.0
F_LES main_ca_i
JIZ __Lfa13b
LOD __fase_t
F_ADD -3.14159265359
JMP __Lfa13z
@__Lfa13b LOD __fase_t
F_ADD 3.14159265359
JMP __Lfa13z
@__Lfa13a LOD main_ca
F_LES 0.0
JIZ __Lfa13c
LOD main_ca
F_DIV main_ca_i
NOP
NOP
F_QUO
CAL __float_atan
JMP __Lfa13z
@__Lfa13c LOD main_ca_i
F_LES 0.0
JIZ __Lfa13d
LOD 1.57079632679
JMP __Lfa13z
@__Lfa13d LOD 0.0
F_LES main_ca_i
JIZ __Lfa13e
LOD -1.57079632679
JMP __Lfa13z
@__Lfa13e LOD 0.0
@__Lfa13z SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 0.0
F_LES main_ca
JIZ __Lfa14a
LOD main_ca
F_DIV main_ca_i
NOP
NOP
F_QUO
CAL __float_atan
SET __fase_t
LOD 0.0
F_LES main_ca_i
JIZ __Lfa14b
LOD __fase_t
F_ADD -3.14159265359
JMP __Lfa14z
@__Lfa14b LOD __fase_t
F_ADD 3.14159265359
JMP __Lfa14z
@__Lfa14a LOD main_ca
F_LES 0.0
JIZ __Lfa14c
LOD main_ca
F_DIV main_ca_i
NOP
NOP
F_QUO
CAL __float_atan
JMP __Lfa14z
@__Lfa14c LOD main_ca_i
F_LES 0.0
JIZ __Lfa14d
LOD 1.57079632679
JMP __Lfa14z
@__Lfa14d LOD 0.0
F_LES main_ca_i
JIZ __Lfa14e
LOD -1.57079632679
JMP __Lfa14z
@__Lfa14e LOD 0.0
@__Lfa14z SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ca
F_MLT main_one
P_LOD main_ca_i
F_MLT main_one_i
SF_SU2
P_LOD main_ca
F_MLT main_one_i
P_LOD main_ca_i
F_MLT main_one
SF_ADD
SET_P __fase_im
SET __fase_re
LOD 0.0
F_LES __fase_re
JIZ __Lfa15a
LOD __fase_re
F_DIV __fase_im
NOP
NOP
F_QUO
CAL __float_atan
SET __fase_t
LOD 0.0
F_LES __fase_im
JIZ __Lfa15b
LOD __fase_t
F_ADD -3.14159265359
JMP __Lfa15z
@__Lfa15b LOD __fase_t
F_ADD 3.14159265359
JMP __Lfa15z
@__Lfa15a LOD __fase_re
F_LES 0.0
JIZ __Lfa15c
LOD __fase_re
F_DIV __fase_im
NOP
NOP
F_QUO
CAL __float_atan
JMP __Lfa15z
@__Lfa15c LOD __fase_im
F_LES 0.0
JIZ __Lfa15d
LOD 1.57079632679
JMP __Lfa15z
@__Lfa15d LOD 0.0
F_LES __fase_im
JIZ __Lfa15e
LOD -1.57079632679
JMP __Lfa15z
@__Lfa15e LOD 0.0
@__Lfa15z SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_ca
F_MLT main_one
P_LOD main_ca_i
F_MLT main_one_i
SF_SU2
P_LOD main_ca
F_MLT main_one_i
P_LOD main_ca_i
F_MLT main_one
SF_ADD
SET_P __fase_im
SET __fase_re
LOD 0.0
F_LES __fase_re
JIZ __Lfa16a
LOD __fase_re
F_DIV __fase_im
NOP
NOP
F_QUO
CAL __float_atan
SET __fase_t
LOD 0.0
F_LES __fase_im
JIZ __Lfa16b
LOD __fase_t
F_ADD -3.14159265359
JMP __Lfa16z
@__Lfa16b LOD __fase_t
F_ADD 3.14159265359
JMP __Lfa16z
@__Lfa16a LOD __fase_re
F_LES 0.0
JIZ __Lfa16c
LOD __fase_re
F_DIV __fase_im
NOP
NOP
F_QUO
CAL __float_atan
JMP __Lfa16z
@__Lfa16c LOD __fase_im
F_LES 0.0
JIZ __Lfa16d
LOD 1.57079632679
JMP __Lfa16z
@__Lfa16d LOD 0.0
F_LES __fase_im
JIZ __Lfa16e
LOD -1.57079632679
JMP __Lfa16z
@__Lfa16e LOD 0.0
@__Lfa16z SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 0.0
F_LES 3.0
JIZ __Lfa17a
LOD 3.0
F_DIV 4.0
NOP
NOP
F_QUO
CAL __float_atan
SET __fase_t
LOD 0.0
F_LES 4.0
JIZ __Lfa17b
LOD __fase_t
F_ADD -3.14159265359
JMP __Lfa17z
@__Lfa17b LOD __fase_t
F_ADD 3.14159265359
JMP __Lfa17z
@__Lfa17a LOD 3.0
F_LES 0.0
JIZ __Lfa17c
LOD 3.0
F_DIV 4.0
NOP
NOP
F_QUO
CAL __float_atan
JMP __Lfa17z
@__Lfa17c LOD 4.0
F_LES 0.0
JIZ __Lfa17d
LOD 1.57079632679
JMP __Lfa17z
@__Lfa17d LOD 0.0
F_LES 4.0
JIZ __Lfa17e
LOD -1.57079632679
JMP __Lfa17z
@__Lfa17e LOD 0.0
@__Lfa17z SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 0.0
F_LES 3.0
JIZ __Lfa18a
LOD 3.0
F_DIV 4.0
NOP
NOP
F_QUO
CAL __float_atan
SET __fase_t
LOD 0.0
F_LES 4.0
JIZ __Lfa18b
LOD __fase_t
F_ADD -3.14159265359
JMP __Lfa18z
@__Lfa18b LOD __fase_t
F_ADD 3.14159265359
JMP __Lfa18z
@__Lfa18a LOD 3.0
F_LES 0.0
JIZ __Lfa18c
LOD 3.0
F_DIV 4.0
NOP
NOP
F_QUO
CAL __float_atan
JMP __Lfa18z
@__Lfa18c LOD 4.0
F_LES 0.0
JIZ __Lfa18d
LOD 1.57079632679
JMP __Lfa18z
@__Lfa18d LOD 0.0
F_LES 4.0
JIZ __Lfa18e
LOD -1.57079632679
JMP __Lfa18z
@__Lfa18e LOD 0.0
@__Lfa18z SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 3
SET main_ia
LOD 2.6
SET main_fa
LOD 0.0
SET main_ca
SET main_ca_i
I2F_M main_ia
SET __floor_x
F2I
I2F
SET __floor_t
F_LES __floor_x
JIZ __Lflr1end
LOD __floor_t
F_ADD -1.0
SET __floor_t
@__Lflr1end LOD __floor_t
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_I2F_M main_ia
SET __floor_x
F2I
I2F
SET __floor_t
F_LES __floor_x
JIZ __Lflr2end
LOD __floor_t
F_ADD -1.0
SET __floor_t
@__Lflr2end LOD __floor_t
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ia
ADD main_z0
I2F
SET __floor_x
F2I
I2F
SET __floor_t
F_LES __floor_x
JIZ __Lflr3end
LOD __floor_t
F_ADD -1.0
SET __floor_t
@__Lflr3end LOD __floor_t
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_ia
ADD main_z0
I2F
SET __floor_x
F2I
I2F
SET __floor_t
F_LES __floor_x
JIZ __Lflr4end
LOD __floor_t
F_ADD -1.0
SET __floor_t
@__Lflr4end LOD __floor_t
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 3.0
SET __floor_x
F2I
I2F
SET __floor_t
F_LES __floor_x
JIZ __Lflr5end
LOD __floor_t
F_ADD -1.0
SET __floor_t
@__Lflr5end LOD __floor_t
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 3.0
SET __floor_x
F2I
I2F
SET __floor_t
F_LES __floor_x
JIZ __Lflr6end
LOD __floor_t
F_ADD -1.0
SET __floor_t
@__Lflr6end LOD __floor_t
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
SET __floor_x
F2I
I2F
SET __floor_t
F_LES __floor_x
JIZ __Lflr7end
LOD __floor_t
F_ADD -1.0
SET __floor_t
@__Lflr7end LOD __floor_t
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
SET __floor_x
F2I
I2F
SET __floor_t
F_LES __floor_x
JIZ __Lflr8end
LOD __floor_t
F_ADD -1.0
SET __floor_t
@__Lflr8end LOD __floor_t
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
F_ADD main_fz
SET __floor_x
F2I
I2F
SET __floor_t
F_LES __floor_x
JIZ __Lflr9end
LOD __floor_t
F_ADD -1.0
SET __floor_t
@__Lflr9end LOD __floor_t
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
F_ADD main_fz
SET __floor_x
F2I
I2F
SET __floor_t
F_LES __floor_x
JIZ __Lflr10end
LOD __floor_t
F_ADD -1.0
SET __floor_t
@__Lflr10end LOD __floor_t
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 2.6
SET __floor_x
F2I
I2F
SET __floor_t
F_LES __floor_x
JIZ __Lflr11end
LOD __floor_t
F_ADD -1.0
SET __floor_t
@__Lflr11end LOD __floor_t
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 2.6
SET __floor_x
F2I
I2F
SET __floor_t
F_LES __floor_x
JIZ __Lflr12end
LOD __floor_t
F_ADD -1.0
SET __floor_t
@__Lflr12end LOD __floor_t
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 3
SET main_ia
LOD 2.6
SET main_fa
LOD 0.0
SET main_ca
SET main_ca_i
I2F_M main_ia
SET __ceil_x
F2I
I2F
SET __ceil_t
LOD __ceil_x
F_LES __ceil_t
JIZ __Lcel1end
LOD __ceil_t
F_ADD 1.0
SET __ceil_t
@__Lcel1end LOD __ceil_t
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_I2F_M main_ia
SET __ceil_x
F2I
I2F
SET __ceil_t
LOD __ceil_x
F_LES __ceil_t
JIZ __Lcel2end
LOD __ceil_t
F_ADD 1.0
SET __ceil_t
@__Lcel2end LOD __ceil_t
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ia
ADD main_z0
I2F
SET __ceil_x
F2I
I2F
SET __ceil_t
LOD __ceil_x
F_LES __ceil_t
JIZ __Lcel3end
LOD __ceil_t
F_ADD 1.0
SET __ceil_t
@__Lcel3end LOD __ceil_t
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_ia
ADD main_z0
I2F
SET __ceil_x
F2I
I2F
SET __ceil_t
LOD __ceil_x
F_LES __ceil_t
JIZ __Lcel4end
LOD __ceil_t
F_ADD 1.0
SET __ceil_t
@__Lcel4end LOD __ceil_t
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 3.0
SET __ceil_x
F2I
I2F
SET __ceil_t
LOD __ceil_x
F_LES __ceil_t
JIZ __Lcel5end
LOD __ceil_t
F_ADD 1.0
SET __ceil_t
@__Lcel5end LOD __ceil_t
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 3.0
SET __ceil_x
F2I
I2F
SET __ceil_t
LOD __ceil_x
F_LES __ceil_t
JIZ __Lcel6end
LOD __ceil_t
F_ADD 1.0
SET __ceil_t
@__Lcel6end LOD __ceil_t
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
SET __ceil_x
F2I
I2F
SET __ceil_t
LOD __ceil_x
F_LES __ceil_t
JIZ __Lcel7end
LOD __ceil_t
F_ADD 1.0
SET __ceil_t
@__Lcel7end LOD __ceil_t
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
SET __ceil_x
F2I
I2F
SET __ceil_t
LOD __ceil_x
F_LES __ceil_t
JIZ __Lcel8end
LOD __ceil_t
F_ADD 1.0
SET __ceil_t
@__Lcel8end LOD __ceil_t
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
F_ADD main_fz
SET __ceil_x
F2I
I2F
SET __ceil_t
LOD __ceil_x
F_LES __ceil_t
JIZ __Lcel9end
LOD __ceil_t
F_ADD 1.0
SET __ceil_t
@__Lcel9end LOD __ceil_t
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
F_ADD main_fz
SET __ceil_x
F2I
I2F
SET __ceil_t
LOD __ceil_x
F_LES __ceil_t
JIZ __Lcel10end
LOD __ceil_t
F_ADD 1.0
SET __ceil_t
@__Lcel10end LOD __ceil_t
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 2.6
SET __ceil_x
F2I
I2F
SET __ceil_t
LOD __ceil_x
F_LES __ceil_t
JIZ __Lcel11end
LOD __ceil_t
F_ADD 1.0
SET __ceil_t
@__Lcel11end LOD __ceil_t
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 2.6
SET __ceil_x
F2I
I2F
SET __ceil_t
LOD __ceil_x
F_LES __ceil_t
JIZ __Lcel12end
LOD __ceil_t
F_ADD 1.0
SET __ceil_t
@__Lcel12end LOD __ceil_t
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 3
SET main_ia
LOD 2.6
SET main_fa
LOD 0.0
SET main_ca
SET main_ca_i
I2F_M main_ia
SET __round_x
LOD 0.5
F_SGN __round_x
F_ADD __round_x
F2I
I2F
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_I2F_M main_ia
SET __round_x
LOD 0.5
F_SGN __round_x
F_ADD __round_x
F2I
I2F
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ia
ADD main_z0
I2F
SET __round_x
LOD 0.5
F_SGN __round_x
F_ADD __round_x
F2I
I2F
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_ia
ADD main_z0
I2F
SET __round_x
LOD 0.5
F_SGN __round_x
F_ADD __round_x
F2I
I2F
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 3.0
SET __round_x
LOD 0.5
F_SGN __round_x
F_ADD __round_x
F2I
I2F
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 3.0
SET __round_x
LOD 0.5
F_SGN __round_x
F_ADD __round_x
F2I
I2F
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
SET __round_x
LOD 0.5
F_SGN __round_x
F_ADD __round_x
F2I
I2F
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
SET __round_x
LOD 0.5
F_SGN __round_x
F_ADD __round_x
F2I
I2F
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
F_ADD main_fz
SET __round_x
LOD 0.5
F_SGN __round_x
F_ADD __round_x
F2I
I2F
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
F_ADD main_fz
SET __round_x
LOD 0.5
F_SGN __round_x
F_ADD __round_x
F2I
I2F
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 2.6
SET __round_x
LOD 0.5
F_SGN __round_x
F_ADD __round_x
F2I
I2F
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 2.6
SET __round_x
LOD 0.5
F_SGN __round_x
F_ADD __round_x
F2I
I2F
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 5
SET main_ia
LOD 2.5
SET main_fa
LOD 0.0
SET main_ca
SET main_ca_i
PST_M main_ia
OUT 0
PST_M main_ia
P_LOD main_pa
ADD main_z0
S_ADD
OUT 0
LOD main_ia
ADD main_z0
PST
OUT 0
LOD main_ia
ADD main_z0
PST
P_LOD main_pa
ADD main_z0
S_ADD
OUT 0
PST_M 5
OUT 0
PST_M 5
P_LOD main_pa
ADD main_z0
S_ADD
OUT 0
F_PST_M main_fa
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
PF_PST_M main_fa
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
F_ADD main_fz
F_PST
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
F_ADD main_fz
F_PST
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
F_PST_M 2.5
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
PF_PST_M 2.5
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 640
SET main_ia
LOD 0.0
SET main_fa
SET main_ca
LOD 0.0
SET main_ca_i
NRM_M main_ia
OUT 0
NRM_M main_ia
P_LOD main_pa
ADD main_z0
S_ADD
OUT 0
LOD main_ia
ADD main_z0
NRM
OUT 0
LOD main_ia
ADD main_z0
NRM
P_LOD main_pa
ADD main_z0
S_ADD
OUT 0
NRM_M 640
OUT 0
NRM_M 640
P_LOD main_pa
ADD main_z0
S_ADD
OUT 0
NEG_M 3
SET main_ia
LOD -2.6
SET main_fa
LOD 0.0
SET main_ca
SET main_ca_i
I2F_M main_ia
SET __floor_x
F2I
I2F
SET __floor_t
F_LES __floor_x
JIZ __Lflr13end
LOD __floor_t
F_ADD -1.0
SET __floor_t
@__Lflr13end LOD __floor_t
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_I2F_M main_ia
SET __floor_x
F2I
I2F
SET __floor_t
F_LES __floor_x
JIZ __Lflr14end
LOD __floor_t
F_ADD -1.0
SET __floor_t
@__Lflr14end LOD __floor_t
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ia
ADD main_z0
I2F
SET __floor_x
F2I
I2F
SET __floor_t
F_LES __floor_x
JIZ __Lflr15end
LOD __floor_t
F_ADD -1.0
SET __floor_t
@__Lflr15end LOD __floor_t
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_ia
ADD main_z0
I2F
SET __floor_x
F2I
I2F
SET __floor_t
F_LES __floor_x
JIZ __Lflr16end
LOD __floor_t
F_ADD -1.0
SET __floor_t
@__Lflr16end LOD __floor_t
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
NEG_M 3
I2F
SET __floor_x
F2I
I2F
SET __floor_t
F_LES __floor_x
JIZ __Lflr17end
LOD __floor_t
F_ADD -1.0
SET __floor_t
@__Lflr17end LOD __floor_t
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_NEG_M 3
I2F
SET __floor_x
F2I
I2F
SET __floor_t
F_LES __floor_x
JIZ __Lflr18end
LOD __floor_t
F_ADD -1.0
SET __floor_t
@__Lflr18end LOD __floor_t
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
SET __floor_x
F2I
I2F
SET __floor_t
F_LES __floor_x
JIZ __Lflr19end
LOD __floor_t
F_ADD -1.0
SET __floor_t
@__Lflr19end LOD __floor_t
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
SET __floor_x
F2I
I2F
SET __floor_t
F_LES __floor_x
JIZ __Lflr20end
LOD __floor_t
F_ADD -1.0
SET __floor_t
@__Lflr20end LOD __floor_t
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
F_ADD main_fz
SET __floor_x
F2I
I2F
SET __floor_t
F_LES __floor_x
JIZ __Lflr21end
LOD __floor_t
F_ADD -1.0
SET __floor_t
@__Lflr21end LOD __floor_t
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
F_ADD main_fz
SET __floor_x
F2I
I2F
SET __floor_t
F_LES __floor_x
JIZ __Lflr22end
LOD __floor_t
F_ADD -1.0
SET __floor_t
@__Lflr22end LOD __floor_t
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD -2.6
SET __floor_x
F2I
I2F
SET __floor_t
F_LES __floor_x
JIZ __Lflr23end
LOD __floor_t
F_ADD -1.0
SET __floor_t
@__Lflr23end LOD __floor_t
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD -2.6
SET __floor_x
F2I
I2F
SET __floor_t
F_LES __floor_x
JIZ __Lflr24end
LOD __floor_t
F_ADD -1.0
SET __floor_t
@__Lflr24end LOD __floor_t
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
NEG_M 3
SET main_ia
LOD -2.6
SET main_fa
LOD 0.0
SET main_ca
SET main_ca_i
I2F_M main_ia
SET __ceil_x
F2I
I2F
SET __ceil_t
LOD __ceil_x
F_LES __ceil_t
JIZ __Lcel13end
LOD __ceil_t
F_ADD 1.0
SET __ceil_t
@__Lcel13end LOD __ceil_t
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_I2F_M main_ia
SET __ceil_x
F2I
I2F
SET __ceil_t
LOD __ceil_x
F_LES __ceil_t
JIZ __Lcel14end
LOD __ceil_t
F_ADD 1.0
SET __ceil_t
@__Lcel14end LOD __ceil_t
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ia
ADD main_z0
I2F
SET __ceil_x
F2I
I2F
SET __ceil_t
LOD __ceil_x
F_LES __ceil_t
JIZ __Lcel15end
LOD __ceil_t
F_ADD 1.0
SET __ceil_t
@__Lcel15end LOD __ceil_t
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_ia
ADD main_z0
I2F
SET __ceil_x
F2I
I2F
SET __ceil_t
LOD __ceil_x
F_LES __ceil_t
JIZ __Lcel16end
LOD __ceil_t
F_ADD 1.0
SET __ceil_t
@__Lcel16end LOD __ceil_t
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
NEG_M 3
I2F
SET __ceil_x
F2I
I2F
SET __ceil_t
LOD __ceil_x
F_LES __ceil_t
JIZ __Lcel17end
LOD __ceil_t
F_ADD 1.0
SET __ceil_t
@__Lcel17end LOD __ceil_t
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_NEG_M 3
I2F
SET __ceil_x
F2I
I2F
SET __ceil_t
LOD __ceil_x
F_LES __ceil_t
JIZ __Lcel18end
LOD __ceil_t
F_ADD 1.0
SET __ceil_t
@__Lcel18end LOD __ceil_t
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
SET __ceil_x
F2I
I2F
SET __ceil_t
LOD __ceil_x
F_LES __ceil_t
JIZ __Lcel19end
LOD __ceil_t
F_ADD 1.0
SET __ceil_t
@__Lcel19end LOD __ceil_t
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
SET __ceil_x
F2I
I2F
SET __ceil_t
LOD __ceil_x
F_LES __ceil_t
JIZ __Lcel20end
LOD __ceil_t
F_ADD 1.0
SET __ceil_t
@__Lcel20end LOD __ceil_t
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
F_ADD main_fz
SET __ceil_x
F2I
I2F
SET __ceil_t
LOD __ceil_x
F_LES __ceil_t
JIZ __Lcel21end
LOD __ceil_t
F_ADD 1.0
SET __ceil_t
@__Lcel21end LOD __ceil_t
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
F_ADD main_fz
SET __ceil_x
F2I
I2F
SET __ceil_t
LOD __ceil_x
F_LES __ceil_t
JIZ __Lcel22end
LOD __ceil_t
F_ADD 1.0
SET __ceil_t
@__Lcel22end LOD __ceil_t
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD -2.6
SET __ceil_x
F2I
I2F
SET __ceil_t
LOD __ceil_x
F_LES __ceil_t
JIZ __Lcel23end
LOD __ceil_t
F_ADD 1.0
SET __ceil_t
@__Lcel23end LOD __ceil_t
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD -2.6
SET __ceil_x
F2I
I2F
SET __ceil_t
LOD __ceil_x
F_LES __ceil_t
JIZ __Lcel24end
LOD __ceil_t
F_ADD 1.0
SET __ceil_t
@__Lcel24end LOD __ceil_t
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
NEG_M 3
SET main_ia
LOD -2.6
SET main_fa
LOD 0.0
SET main_ca
SET main_ca_i
I2F_M main_ia
SET __round_x
LOD 0.5
F_SGN __round_x
F_ADD __round_x
F2I
I2F
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_I2F_M main_ia
SET __round_x
LOD 0.5
F_SGN __round_x
F_ADD __round_x
F2I
I2F
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ia
ADD main_z0
I2F
SET __round_x
LOD 0.5
F_SGN __round_x
F_ADD __round_x
F2I
I2F
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_ia
ADD main_z0
I2F
SET __round_x
LOD 0.5
F_SGN __round_x
F_ADD __round_x
F2I
I2F
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
NEG_M 3
I2F
SET __round_x
LOD 0.5
F_SGN __round_x
F_ADD __round_x
F2I
I2F
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_NEG_M 3
I2F
SET __round_x
LOD 0.5
F_SGN __round_x
F_ADD __round_x
F2I
I2F
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
SET __round_x
LOD 0.5
F_SGN __round_x
F_ADD __round_x
F2I
I2F
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
SET __round_x
LOD 0.5
F_SGN __round_x
F_ADD __round_x
F2I
I2F
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
F_ADD main_fz
SET __round_x
LOD 0.5
F_SGN __round_x
F_ADD __round_x
F2I
I2F
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
F_ADD main_fz
SET __round_x
LOD 0.5
F_SGN __round_x
F_ADD __round_x
F2I
I2F
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD -2.6
SET __round_x
LOD 0.5
F_SGN __round_x
F_ADD __round_x
F2I
I2F
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD -2.6
SET __round_x
LOD 0.5
F_SGN __round_x
F_ADD __round_x
F2I
I2F
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
NEG_M 5
SET main_ia
LOD -2.5
SET main_fa
LOD 0.0
SET main_ca
SET main_ca_i
PST_M main_ia
OUT 0
PST_M main_ia
P_LOD main_pa
ADD main_z0
S_ADD
OUT 0
LOD main_ia
ADD main_z0
PST
OUT 0
LOD main_ia
ADD main_z0
PST
P_LOD main_pa
ADD main_z0
S_ADD
OUT 0
NEG_M 5
PST
OUT 0
NEG_M 5
PST
P_LOD main_pa
ADD main_z0
S_ADD
OUT 0
F_PST_M main_fa
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
PF_PST_M main_fa
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
F_ADD main_fz
F_PST
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
F_ADD main_fz
F_PST
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
F_PST_M -2.5
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
PF_PST_M -2.5
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
NEG_M 640
SET main_ia
LOD 0.0
SET main_fa
SET main_ca
LOD 0.0
SET main_ca_i
NRM_M main_ia
OUT 0
NRM_M main_ia
P_LOD main_pa
ADD main_z0
S_ADD
OUT 0
LOD main_ia
ADD main_z0
NRM
OUT 0
LOD main_ia
ADD main_z0
NRM
P_LOD main_pa
ADD main_z0
S_ADD
OUT 0
NEG_M 640
NRM
OUT 0
NEG_M 640
NRM
P_LOD main_pa
ADD main_z0
S_ADD
OUT 0
NEG_M 5
SET main_ia
LOD -2.5
SET main_fa
LOD 3.0
SET main_ca
LOD 4.0
SET main_ca_i
ABS_M main_ia
OUT 0
ABS_M main_ia
P_LOD main_pa
ADD main_z0
S_ADD
OUT 0
LOD main_ia
ADD main_z0
ABS
OUT 0
LOD main_ia
ADD main_z0
ABS
P_LOD main_pa
ADD main_z0
S_ADD
OUT 0
NEG_M 5
ABS
OUT 0
NEG_M 5
ABS
P_LOD main_pa
ADD main_z0
S_ADD
OUT 0
F_ABS_M main_fa
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
PF_ABS_M main_fa
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
F_ADD main_fz
F_ABS
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
F_ADD main_fz
F_ABS
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
F_ABS_M -2.5
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
PF_ABS_M -2.5
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ca
F_MLT main_ca
P_LOD main_ca_i
F_MLT main_ca_i
SF_ADD
CAL __float_sqrt
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_ca
F_MLT main_ca
P_LOD main_ca_i
F_MLT main_ca_i
SF_ADD
CAL __float_sqrt
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ca
F_MLT main_one
P_LOD main_ca_i
F_MLT main_one_i
SF_SU2
P_LOD main_ca
F_MLT main_one_i
P_LOD main_ca_i
F_MLT main_one
SF_ADD
PSH
SF_MLT
SET_P __aux_var
PSH
SF_MLT
P_LOD __aux_var
SF_ADD
CAL __float_sqrt
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_ca
F_MLT main_one
P_LOD main_ca_i
F_MLT main_one_i
SF_SU2
P_LOD main_ca
F_MLT main_one_i
P_LOD main_ca_i
F_MLT main_one
SF_ADD
PSH
SF_MLT
SET_P __aux_var
PSH
SF_MLT
P_LOD __aux_var
SF_ADD
CAL __float_sqrt
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 3.0
F_MLT 3.0
P_LOD 4.0
F_MLT 4.0
SF_ADD
CAL __float_sqrt
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 3.0
F_MLT 3.0
P_LOD 4.0
F_MLT 4.0
SF_ADD
CAL __float_sqrt
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 0
SET main_ia
LOD 0.0
SET main_fa
LOD 3.0
SET main_ca
LOD 4.0
SET main_ca_i
LOD main_ca
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
F_ADD main_ca
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ca
F_MLT main_one
P_LOD main_ca_i
F_MLT main_one_i
SF_SU2
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_ca
F_MLT main_one
P_LOD main_ca_i
F_MLT main_one_i
SF_SU2
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 3.0
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
F_ADD 3.0
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 0
SET main_ia
LOD 0.0
SET main_fa
LOD 3.0
SET main_ca
LOD 4.0
SET main_ca_i
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
F_ADD main_ca_i
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ca
F_MLT main_one_i
P_LOD main_ca_i
F_MLT main_one
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_ca
F_MLT main_one_i
P_LOD main_ca_i
F_MLT main_one
SF_ADD
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 4.0
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
F_ADD 4.0
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 0
SET main_ia
LOD 0.0
SET main_fa
LOD 3.0
SET main_ca
LOD 4.0
SET main_ca_i
F_MLT main_ca_i
P_LOD main_ca
F_MLT main_ca
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_ca
F_MLT main_ca
P_LOD main_ca_i
F_MLT main_ca_i
SF_ADD
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ca
F_MLT main_one
P_LOD main_ca_i
F_MLT main_one_i
SF_SU2
P_LOD main_ca
F_MLT main_one_i
P_LOD main_ca_i
F_MLT main_one
SF_ADD
PSH
SF_MLT
SET_P __aux_var
PSH
SF_MLT
P_LOD __aux_var
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_ca
F_MLT main_one
P_LOD main_ca_i
F_MLT main_one_i
SF_SU2
P_LOD main_ca
F_MLT main_one_i
P_LOD main_ca_i
F_MLT main_one
SF_ADD
PSH
SF_MLT
SET_P __aux_var
PSH
SF_MLT
P_LOD __aux_var
SF_ADD
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 3.0
F_MLT 3.0
P_LOD 4.0
F_MLT 4.0
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 3.0
F_MLT 3.0
P_LOD 4.0
F_MLT 4.0
SF_ADD
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 2
SET main_ia
LOD 2.5
SET main_fa
LOD 3.0
SET main_ca
LOD 4.0
SET main_ca_i
I2F_M main_ia
P_LOD 0.0
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_I2F_M main_ia
P_LOD 0.0
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD main_ia
ADD main_z0
I2F
P_LOD 0.0
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_ia
ADD main_z0
I2F
P_LOD 0.0
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD 2.0
P_LOD 0.0
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD 2.0
P_LOD 0.0
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD main_fa
P_LOD 0.0
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_fa
P_LOD 0.0
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD main_fa
F_ADD main_fz
P_LOD 0.0
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_fa
F_ADD main_fz
P_LOD 0.0
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD 2.5
P_LOD 0.0
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD main_ca
PF_NEG_M main_ca_i
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_ca
PF_NEG_M main_ca_i
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD main_ca
F_MLT main_one
P_LOD main_ca_i
F_MLT main_one_i
SF_SU2
P_LOD main_ca
F_MLT main_one_i
P_LOD main_ca_i
F_MLT main_one
SF_ADD
F_NEG
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_ca
F_MLT main_one
P_LOD main_ca_i
F_MLT main_one_i
SF_SU2
P_LOD main_ca
F_MLT main_one_i
P_LOD main_ca_i
F_MLT main_one
SF_ADD
F_NEG
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD 3.0
PF_NEG_M 4.0
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
PF_NEG_M 4.0
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
NEG_M 3
SET main_ia
LOD -2.5
SET main_fa
LOD 0.0
SET main_ca
SET main_ca_i
LOD 5
SET main_ib
LOD 2.5
SET main_fb
LOD main_ib
SGN main_ia
OUT 0
LOD main_ib
SGN main_ia
P_LOD main_pa
ADD main_z0
S_ADD
OUT 0
I2F_M main_ia
P_I2F_M main_ib
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_I2F_M main_ia
P_I2F_M main_ib
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD main_ib
ADD main_z0
SGN main_ia
OUT 0
LOD main_ib
ADD main_z0
SGN main_ia
P_LOD main_pa
ADD main_z0
S_ADD
OUT 0
LOD main_ib
ADD main_z0
SET __aux_var
I2F_M main_ia
P_I2F_M __aux_var
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_ib
ADD main_z0
SET __aux_var
I2F_M main_ia
P_I2F_M __aux_var
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD 5
SGN main_ia
OUT 0
LOD 5
SGN main_ia
P_LOD main_pa
ADD main_z0
S_ADD
OUT 0
I2F_M main_ia
P_LOD 5.0
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_I2F_M main_ia
P_LOD 5.0
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD main_fb
F_SGN main_ia
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fb
F_SGN main_ia
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
I2F_M main_ia
P_LOD main_fb
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_I2F_M main_ia
P_LOD main_fb
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD main_fb
F_ADD main_fz
F_SGN main_ia
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fb
F_ADD main_fz
F_SGN main_ia
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fb
F_ADD main_fz
SET __aux_var
I2F_M main_ia
P_LOD __aux_var
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_fb
F_ADD main_fz
SET __aux_var
I2F_M main_ia
P_LOD __aux_var
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD 2.5
F_SGN main_ia
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 2.5
F_SGN main_ia
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
I2F_M main_ia
P_LOD 2.5
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_I2F_M main_ia
P_LOD 2.5
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD main_ia
ADD main_z0
P_LOD main_ib
S_SGN
OUT 0
LOD main_ia
ADD main_z0
P_LOD main_ib
S_SGN
P_LOD main_pa
ADD main_z0
S_ADD
OUT 0
LOD main_ia
ADD main_z0
I2F
P_I2F_M main_ib
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_ia
ADD main_z0
I2F
P_I2F_M main_ib
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD main_ia
ADD main_z0
P_LOD main_ib
ADD main_z0
S_SGN
OUT 0
LOD main_ia
ADD main_z0
P_LOD main_ib
ADD main_z0
S_SGN
P_LOD main_pa
ADD main_z0
S_ADD
OUT 0
LOD main_ia
ADD main_z0
P_LOD main_ib
ADD main_z0
SET_P __aux_var
I2F
P_I2F_M __aux_var
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_ia
ADD main_z0
P_LOD main_ib
ADD main_z0
SET_P __aux_var
I2F
P_I2F_M __aux_var
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD main_ia
ADD main_z0
P_LOD 5
S_SGN
OUT 0
LOD main_ia
ADD main_z0
P_LOD 5
S_SGN
P_LOD main_pa
ADD main_z0
S_ADD
OUT 0
LOD main_ia
ADD main_z0
I2F
P_LOD 5.0
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_ia
ADD main_z0
I2F
P_LOD 5.0
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD main_ia
ADD main_z0
P_LOD main_fb
SF_SGN
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_ia
ADD main_z0
P_LOD main_fb
SF_SGN
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ia
ADD main_z0
I2F
P_LOD main_fb
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_ia
ADD main_z0
I2F
P_LOD main_fb
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD main_ia
ADD main_z0
P_LOD main_fb
F_ADD main_fz
SF_SGN
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_ia
ADD main_z0
P_LOD main_fb
F_ADD main_fz
SF_SGN
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ia
ADD main_z0
P_LOD main_fb
F_ADD main_fz
SET_P __aux_var
I2F
P_LOD __aux_var
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_ia
ADD main_z0
P_LOD main_fb
F_ADD main_fz
SET_P __aux_var
I2F
P_LOD __aux_var
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD main_ia
ADD main_z0
P_LOD 2.5
SF_SGN
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_ia
ADD main_z0
P_LOD 2.5
SF_SGN
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ia
ADD main_z0
I2F
P_LOD 2.5
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_ia
ADD main_z0
I2F
P_LOD 2.5
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
NEG_M 3
P_LOD main_ib
S_SGN
OUT 0
NEG_M 3
P_LOD main_ib
S_SGN
P_LOD main_pa
ADD main_z0
S_ADD
OUT 0
NEG_M 3
I2F
P_I2F_M main_ib
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_NEG_M 3
I2F
P_I2F_M main_ib
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
NEG_M 3
P_LOD main_ib
ADD main_z0
S_SGN
OUT 0
NEG_M 3
P_LOD main_ib
ADD main_z0
S_SGN
P_LOD main_pa
ADD main_z0
S_ADD
OUT 0
NEG_M 3
P_LOD main_ib
ADD main_z0
SET_P __aux_var
I2F
P_I2F_M __aux_var
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_NEG_M 3
P_LOD main_ib
ADD main_z0
SET_P __aux_var
I2F
P_I2F_M __aux_var
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
NEG_M 3
P_LOD 5
S_SGN
OUT 0
NEG_M 3
P_LOD 5
S_SGN
P_LOD main_pa
ADD main_z0
S_ADD
OUT 0
NEG_M 3
I2F
P_LOD 5.0
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_NEG_M 3
I2F
P_LOD 5.0
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
NEG_M 3
P_LOD main_fb
SF_SGN
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_NEG_M 3
P_LOD main_fb
SF_SGN
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
NEG_M 3
I2F
P_LOD main_fb
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_NEG_M 3
I2F
P_LOD main_fb
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
NEG_M 3
P_LOD main_fb
F_ADD main_fz
SF_SGN
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_NEG_M 3
P_LOD main_fb
F_ADD main_fz
SF_SGN
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
NEG_M 3
P_LOD main_fb
F_ADD main_fz
SET_P __aux_var
I2F
P_LOD __aux_var
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_NEG_M 3
P_LOD main_fb
F_ADD main_fz
SET_P __aux_var
I2F
P_LOD __aux_var
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
NEG_M 3
P_LOD 2.5
SF_SGN
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_NEG_M 3
P_LOD 2.5
SF_SGN
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
NEG_M 3
I2F
P_LOD 2.5
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_NEG_M 3
I2F
P_LOD 2.5
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD main_ib
SGN main_fa
OUT 0
LOD main_ib
SGN main_fa
P_LOD main_pa
ADD main_z0
S_ADD
OUT 0
LOD main_fa
P_I2F_M main_ib
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_fa
P_I2F_M main_ib
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD main_ib
ADD main_z0
SGN main_fa
OUT 0
LOD main_ib
ADD main_z0
SGN main_fa
P_LOD main_pa
ADD main_z0
S_ADD
OUT 0
LOD main_ib
ADD main_z0
SET __aux_var
LOD main_fa
P_I2F_M __aux_var
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_ib
ADD main_z0
SET __aux_var
LOD main_fa
P_I2F_M __aux_var
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD 5
SGN main_fa
OUT 0
LOD 5
SGN main_fa
P_LOD main_pa
ADD main_z0
S_ADD
OUT 0
LOD main_fa
P_LOD 5.0
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_fa
P_LOD 5.0
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD main_fb
F_SGN main_fa
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fb
F_SGN main_fa
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
P_LOD main_fb
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_fa
P_LOD main_fb
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD main_fb
F_ADD main_fz
F_SGN main_fa
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fb
F_ADD main_fz
F_SGN main_fa
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fb
F_ADD main_fz
SET __aux_var
LOD main_fa
P_LOD __aux_var
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_fb
F_ADD main_fz
SET __aux_var
LOD main_fa
P_LOD __aux_var
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD 2.5
F_SGN main_fa
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 2.5
F_SGN main_fa
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
P_LOD 2.5
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_fa
P_LOD 2.5
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD main_fa
F_ADD main_fz
P_LOD main_ib
S_SGN
OUT 0
LOD main_fa
F_ADD main_fz
P_LOD main_ib
S_SGN
P_LOD main_pa
ADD main_z0
S_ADD
OUT 0
LOD main_fa
F_ADD main_fz
P_I2F_M main_ib
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_fa
F_ADD main_fz
P_I2F_M main_ib
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD main_fa
F_ADD main_fz
P_LOD main_ib
ADD main_z0
S_SGN
OUT 0
LOD main_fa
F_ADD main_fz
P_LOD main_ib
ADD main_z0
S_SGN
P_LOD main_pa
ADD main_z0
S_ADD
OUT 0
LOD main_fa
F_ADD main_fz
P_LOD main_ib
ADD main_z0
I2F
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_fa
F_ADD main_fz
P_LOD main_ib
ADD main_z0
I2F
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD main_fa
F_ADD main_fz
P_LOD 5
S_SGN
OUT 0
LOD main_fa
F_ADD main_fz
P_LOD 5
S_SGN
P_LOD main_pa
ADD main_z0
S_ADD
OUT 0
LOD main_fa
F_ADD main_fz
P_LOD 5.0
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_fa
F_ADD main_fz
P_LOD 5.0
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD main_fa
F_ADD main_fz
P_LOD main_fb
SF_SGN
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
F_ADD main_fz
P_LOD main_fb
SF_SGN
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
F_ADD main_fz
P_LOD main_fb
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_fa
F_ADD main_fz
P_LOD main_fb
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD main_fa
F_ADD main_fz
P_LOD main_fb
F_ADD main_fz
SF_SGN
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
F_ADD main_fz
P_LOD main_fb
F_ADD main_fz
SF_SGN
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
F_ADD main_fz
P_LOD main_fb
F_ADD main_fz
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_fa
F_ADD main_fz
P_LOD main_fb
F_ADD main_fz
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD main_fa
F_ADD main_fz
P_LOD 2.5
SF_SGN
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
F_ADD main_fz
P_LOD 2.5
SF_SGN
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
F_ADD main_fz
P_LOD 2.5
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_fa
F_ADD main_fz
P_LOD 2.5
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD main_ib
SGN -2.5
OUT 0
LOD main_ib
SGN -2.5
P_LOD main_pa
ADD main_z0
S_ADD
OUT 0
LOD -2.5
P_I2F_M main_ib
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD -2.5
P_I2F_M main_ib
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD main_ib
ADD main_z0
SGN -2.5
OUT 0
LOD main_ib
ADD main_z0
SGN -2.5
P_LOD main_pa
ADD main_z0
S_ADD
OUT 0
LOD main_ib
ADD main_z0
SET __aux_var
LOD -2.5
P_I2F_M __aux_var
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_ib
ADD main_z0
SET __aux_var
LOD -2.5
P_I2F_M __aux_var
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD 5
SGN -2.5
OUT 0
LOD 5
SGN -2.5
P_LOD main_pa
ADD main_z0
S_ADD
OUT 0
LOD -2.5
P_LOD 5.0
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD -2.5
P_LOD 5.0
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD main_fb
F_SGN -2.5
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fb
F_SGN -2.5
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD -2.5
P_LOD main_fb
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD -2.5
P_LOD main_fb
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD main_fb
F_ADD main_fz
F_SGN -2.5
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fb
F_ADD main_fz
F_SGN -2.5
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fb
F_ADD main_fz
SET __aux_var
LOD -2.5
P_LOD __aux_var
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD main_fb
F_ADD main_fz
SET __aux_var
LOD -2.5
P_LOD __aux_var
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD 2.5
F_SGN -2.5
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 2.5
F_SGN -2.5
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD -2.5
P_LOD 2.5
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
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
P_LOD -2.5
P_LOD 2.5
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P main_w_i
SET main_w
LOD 1000.0
F_MLT main_w
F2I
OUT 0
LOD 1000.0
F_MLT main_w_i
F2I
OUT 0
LOD 2
SET main_ia
LOD 1.5
SET main_fa
LOD 0.0
SET main_ca
SET main_ca_i
LOD 3
SET main_ib
LOD 2.5
SET main_fb
I2F_M main_ia
SET __pow_x
I2F_M main_ib
SET __pow_y
LOD 1.0
SET __pow_r
F_ABS_M __pow_y
SET __pow_c
@__Lpow1 LOD __pow_c
F_LES 0.5
JIZ __Lpow1end
LOD __pow_r
F_MLT __pow_x
SET __pow_r
LOD __pow_c
F_ADD -1.0
SET __pow_c
JMP __Lpow1
@__Lpow1end LOD 0.0
F_LES __pow_y
JIZ __Lpow1pos
LOD __pow_r
F_DIV 1.0
NOP
NOP
F_QUO
SET __pow_r
@__Lpow1pos LOD __pow_r
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_I2F_M main_ia
SET __pow_x
I2F_M main_ib
SET __pow_y
LOD 1.0
SET __pow_r
F_ABS_M __pow_y
SET __pow_c
@__Lpow2 LOD __pow_c
F_LES 0.5
JIZ __Lpow2end
LOD __pow_r
F_MLT __pow_x
SET __pow_r
LOD __pow_c
F_ADD -1.0
SET __pow_c
JMP __Lpow2
@__Lpow2end LOD 0.0
F_LES __pow_y
JIZ __Lpow2pos
LOD __pow_r
F_DIV 1.0
NOP
NOP
F_QUO
SET __pow_r
@__Lpow2pos LOD __pow_r
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ib
ADD main_z0
SET __pow_y
I2F_M main_ia
SET __pow_x
I2F_M __pow_y
SET __pow_y
LOD 1.0
SET __pow_r
F_ABS_M __pow_y
SET __pow_c
@__Lpow3 LOD __pow_c
F_LES 0.5
JIZ __Lpow3end
LOD __pow_r
F_MLT __pow_x
SET __pow_r
LOD __pow_c
F_ADD -1.0
SET __pow_c
JMP __Lpow3
@__Lpow3end LOD 0.0
F_LES __pow_y
JIZ __Lpow3pos
LOD __pow_r
F_DIV 1.0
NOP
NOP
F_QUO
SET __pow_r
@__Lpow3pos LOD __pow_r
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_ib
ADD main_z0
SET __pow_y
I2F_M main_ia
SET __pow_x
I2F_M __pow_y
SET __pow_y
LOD 1.0
SET __pow_r
F_ABS_M __pow_y
SET __pow_c
@__Lpow4 LOD __pow_c
F_LES 0.5
JIZ __Lpow4end
LOD __pow_r
F_MLT __pow_x
SET __pow_r
LOD __pow_c
F_ADD -1.0
SET __pow_c
JMP __Lpow4
@__Lpow4end LOD 0.0
F_LES __pow_y
JIZ __Lpow4pos
LOD __pow_r
F_DIV 1.0
NOP
NOP
F_QUO
SET __pow_r
@__Lpow4pos LOD __pow_r
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
I2F_M main_ia
SET __pow_x
LOD main_fb
SET __pow_y
LOD __pow_x
CAL __float_log
F_MLT __pow_y
CAL __float_exp
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_I2F_M main_ia
SET __pow_x
LOD main_fb
SET __pow_y
LOD __pow_x
CAL __float_log
F_MLT __pow_y
CAL __float_exp
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fb
F_ADD main_fz
SET __pow_y
I2F_M main_ia
SET __pow_x
CAL __float_log
F_MLT __pow_y
CAL __float_exp
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fb
F_ADD main_fz
SET __pow_y
I2F_M main_ia
SET __pow_x
CAL __float_log
F_MLT __pow_y
CAL __float_exp
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 1.0
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
F_ADD 1.0
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
I2F_M main_ia
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_I2F_M main_ia
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
I2F_M main_ia
SET __pow_b
F_MLT __pow_b
SET __pow_b
SET __pow_r
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_I2F_M main_ia
SET __pow_b
F_MLT __pow_b
SET __pow_b
SET __pow_r
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
I2F_M main_ia
SET __pow_b
SET __pow_r
F_MLT __pow_b
SET __pow_b
F_MLT __pow_b
SET __pow_b
LOD __pow_r
F_MLT __pow_b
SET __pow_r
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_I2F_M main_ia
SET __pow_b
SET __pow_r
F_MLT __pow_b
SET __pow_b
F_MLT __pow_b
SET __pow_b
LOD __pow_r
F_MLT __pow_b
SET __pow_r
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
NEG_M 2
SET __pow_y
I2F_M main_ia
SET __pow_x
I2F_M __pow_y
SET __pow_y
LOD 1.0
SET __pow_r
F_ABS_M __pow_y
SET __pow_c
@__Lpow5 LOD __pow_c
F_LES 0.5
JIZ __Lpow5end
LOD __pow_r
F_MLT __pow_x
SET __pow_r
LOD __pow_c
F_ADD -1.0
SET __pow_c
JMP __Lpow5
@__Lpow5end LOD 0.0
F_LES __pow_y
JIZ __Lpow5pos
LOD __pow_r
F_DIV 1.0
NOP
NOP
F_QUO
SET __pow_r
@__Lpow5pos LOD __pow_r
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_NEG_M 2
SET __pow_y
I2F_M main_ia
SET __pow_x
I2F_M __pow_y
SET __pow_y
LOD 1.0
SET __pow_r
F_ABS_M __pow_y
SET __pow_c
@__Lpow6 LOD __pow_c
F_LES 0.5
JIZ __Lpow6end
LOD __pow_r
F_MLT __pow_x
SET __pow_r
LOD __pow_c
F_ADD -1.0
SET __pow_c
JMP __Lpow6
@__Lpow6end LOD 0.0
F_LES __pow_y
JIZ __Lpow6pos
LOD __pow_r
F_DIV 1.0
NOP
NOP
F_QUO
SET __pow_r
@__Lpow6pos LOD __pow_r
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ia
ADD main_z0
I2F
SET __pow_x
I2F_M main_ib
SET __pow_y
LOD 1.0
SET __pow_r
F_ABS_M __pow_y
SET __pow_c
@__Lpow7 LOD __pow_c
F_LES 0.5
JIZ __Lpow7end
LOD __pow_r
F_MLT __pow_x
SET __pow_r
LOD __pow_c
F_ADD -1.0
SET __pow_c
JMP __Lpow7
@__Lpow7end LOD 0.0
F_LES __pow_y
JIZ __Lpow7pos
LOD __pow_r
F_DIV 1.0
NOP
NOP
F_QUO
SET __pow_r
@__Lpow7pos LOD __pow_r
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_ia
ADD main_z0
I2F
SET __pow_x
I2F_M main_ib
SET __pow_y
LOD 1.0
SET __pow_r
F_ABS_M __pow_y
SET __pow_c
@__Lpow8 LOD __pow_c
F_LES 0.5
JIZ __Lpow8end
LOD __pow_r
F_MLT __pow_x
SET __pow_r
LOD __pow_c
F_ADD -1.0
SET __pow_c
JMP __Lpow8
@__Lpow8end LOD 0.0
F_LES __pow_y
JIZ __Lpow8pos
LOD __pow_r
F_DIV 1.0
NOP
NOP
F_QUO
SET __pow_r
@__Lpow8pos LOD __pow_r
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ia
ADD main_z0
P_LOD main_ib
ADD main_z0
SET_P __pow_y
I2F
SET __pow_x
I2F_M __pow_y
SET __pow_y
LOD 1.0
SET __pow_r
F_ABS_M __pow_y
SET __pow_c
@__Lpow9 LOD __pow_c
F_LES 0.5
JIZ __Lpow9end
LOD __pow_r
F_MLT __pow_x
SET __pow_r
LOD __pow_c
F_ADD -1.0
SET __pow_c
JMP __Lpow9
@__Lpow9end LOD 0.0
F_LES __pow_y
JIZ __Lpow9pos
LOD __pow_r
F_DIV 1.0
NOP
NOP
F_QUO
SET __pow_r
@__Lpow9pos LOD __pow_r
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_ia
ADD main_z0
P_LOD main_ib
ADD main_z0
SET_P __pow_y
I2F
SET __pow_x
I2F_M __pow_y
SET __pow_y
LOD 1.0
SET __pow_r
F_ABS_M __pow_y
SET __pow_c
@__Lpow10 LOD __pow_c
F_LES 0.5
JIZ __Lpow10end
LOD __pow_r
F_MLT __pow_x
SET __pow_r
LOD __pow_c
F_ADD -1.0
SET __pow_c
JMP __Lpow10
@__Lpow10end LOD 0.0
F_LES __pow_y
JIZ __Lpow10pos
LOD __pow_r
F_DIV 1.0
NOP
NOP
F_QUO
SET __pow_r
@__Lpow10pos LOD __pow_r
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ia
ADD main_z0
I2F
SET __pow_x
LOD main_fb
SET __pow_y
LOD __pow_x
CAL __float_log
F_MLT __pow_y
CAL __float_exp
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_ia
ADD main_z0
I2F
SET __pow_x
LOD main_fb
SET __pow_y
LOD __pow_x
CAL __float_log
F_MLT __pow_y
CAL __float_exp
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ia
ADD main_z0
P_LOD main_fb
F_ADD main_fz
SET_P __pow_y
I2F
SET __pow_x
CAL __float_log
F_MLT __pow_y
CAL __float_exp
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_ia
ADD main_z0
P_LOD main_fb
F_ADD main_fz
SET_P __pow_y
I2F
SET __pow_x
CAL __float_log
F_MLT __pow_y
CAL __float_exp
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ia
ADD main_z0
LOD 1.0
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_ia
ADD main_z0
LOD 1.0
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ia
ADD main_z0
I2F
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_ia
ADD main_z0
I2F
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ia
ADD main_z0
I2F
SET __pow_b
F_MLT __pow_b
SET __pow_b
SET __pow_r
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_ia
ADD main_z0
I2F
SET __pow_b
F_MLT __pow_b
SET __pow_b
SET __pow_r
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ia
ADD main_z0
I2F
SET __pow_b
SET __pow_r
F_MLT __pow_b
SET __pow_b
F_MLT __pow_b
SET __pow_b
LOD __pow_r
F_MLT __pow_b
SET __pow_r
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_ia
ADD main_z0
I2F
SET __pow_b
SET __pow_r
F_MLT __pow_b
SET __pow_b
F_MLT __pow_b
SET __pow_b
LOD __pow_r
F_MLT __pow_b
SET __pow_r
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ia
ADD main_z0
P_NEG_M 2
SET_P __pow_y
I2F
SET __pow_x
I2F_M __pow_y
SET __pow_y
LOD 1.0
SET __pow_r
F_ABS_M __pow_y
SET __pow_c
@__Lpow11 LOD __pow_c
F_LES 0.5
JIZ __Lpow11end
LOD __pow_r
F_MLT __pow_x
SET __pow_r
LOD __pow_c
F_ADD -1.0
SET __pow_c
JMP __Lpow11
@__Lpow11end LOD 0.0
F_LES __pow_y
JIZ __Lpow11pos
LOD __pow_r
F_DIV 1.0
NOP
NOP
F_QUO
SET __pow_r
@__Lpow11pos LOD __pow_r
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_ia
ADD main_z0
P_NEG_M 2
SET_P __pow_y
I2F
SET __pow_x
I2F_M __pow_y
SET __pow_y
LOD 1.0
SET __pow_r
F_ABS_M __pow_y
SET __pow_c
@__Lpow12 LOD __pow_c
F_LES 0.5
JIZ __Lpow12end
LOD __pow_r
F_MLT __pow_x
SET __pow_r
LOD __pow_c
F_ADD -1.0
SET __pow_c
JMP __Lpow12
@__Lpow12end LOD 0.0
F_LES __pow_y
JIZ __Lpow12pos
LOD __pow_r
F_DIV 1.0
NOP
NOP
F_QUO
SET __pow_r
@__Lpow12pos LOD __pow_r
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 2.0
SET __pow_x
I2F_M main_ib
SET __pow_y
LOD 1.0
SET __pow_r
F_ABS_M __pow_y
SET __pow_c
@__Lpow13 LOD __pow_c
F_LES 0.5
JIZ __Lpow13end
LOD __pow_r
F_MLT __pow_x
SET __pow_r
LOD __pow_c
F_ADD -1.0
SET __pow_c
JMP __Lpow13
@__Lpow13end LOD 0.0
F_LES __pow_y
JIZ __Lpow13pos
LOD __pow_r
F_DIV 1.0
NOP
NOP
F_QUO
SET __pow_r
@__Lpow13pos LOD __pow_r
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 2.0
SET __pow_x
I2F_M main_ib
SET __pow_y
LOD 1.0
SET __pow_r
F_ABS_M __pow_y
SET __pow_c
@__Lpow14 LOD __pow_c
F_LES 0.5
JIZ __Lpow14end
LOD __pow_r
F_MLT __pow_x
SET __pow_r
LOD __pow_c
F_ADD -1.0
SET __pow_c
JMP __Lpow14
@__Lpow14end LOD 0.0
F_LES __pow_y
JIZ __Lpow14pos
LOD __pow_r
F_DIV 1.0
NOP
NOP
F_QUO
SET __pow_r
@__Lpow14pos LOD __pow_r
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ib
ADD main_z0
SET __pow_y
LOD 2.0
SET __pow_x
I2F_M __pow_y
SET __pow_y
LOD 1.0
SET __pow_r
F_ABS_M __pow_y
SET __pow_c
@__Lpow15 LOD __pow_c
F_LES 0.5
JIZ __Lpow15end
LOD __pow_r
F_MLT __pow_x
SET __pow_r
LOD __pow_c
F_ADD -1.0
SET __pow_c
JMP __Lpow15
@__Lpow15end LOD 0.0
F_LES __pow_y
JIZ __Lpow15pos
LOD __pow_r
F_DIV 1.0
NOP
NOP
F_QUO
SET __pow_r
@__Lpow15pos LOD __pow_r
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_ib
ADD main_z0
SET __pow_y
LOD 2.0
SET __pow_x
I2F_M __pow_y
SET __pow_y
LOD 1.0
SET __pow_r
F_ABS_M __pow_y
SET __pow_c
@__Lpow16 LOD __pow_c
F_LES 0.5
JIZ __Lpow16end
LOD __pow_r
F_MLT __pow_x
SET __pow_r
LOD __pow_c
F_ADD -1.0
SET __pow_c
JMP __Lpow16
@__Lpow16end LOD 0.0
F_LES __pow_y
JIZ __Lpow16pos
LOD __pow_r
F_DIV 1.0
NOP
NOP
F_QUO
SET __pow_r
@__Lpow16pos LOD __pow_r
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 2.0
SET __pow_x
LOD main_fb
SET __pow_y
LOD __pow_x
CAL __float_log
F_MLT __pow_y
CAL __float_exp
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 2.0
SET __pow_x
LOD main_fb
SET __pow_y
LOD __pow_x
CAL __float_log
F_MLT __pow_y
CAL __float_exp
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fb
F_ADD main_fz
SET __pow_y
LOD 2.0
SET __pow_x
CAL __float_log
F_MLT __pow_y
CAL __float_exp
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fb
F_ADD main_fz
SET __pow_y
LOD 2.0
SET __pow_x
CAL __float_log
F_MLT __pow_y
CAL __float_exp
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 1.0
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
F_ADD 1.0
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 2.0
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 2.0
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 2.0
SET __pow_b
F_MLT __pow_b
SET __pow_b
SET __pow_r
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 2.0
SET __pow_b
F_MLT __pow_b
SET __pow_b
SET __pow_r
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 2.0
SET __pow_b
SET __pow_r
F_MLT __pow_b
SET __pow_b
F_MLT __pow_b
SET __pow_b
LOD __pow_r
F_MLT __pow_b
SET __pow_r
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 2.0
SET __pow_b
SET __pow_r
F_MLT __pow_b
SET __pow_b
F_MLT __pow_b
SET __pow_b
LOD __pow_r
F_MLT __pow_b
SET __pow_r
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
NEG_M 2
SET __pow_y
LOD 2.0
SET __pow_x
I2F_M __pow_y
SET __pow_y
LOD 1.0
SET __pow_r
F_ABS_M __pow_y
SET __pow_c
@__Lpow17 LOD __pow_c
F_LES 0.5
JIZ __Lpow17end
LOD __pow_r
F_MLT __pow_x
SET __pow_r
LOD __pow_c
F_ADD -1.0
SET __pow_c
JMP __Lpow17
@__Lpow17end LOD 0.0
F_LES __pow_y
JIZ __Lpow17pos
LOD __pow_r
F_DIV 1.0
NOP
NOP
F_QUO
SET __pow_r
@__Lpow17pos LOD __pow_r
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_NEG_M 2
SET __pow_y
LOD 2.0
SET __pow_x
I2F_M __pow_y
SET __pow_y
LOD 1.0
SET __pow_r
F_ABS_M __pow_y
SET __pow_c
@__Lpow18 LOD __pow_c
F_LES 0.5
JIZ __Lpow18end
LOD __pow_r
F_MLT __pow_x
SET __pow_r
LOD __pow_c
F_ADD -1.0
SET __pow_c
JMP __Lpow18
@__Lpow18end LOD 0.0
F_LES __pow_y
JIZ __Lpow18pos
LOD __pow_r
F_DIV 1.0
NOP
NOP
F_QUO
SET __pow_r
@__Lpow18pos LOD __pow_r
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
SET __pow_x
I2F_M main_ib
SET __pow_y
LOD 1.0
SET __pow_r
F_ABS_M __pow_y
SET __pow_c
@__Lpow19 LOD __pow_c
F_LES 0.5
JIZ __Lpow19end
LOD __pow_r
F_MLT __pow_x
SET __pow_r
LOD __pow_c
F_ADD -1.0
SET __pow_c
JMP __Lpow19
@__Lpow19end LOD 0.0
F_LES __pow_y
JIZ __Lpow19pos
LOD __pow_r
F_DIV 1.0
NOP
NOP
F_QUO
SET __pow_r
@__Lpow19pos LOD __pow_r
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
SET __pow_x
I2F_M main_ib
SET __pow_y
LOD 1.0
SET __pow_r
F_ABS_M __pow_y
SET __pow_c
@__Lpow20 LOD __pow_c
F_LES 0.5
JIZ __Lpow20end
LOD __pow_r
F_MLT __pow_x
SET __pow_r
LOD __pow_c
F_ADD -1.0
SET __pow_c
JMP __Lpow20
@__Lpow20end LOD 0.0
F_LES __pow_y
JIZ __Lpow20pos
LOD __pow_r
F_DIV 1.0
NOP
NOP
F_QUO
SET __pow_r
@__Lpow20pos LOD __pow_r
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ib
ADD main_z0
SET __pow_y
LOD main_fa
SET __pow_x
I2F_M __pow_y
SET __pow_y
LOD 1.0
SET __pow_r
F_ABS_M __pow_y
SET __pow_c
@__Lpow21 LOD __pow_c
F_LES 0.5
JIZ __Lpow21end
LOD __pow_r
F_MLT __pow_x
SET __pow_r
LOD __pow_c
F_ADD -1.0
SET __pow_c
JMP __Lpow21
@__Lpow21end LOD 0.0
F_LES __pow_y
JIZ __Lpow21pos
LOD __pow_r
F_DIV 1.0
NOP
NOP
F_QUO
SET __pow_r
@__Lpow21pos LOD __pow_r
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_ib
ADD main_z0
SET __pow_y
LOD main_fa
SET __pow_x
I2F_M __pow_y
SET __pow_y
LOD 1.0
SET __pow_r
F_ABS_M __pow_y
SET __pow_c
@__Lpow22 LOD __pow_c
F_LES 0.5
JIZ __Lpow22end
LOD __pow_r
F_MLT __pow_x
SET __pow_r
LOD __pow_c
F_ADD -1.0
SET __pow_c
JMP __Lpow22
@__Lpow22end LOD 0.0
F_LES __pow_y
JIZ __Lpow22pos
LOD __pow_r
F_DIV 1.0
NOP
NOP
F_QUO
SET __pow_r
@__Lpow22pos LOD __pow_r
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
SET __pow_x
LOD main_fb
SET __pow_y
LOD __pow_x
CAL __float_log
F_MLT __pow_y
CAL __float_exp
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
SET __pow_x
LOD main_fb
SET __pow_y
LOD __pow_x
CAL __float_log
F_MLT __pow_y
CAL __float_exp
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fb
F_ADD main_fz
SET __pow_y
LOD main_fa
SET __pow_x
CAL __float_log
F_MLT __pow_y
CAL __float_exp
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fb
F_ADD main_fz
SET __pow_y
LOD main_fa
SET __pow_x
CAL __float_log
F_MLT __pow_y
CAL __float_exp
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 1.0
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
F_ADD 1.0
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
SET __pow_b
F_MLT __pow_b
SET __pow_b
SET __pow_r
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
SET __pow_b
F_MLT __pow_b
SET __pow_b
SET __pow_r
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
SET __pow_b
SET __pow_r
F_MLT __pow_b
SET __pow_b
F_MLT __pow_b
SET __pow_b
LOD __pow_r
F_MLT __pow_b
SET __pow_r
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
SET __pow_b
SET __pow_r
F_MLT __pow_b
SET __pow_b
F_MLT __pow_b
SET __pow_b
LOD __pow_r
F_MLT __pow_b
SET __pow_r
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
NEG_M 2
SET __pow_y
LOD main_fa
SET __pow_x
I2F_M __pow_y
SET __pow_y
LOD 1.0
SET __pow_r
F_ABS_M __pow_y
SET __pow_c
@__Lpow23 LOD __pow_c
F_LES 0.5
JIZ __Lpow23end
LOD __pow_r
F_MLT __pow_x
SET __pow_r
LOD __pow_c
F_ADD -1.0
SET __pow_c
JMP __Lpow23
@__Lpow23end LOD 0.0
F_LES __pow_y
JIZ __Lpow23pos
LOD __pow_r
F_DIV 1.0
NOP
NOP
F_QUO
SET __pow_r
@__Lpow23pos LOD __pow_r
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_NEG_M 2
SET __pow_y
LOD main_fa
SET __pow_x
I2F_M __pow_y
SET __pow_y
LOD 1.0
SET __pow_r
F_ABS_M __pow_y
SET __pow_c
@__Lpow24 LOD __pow_c
F_LES 0.5
JIZ __Lpow24end
LOD __pow_r
F_MLT __pow_x
SET __pow_r
LOD __pow_c
F_ADD -1.0
SET __pow_c
JMP __Lpow24
@__Lpow24end LOD 0.0
F_LES __pow_y
JIZ __Lpow24pos
LOD __pow_r
F_DIV 1.0
NOP
NOP
F_QUO
SET __pow_r
@__Lpow24pos LOD __pow_r
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
F_ADD main_fz
SET __pow_x
I2F_M main_ib
SET __pow_y
LOD 1.0
SET __pow_r
F_ABS_M __pow_y
SET __pow_c
@__Lpow25 LOD __pow_c
F_LES 0.5
JIZ __Lpow25end
LOD __pow_r
F_MLT __pow_x
SET __pow_r
LOD __pow_c
F_ADD -1.0
SET __pow_c
JMP __Lpow25
@__Lpow25end LOD 0.0
F_LES __pow_y
JIZ __Lpow25pos
LOD __pow_r
F_DIV 1.0
NOP
NOP
F_QUO
SET __pow_r
@__Lpow25pos LOD __pow_r
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
F_ADD main_fz
SET __pow_x
I2F_M main_ib
SET __pow_y
LOD 1.0
SET __pow_r
F_ABS_M __pow_y
SET __pow_c
@__Lpow26 LOD __pow_c
F_LES 0.5
JIZ __Lpow26end
LOD __pow_r
F_MLT __pow_x
SET __pow_r
LOD __pow_c
F_ADD -1.0
SET __pow_c
JMP __Lpow26
@__Lpow26end LOD 0.0
F_LES __pow_y
JIZ __Lpow26pos
LOD __pow_r
F_DIV 1.0
NOP
NOP
F_QUO
SET __pow_r
@__Lpow26pos LOD __pow_r
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
F_ADD main_fz
P_LOD main_ib
ADD main_z0
SET_P __pow_y
SET __pow_x
I2F_M __pow_y
SET __pow_y
LOD 1.0
SET __pow_r
F_ABS_M __pow_y
SET __pow_c
@__Lpow27 LOD __pow_c
F_LES 0.5
JIZ __Lpow27end
LOD __pow_r
F_MLT __pow_x
SET __pow_r
LOD __pow_c
F_ADD -1.0
SET __pow_c
JMP __Lpow27
@__Lpow27end LOD 0.0
F_LES __pow_y
JIZ __Lpow27pos
LOD __pow_r
F_DIV 1.0
NOP
NOP
F_QUO
SET __pow_r
@__Lpow27pos LOD __pow_r
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
F_ADD main_fz
P_LOD main_ib
ADD main_z0
SET_P __pow_y
SET __pow_x
I2F_M __pow_y
SET __pow_y
LOD 1.0
SET __pow_r
F_ABS_M __pow_y
SET __pow_c
@__Lpow28 LOD __pow_c
F_LES 0.5
JIZ __Lpow28end
LOD __pow_r
F_MLT __pow_x
SET __pow_r
LOD __pow_c
F_ADD -1.0
SET __pow_c
JMP __Lpow28
@__Lpow28end LOD 0.0
F_LES __pow_y
JIZ __Lpow28pos
LOD __pow_r
F_DIV 1.0
NOP
NOP
F_QUO
SET __pow_r
@__Lpow28pos LOD __pow_r
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
F_ADD main_fz
SET __pow_x
LOD main_fb
SET __pow_y
LOD __pow_x
CAL __float_log
F_MLT __pow_y
CAL __float_exp
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
F_ADD main_fz
SET __pow_x
LOD main_fb
SET __pow_y
LOD __pow_x
CAL __float_log
F_MLT __pow_y
CAL __float_exp
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
F_ADD main_fz
P_LOD main_fb
F_ADD main_fz
SET_P __pow_y
SET __pow_x
CAL __float_log
F_MLT __pow_y
CAL __float_exp
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
F_ADD main_fz
P_LOD main_fb
F_ADD main_fz
SET_P __pow_y
SET __pow_x
CAL __float_log
F_MLT __pow_y
CAL __float_exp
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
F_ADD main_fz
LOD 1.0
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
F_ADD main_fz
LOD 1.0
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
F_ADD main_fz
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
F_ADD main_fz
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
F_ADD main_fz
SET __pow_b
F_MLT __pow_b
SET __pow_b
SET __pow_r
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
F_ADD main_fz
SET __pow_b
F_MLT __pow_b
SET __pow_b
SET __pow_r
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
F_ADD main_fz
SET __pow_b
SET __pow_r
F_MLT __pow_b
SET __pow_b
F_MLT __pow_b
SET __pow_b
LOD __pow_r
F_MLT __pow_b
SET __pow_r
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
F_ADD main_fz
SET __pow_b
SET __pow_r
F_MLT __pow_b
SET __pow_b
F_MLT __pow_b
SET __pow_b
LOD __pow_r
F_MLT __pow_b
SET __pow_r
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
F_ADD main_fz
P_NEG_M 2
SET_P __pow_y
SET __pow_x
I2F_M __pow_y
SET __pow_y
LOD 1.0
SET __pow_r
F_ABS_M __pow_y
SET __pow_c
@__Lpow29 LOD __pow_c
F_LES 0.5
JIZ __Lpow29end
LOD __pow_r
F_MLT __pow_x
SET __pow_r
LOD __pow_c
F_ADD -1.0
SET __pow_c
JMP __Lpow29
@__Lpow29end LOD 0.0
F_LES __pow_y
JIZ __Lpow29pos
LOD __pow_r
F_DIV 1.0
NOP
NOP
F_QUO
SET __pow_r
@__Lpow29pos LOD __pow_r
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fa
F_ADD main_fz
P_NEG_M 2
SET_P __pow_y
SET __pow_x
I2F_M __pow_y
SET __pow_y
LOD 1.0
SET __pow_r
F_ABS_M __pow_y
SET __pow_c
@__Lpow30 LOD __pow_c
F_LES 0.5
JIZ __Lpow30end
LOD __pow_r
F_MLT __pow_x
SET __pow_r
LOD __pow_c
F_ADD -1.0
SET __pow_c
JMP __Lpow30
@__Lpow30end LOD 0.0
F_LES __pow_y
JIZ __Lpow30pos
LOD __pow_r
F_DIV 1.0
NOP
NOP
F_QUO
SET __pow_r
@__Lpow30pos LOD __pow_r
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 1.5
SET __pow_x
I2F_M main_ib
SET __pow_y
LOD 1.0
SET __pow_r
F_ABS_M __pow_y
SET __pow_c
@__Lpow31 LOD __pow_c
F_LES 0.5
JIZ __Lpow31end
LOD __pow_r
F_MLT __pow_x
SET __pow_r
LOD __pow_c
F_ADD -1.0
SET __pow_c
JMP __Lpow31
@__Lpow31end LOD 0.0
F_LES __pow_y
JIZ __Lpow31pos
LOD __pow_r
F_DIV 1.0
NOP
NOP
F_QUO
SET __pow_r
@__Lpow31pos LOD __pow_r
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 1.5
SET __pow_x
I2F_M main_ib
SET __pow_y
LOD 1.0
SET __pow_r
F_ABS_M __pow_y
SET __pow_c
@__Lpow32 LOD __pow_c
F_LES 0.5
JIZ __Lpow32end
LOD __pow_r
F_MLT __pow_x
SET __pow_r
LOD __pow_c
F_ADD -1.0
SET __pow_c
JMP __Lpow32
@__Lpow32end LOD 0.0
F_LES __pow_y
JIZ __Lpow32pos
LOD __pow_r
F_DIV 1.0
NOP
NOP
F_QUO
SET __pow_r
@__Lpow32pos LOD __pow_r
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_ib
ADD main_z0
SET __pow_y
LOD 1.5
SET __pow_x
I2F_M __pow_y
SET __pow_y
LOD 1.0
SET __pow_r
F_ABS_M __pow_y
SET __pow_c
@__Lpow33 LOD __pow_c
F_LES 0.5
JIZ __Lpow33end
LOD __pow_r
F_MLT __pow_x
SET __pow_r
LOD __pow_c
F_ADD -1.0
SET __pow_c
JMP __Lpow33
@__Lpow33end LOD 0.0
F_LES __pow_y
JIZ __Lpow33pos
LOD __pow_r
F_DIV 1.0
NOP
NOP
F_QUO
SET __pow_r
@__Lpow33pos LOD __pow_r
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_ib
ADD main_z0
SET __pow_y
LOD 1.5
SET __pow_x
I2F_M __pow_y
SET __pow_y
LOD 1.0
SET __pow_r
F_ABS_M __pow_y
SET __pow_c
@__Lpow34 LOD __pow_c
F_LES 0.5
JIZ __Lpow34end
LOD __pow_r
F_MLT __pow_x
SET __pow_r
LOD __pow_c
F_ADD -1.0
SET __pow_c
JMP __Lpow34
@__Lpow34end LOD 0.0
F_LES __pow_y
JIZ __Lpow34pos
LOD __pow_r
F_DIV 1.0
NOP
NOP
F_QUO
SET __pow_r
@__Lpow34pos LOD __pow_r
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 1.5
SET __pow_x
LOD main_fb
SET __pow_y
LOD __pow_x
CAL __float_log
F_MLT __pow_y
CAL __float_exp
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 1.5
SET __pow_x
LOD main_fb
SET __pow_y
LOD __pow_x
CAL __float_log
F_MLT __pow_y
CAL __float_exp
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fb
F_ADD main_fz
SET __pow_y
LOD 1.5
SET __pow_x
CAL __float_log
F_MLT __pow_y
CAL __float_exp
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD main_fb
F_ADD main_fz
SET __pow_y
LOD 1.5
SET __pow_x
CAL __float_log
F_MLT __pow_y
CAL __float_exp
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 1.0
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
F_ADD 1.0
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 1.5
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 1.5
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 1.5
SET __pow_b
F_MLT __pow_b
SET __pow_b
SET __pow_r
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 1.5
SET __pow_b
F_MLT __pow_b
SET __pow_b
SET __pow_r
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 1.5
SET __pow_b
SET __pow_r
F_MLT __pow_b
SET __pow_b
F_MLT __pow_b
SET __pow_b
LOD __pow_r
F_MLT __pow_b
SET __pow_r
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_LOD 1.5
SET __pow_b
SET __pow_r
F_MLT __pow_b
SET __pow_b
F_MLT __pow_b
SET __pow_b
LOD __pow_r
F_MLT __pow_b
SET __pow_r
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
NEG_M 2
SET __pow_y
LOD 1.5
SET __pow_x
I2F_M __pow_y
SET __pow_y
LOD 1.0
SET __pow_r
F_ABS_M __pow_y
SET __pow_c
@__Lpow35 LOD __pow_c
F_LES 0.5
JIZ __Lpow35end
LOD __pow_r
F_MLT __pow_x
SET __pow_r
LOD __pow_c
F_ADD -1.0
SET __pow_c
JMP __Lpow35
@__Lpow35end LOD 0.0
F_LES __pow_y
JIZ __Lpow35pos
LOD __pow_r
F_DIV 1.0
NOP
NOP
F_QUO
SET __pow_r
@__Lpow35pos LOD __pow_r
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_pf
F_ADD main_fz
P_NEG_M 2
SET __pow_y
LOD 1.5
SET __pow_x
I2F_M __pow_y
SET __pow_y
LOD 1.0
SET __pow_r
F_ABS_M __pow_y
SET __pow_c
@__Lpow36 LOD __pow_c
F_LES 0.5
JIZ __Lpow36end
LOD __pow_r
F_MLT __pow_x
SET __pow_r
LOD __pow_c
F_ADD -1.0
SET __pow_c
JMP __Lpow36
@__Lpow36end LOD 0.0
F_LES __pow_y
JIZ __Lpow36pos
LOD __pow_r
F_DIV 1.0
NOP
NOP
F_QUO
SET __pow_r
@__Lpow36pos LOD __pow_r
SF_ADD
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 42
SET main_ia
LOD 4.25
SET main_fa
LOD 0.0
SET main_ca
SET main_ca_i
LOD main_ia
OUT 0
LOD main_ia
ADD main_z0
OUT 0
LOD 42
OUT 0
LOD main_fa
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_fa
F_ADD main_fz
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 4.25
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
@fim JMP fim

// sqrt function --------------------------------------------------------------

@__float_sqrt     SET __sqrt_num       // get the parameter
              F_ROT                // first estimate (nearest power of 2)

                PSH                // update x
              F_DIV __sqrt_num       // iteration 1
NOP
NOP
F_QUO
             SF_ADD
              F_MLT 0.5

                PSH                // update x
              F_DIV __sqrt_num       // iteration 2
NOP
NOP
F_QUO
             SF_ADD
              F_MLT 0.5

                PSH                // update x
              F_DIV __sqrt_num       // iteration 3
NOP
NOP
F_QUO
             SF_ADD
              F_MLT 0.5

                PSH                // update x
              F_DIV __sqrt_num       // iteration 4
NOP
NOP
F_QUO
             SF_ADD
              F_MLT 0.5

                RET

// Arctangent function --------------------------------------------------------
// |x| is folded into [0,1] with the 1/x identity: atan(|x|) = pi/2 - atan(1/|x|)
// for |x|>1. atan(t) on [0,1] is a degree-11 odd minimax polynomial (least-
// squares fit, max abs error ~4.9e-6 -- ~6 digits, vs the old 49-point LUT's
// ~4e-5). One shared polynomial for both branches; no table, no new hardware.

@__float_atan SET   __atan_x                 // save x
            F_ABS_M __atan_x               // ax = |x|
            SET   __atan_ax
            F_LES 1.0                    // (ax > 1)?  [F_LES true when acc > X]
            SET   __atan_big               // flag: 1 if |x|>1 (use 1/x)
            JIZ   __L_atan_small
            LOD   __atan_ax                // big branch: t = 1/ax
            F_DIV 1.0                    // 1.0 / ax   (F_DIV X = X/acc)
NOP
NOP
F_QUO
            JMP   __L_atan_haveT
@__L_atan_small LOD __atan_ax               // small branch: t = ax
@__L_atan_haveT SET __atan_t

            F_MLT __atan_t                 // w = t^2
            SET   __atan_w
            LOD  -0.012300666580         // Horner in w: atan(t)/t = a1 + a3 w + ... + a11 w^5
            F_MLT __atan_w                 // a11
            F_ADD 0.054084934779         // a9
            F_MLT __atan_w
            F_ADD -0.117697073484        // a7
            F_MLT __atan_w
            F_ADD 0.194020561451         // a5
            F_MLT __atan_w
            F_ADD -0.332694520616        // a3
            F_MLT __atan_w
            F_ADD 0.999980069822         // a1
            F_MLT __atan_t                 // * t  -> atan(t)
            SET   __atan_p

            LOD   __atan_big
            JIZ   __L_atan_done            // |x|<=1 -> result = atan(t)
            LOD   __atan_p                 // |x|>1  -> result = pi/2 - atan(1/x)
            F_SU2 1.5707963268           // pi/2 - p   (F_SU2 X = X - acc)
            JMP   __L_atan_sign
@__L_atan_done LOD __atan_p
@__L_atan_sign F_SGN __atan_x                // apply the sign of x
            RET

// Sine function --------------------------------------------------------------
// O(1) range reduction to [-pi/2, pi/2]: k = round(x/pi); r = x - k*pi; then
// sin(x) = (-1)^k * sin(r). sin(r) is a degree-7 odd minimax polynomial (the
// coefficients are a least-squares fit on [0, pi/2], max abs error ~1.6e-6 --
// ~6 digits, vs the old 152-point LUT's ~3). No lookup table, no new hardware.
// NB: F2I truncates toward zero, so round-to-nearest uses a sign-half bias.

@__float_sin  SET   __sin_x                  // save x
            F_MLT 0.3183098862           // q = x / pi   (1/pi)
            SET   __sin_q
            LOD   0.5
            F_SGN __sin_q                  // copysign(0.5, q)
            F_ADD __sin_q                  // q + copysign(0.5,q)
            F2I                          // k = round(q)
            SET   __sin_k                  // keep k (int) for the (-1)^k sign
            I2F                          // float(k)
            F_MLT 3.1415926536           // k * pi
            F_SU2 __sin_x                  // r = x - k*pi  -> [-pi/2, pi/2]
            SET   __sin_r
            F_MLT __sin_r                  // w = r^2
            SET   __sin_w

            LOD  -0.000184472138         // Horner in w: sin(r)/r = a1 + a3 w + a5 w^2 + a7 w^3
            F_MLT __sin_w                  // a7
            F_ADD 0.008309516704         // a5
            F_MLT __sin_w
            F_ADD -0.166651680787        // a3
            F_MLT __sin_w
            F_ADD 0.999997487148         // a1
            F_MLT __sin_r                  // * r  -> sin(r)
            SET   __sin_p

            LOD   __sin_k
            AND   1                      // k & 1  (parity)
            JIZ   __L_sin_even             // k even -> +sin(r)
            LOD   __sin_p
            F_NEG                        // k odd  -> -sin(r)
            RET
@__L_sin_even LOD   __sin_p
            RET

// Tangent function -----------------------------------------------------------
// tan has period pi, so x is reduced to r = x - round(x/pi)*pi in [-pi/2, pi/2].
// Since +-pi/2 are poles, r is folded into [0, pi/4] with the cotangent identity
// tan(r) = 1/tan(pi/2 - |r|) for |r| > pi/4, and one shared degree-11 odd minimax
// polynomial (least-squares fit of tan(u)/u on [0, pi/4], max abs error ~6.5e-7)
// serves both branches. tan is odd, so the sign of r is applied at the end with
// F_SGN. No lookup table, no new hardware.
// NB: F2I truncates toward zero, so round-to-nearest uses a sign-half bias.

@__float_tan  SET   __tan_x                  // save x
            F_MLT 0.3183098862           // q = x / pi   (1/pi)
            SET   __tan_q
            LOD   0.5
            F_SGN __tan_q                  // copysign(0.5, q)
            F_ADD __tan_q                  // q + copysign(0.5,q)
            F2I                          // k = round(q)
            I2F                          // float(k)
            F_MLT 3.1415926536           // k * pi
            F_SU2 __tan_x                  // r = x - k*pi  -> [-pi/2, pi/2]  (F_SU2 X = X - acc)
            SET   __tan_r
            F_ABS                        // a = |r|
            SET   __tan_a
            F_LES 0.785398163            // a > pi/4 ?  [F_LES true when acc > X]
            SET   __tan_fold               // flag: 1 -> cotangent branch
            JIZ   __L_tan_inner
            LOD   __tan_a                  // outer branch: u = pi/2 - a   (u in [0,pi/4))
            F_SU2 1.5707963268           // pi/2 - a   (F_SU2 X = X - acc)
            JMP   __L_tan_haveU
@__L_tan_inner LOD  __tan_a                  // inner branch: u = a
@__L_tan_haveU SET  __tan_u

            F_MLT __tan_u                  // w = u^2
            SET   __tan_w
            LOD   0.019691612742         // Horner in w: tan(u)/u = a1 + a3 w + ... + a11 w^5
            F_MLT __tan_w                  // a11
            F_ADD 0.013512594373         // a9
            F_MLT __tan_w
            F_ADD 0.056704928952         // a7
            F_MLT __tan_w
            F_ADD 0.132942907215         // a5
            F_MLT __tan_w
            F_ADD 0.333353228230         // a3
            F_MLT __tan_w
            F_ADD 0.999999838962         // a1
            F_MLT __tan_u                  // * u  -> tan(u) = P  (>= 0)
            SET   __tan_p

            LOD   __tan_fold
            JIZ   __L_tan_done             // a <= pi/4 -> tan = P
            LOD   __tan_p                  // a > pi/4  -> tan = 1/P  (cotangent)
            F_DIV 1.0                    // 1.0 / P   (F_DIV X = X/acc)
NOP
NOP
F_QUO
            JMP   __L_tan_sign
@__L_tan_done LOD  __tan_p
@__L_tan_sign F_SGN __tan_r                  // tan is odd -> apply the sign of r
            RET

// Exponential function -------------------------------------------------------
// exp(x) = 2^n * exp(r),  n = round(x/ln2),  r = x - n*ln2 in [-ln2/2, ln2/2].
// O(1): n via F2I (round-to-nearest with a sign-half bias, since F2I truncates),
// 2^n via a single F_SCL (no loop). exp(r) is a degree-6 Taylor polynomial.

@__float_exp  SET   __exp_x                 // save x
            F_MLT 1.4426950409          // q = x / ln2   (1/ln2 = log2 e)
            SET   __exp_q
            LOD   0.5
            F_SGN __exp_q                 // copysign(0.5, q)
            F_ADD __exp_q                 // q + copysign(0.5,q)
            F2I                         // n = round(q)   (F2I truncates toward 0)
            SET   __exp_n
            I2F                         // float(n)
            F_MLT 0.6931471806          // n * ln2
            F_SU2 __exp_x                 // r = x - n*ln2   (F_SU2 X = X - acc)
            SET   __exp_r

            LOD   0.0013888889          // Horner, exp(r) = sum r^k/k! (k=0..6)
            F_MLT __exp_r                 // 1/6!
            F_ADD 0.0083333333          // 1/5!
            F_MLT __exp_r
            F_ADD 0.0416666667          // 1/4!
            F_MLT __exp_r
            F_ADD 0.1666666667          // 1/3!
            F_MLT __exp_r
            F_ADD 0.5                    // 1/2!
            F_MLT __exp_r
            F_ADD 1.0                    // 1/1!
            F_MLT __exp_r
            F_ADD 1.0                    // 1/0!  -> exp(r)
            F_SCL __exp_n                  // * 2^n  -> exp(x)
            RET

// Natural logarithm ----------------------------------------------------------
// log(x) = e*ln2 + log(m),  e = XPO(x) = floor(log2 x),  m = x*2^-e in [1,2).
// O(1): e via XPO, m via a single F_SCL (no halve/double loop). log(m) is the
// atanh series 2u(1 + u^2/3 + u^4/5 + u^6/7 + u^8/9), u = (m-1)/(m+1).
// x <= 0 returns 0.

@__float_log  SET   __log_x                 // save x (acc still = x)
            F_LES 0.0                    // (x > 0) ?   [F_LES true when acc > X]
            JIZ   __L_log_zero             // x <= 0 -> bail out with 0

            LOD   __log_x
            XPO                          // e = floor(log2 x)   (int)
            SET   __log_e
            NEG                          // -e
            SET   __log_ne
            LOD   __log_x
            F_SCL __log_ne                 // m = x * 2^(-e)  -> [1,2)
            SET   __log_m

            F_ADD 1.0                    // m+1   (acc still = m)
            SET   __log_t2
            LOD   __log_m
            F_SU1 1.0                    // m-1   (acc - X)
            SET   __log_t1
            LOD   __log_t2
            F_DIV __log_t1                 // u = (m-1)/(m+1)   [F_DIV X = X/acc]
NOP
NOP
F_QUO
            SET   __log_u
            F_MLT __log_u
            SET   __log_w                  // w = u^2

            LOD   0.1111111111           // Horner in w (1/9, 1/7, 1/5, 1/3, 1)
            F_MLT __log_w
            F_ADD 0.1428571429
            F_MLT __log_w
            F_ADD 0.2
            F_MLT __log_w
            F_ADD 0.3333333333
            F_MLT __log_w
            F_ADD 1.0
            F_MLT __log_u
            F_MLT 2.0                     // log(m)
            SET   __log_lm

            LOD   __log_e
            I2F                           // float(e)
            F_MLT 0.6931471806            // e * ln2
            F_ADD __log_lm                  // + log(m)  -> log(x)
            RET

@__L_log_zero LOD   0.0
            RET
