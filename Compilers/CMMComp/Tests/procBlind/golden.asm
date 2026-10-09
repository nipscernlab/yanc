NOP
#PRNAME procBlind
#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 1
#NUIOOU 1
#array y 2 40
#array x_hat 2 34
#array h_hat 2 7
#array s_z 2 34
#array s_x_prev 2 34
#array s_grad_x 2 34
#array s_conv 2 40
#array s_h_cand 2 7
#array s_h_best 2 7
#array s_grad_h 2 7
#SHARE project_h_hat_k project_h_hat_dummy
#SHARE project_h_cand_dummy project_h_hat_dummy
#SHARE project_h_cand_k project_h_hat_dummy
#SHARE lipschitz_x_dummy project_h_hat_dummy
#SHARE lipschitz_x_s project_h_hat_dummy
#SHARE lipschitz_x_L project_h_hat_dummy
#SHARE conv_zh_dummy project_h_hat_dummy
#SHARE conv_zh_n project_h_hat_dummy
#SHARE conv_zh_ii project_h_hat_dummy
#SHARE conv_zh_zi lipschitz_x_k
#SHARE conv_xh_dummy project_h_hat_dummy
#SHARE conv_xh_n project_h_hat_dummy
#SHARE conv_xh_ii project_h_hat_dummy
#SHARE conv_xh_xi lipschitz_x_k
#SHARE conv_xh_j conv_zh_j
#SHARE conv_x_hcand_dummy project_h_hat_dummy
#SHARE conv_x_hcand_n project_h_hat_dummy
#SHARE conv_x_hcand_ii project_h_hat_dummy
#SHARE conv_x_hcand_xi lipschitz_x_k
#SHARE conv_x_hcand_j conv_zh_j
#SHARE residual_sub_y_dummy project_h_hat_dummy
#SHARE residual_sub_y_n project_h_hat_dummy
#SHARE sum_residual_sq_dummy project_h_hat_dummy
#SHARE sum_residual_sq_s project_h_hat_dummy
#SHARE sum_residual_sq_n lipschitz_x_k
#SHARE correlate_residual_h_dummy project_h_hat_dummy
#SHARE correlate_residual_h_n project_h_hat_dummy
#SHARE correlate_residual_h_s lipschitz_x_k
#SHARE correlate_residual_h_k conv_zh_j
#SHARE correlate_residual_x_dummy project_h_hat_dummy
#SHARE correlate_residual_x_n project_h_hat_dummy
#SHARE correlate_residual_x_s lipschitz_x_k
#SHARE correlate_residual_x_k conv_zh_j
#SHARE fista_x_dummy fista_x_t
#SHARE fista_x_ii project_h_hat_dummy
#SHARE fista_x_L fista_x_step
#SHARE fista_x_v lipschitz_x_k
#SHARE fista_x_tn lipschitz_x_k
#SHARE fista_x_mom fista_x_t
#SHARE fista_x_xi conv_zh_j
#SHARE update_h_dummy project_h_hat_dummy
#SHARE update_h_ii project_h_hat_dummy
#SHARE update_h_cdt project_h_hat_dummy
#SHARE fit_ii fit_dummy
#SHARE fit_it fit_dummy
#SHARE main_n fit_dummy
#SHARE main_j update_h_it
#SHARE main_r update_h_dt
#SHARE __sqrt_num project_h_hat_dummy
JMP main
@project_h_hat SET project_h_hat_dummy
LOD 0
SET project_h_hat_k
@__Lwh1 LOD 7
LES project_h_hat_k
JIZ __Lwh1end
LOD project_h_hat_k
LDI h_hat
F_GRE 0.0
JIZ __Lif1else
LOD project_h_hat_k
P_LOD 0.0
STI h_hat
@__Lif1else LOD project_h_hat_k
LDI h_hat
F_LES 1.0
JIZ __Lif2else
LOD project_h_hat_k
P_LOD 1.0
STI h_hat
@__Lif2else LOD project_h_hat_k
ADD 1
SET project_h_hat_k
JMP __Lwh1
@__Lwh1end LOD 1.0
SET_V h_hat 3
RET
@project_h_cand SET project_h_cand_dummy
LOD 0
SET project_h_cand_k
@__Lwh2 LOD 7
LES project_h_cand_k
JIZ __Lwh2end
LOD project_h_cand_k
LDI s_h_cand
F_GRE 0.0
JIZ __Lif3else
LOD project_h_cand_k
P_LOD 0.0
STI s_h_cand
@__Lif3else LOD project_h_cand_k
LDI s_h_cand
F_LES 1.0
JIZ __Lif4else
LOD project_h_cand_k
P_LOD 1.0
STI s_h_cand
@__Lif4else LOD project_h_cand_k
ADD 1
SET project_h_cand_k
JMP __Lwh2
@__Lwh2end LOD 1.0
SET_V s_h_cand 3
RET
@lipschitz_x SET lipschitz_x_dummy
LOD 0.0
SET lipschitz_x_s
LOD 0
SET lipschitz_x_k
@__Lwh3 LOD 7
LES lipschitz_x_k
JIZ __Lwh3end
LOD lipschitz_x_k
LDI h_hat
F_ADD lipschitz_x_s
SET lipschitz_x_s
LOD lipschitz_x_k
ADD 1
SET lipschitz_x_k
JMP __Lwh3
@__Lwh3end LOD lipschitz_x_s
F_MLT lipschitz_x_s
SET lipschitz_x_L
F_GRE 0.000000000001
JIZ __Lif5else
LOD 0.000000000001
SET lipschitz_x_L
@__Lif5else LOD lipschitz_x_L
RET
@conv_zh SET conv_zh_dummy
LOD 0
SET conv_zh_n
@__Lwh4 LOD 40
LES conv_zh_n
JIZ __Lwh4end
LOD conv_zh_n
P_LOD 0.0
STI s_conv
LOD conv_zh_n
ADD 1
SET conv_zh_n
JMP __Lwh4
@__Lwh4end LOD 0
SET conv_zh_ii
@__Lwh5 LOD 34
LES conv_zh_ii
JIZ __Lwh5end
LOD conv_zh_ii
LDI s_z
SET conv_zh_zi
LOD 0
SET conv_zh_j
@__Lwh6 LOD 7
LES conv_zh_j
JIZ __Lwh6end
LOD conv_zh_ii
ADD conv_zh_j
P_LOD conv_zh_ii
ADD conv_zh_j
LDI s_conv
P_LOD conv_zh_j
LDI h_hat
F_MLT conv_zh_zi
SF_ADD
STI s_conv
LOD conv_zh_j
ADD 1
SET conv_zh_j
JMP __Lwh6
@__Lwh6end LOD conv_zh_ii
ADD 1
SET conv_zh_ii
JMP __Lwh5
@__Lwh5end RET
@conv_xh SET conv_xh_dummy
LOD 0
SET conv_xh_n
@__Lwh7 LOD 40
LES conv_xh_n
JIZ __Lwh7end
LOD conv_xh_n
P_LOD 0.0
STI s_conv
LOD conv_xh_n
ADD 1
SET conv_xh_n
JMP __Lwh7
@__Lwh7end LOD 0
SET conv_xh_ii
@__Lwh8 LOD 34
LES conv_xh_ii
JIZ __Lwh8end
LOD conv_xh_ii
LDI x_hat
SET conv_xh_xi
LOD 0
SET conv_xh_j
@__Lwh9 LOD 7
LES conv_xh_j
JIZ __Lwh9end
LOD conv_xh_ii
ADD conv_xh_j
P_LOD conv_xh_ii
ADD conv_xh_j
LDI s_conv
P_LOD conv_xh_j
LDI h_hat
F_MLT conv_xh_xi
SF_ADD
STI s_conv
LOD conv_xh_j
ADD 1
SET conv_xh_j
JMP __Lwh9
@__Lwh9end LOD conv_xh_ii
ADD 1
SET conv_xh_ii
JMP __Lwh8
@__Lwh8end RET
@conv_x_hcand SET conv_x_hcand_dummy
LOD 0
SET conv_x_hcand_n
@__Lwh10 LOD 40
LES conv_x_hcand_n
JIZ __Lwh10end
LOD conv_x_hcand_n
P_LOD 0.0
STI s_conv
LOD conv_x_hcand_n
ADD 1
SET conv_x_hcand_n
JMP __Lwh10
@__Lwh10end LOD 0
SET conv_x_hcand_ii
@__Lwh11 LOD 34
LES conv_x_hcand_ii
JIZ __Lwh11end
LOD conv_x_hcand_ii
LDI x_hat
SET conv_x_hcand_xi
LOD 0
SET conv_x_hcand_j
@__Lwh12 LOD 7
LES conv_x_hcand_j
JIZ __Lwh12end
LOD conv_x_hcand_ii
ADD conv_x_hcand_j
P_LOD conv_x_hcand_ii
ADD conv_x_hcand_j
LDI s_conv
P_LOD conv_x_hcand_j
LDI s_h_cand
F_MLT conv_x_hcand_xi
SF_ADD
STI s_conv
LOD conv_x_hcand_j
ADD 1
SET conv_x_hcand_j
JMP __Lwh12
@__Lwh12end LOD conv_x_hcand_ii
ADD 1
SET conv_x_hcand_ii
JMP __Lwh11
@__Lwh11end RET
@residual_sub_y SET residual_sub_y_dummy
LOD 0
SET residual_sub_y_n
@__Lwh13 LOD 40
LES residual_sub_y_n
JIZ __Lwh13end
LOD residual_sub_y_n
P_LOD residual_sub_y_n
LDI s_conv
P_LOD residual_sub_y_n
LDI y
SF_SU2
STI s_conv
LOD residual_sub_y_n
ADD 1
SET residual_sub_y_n
JMP __Lwh13
@__Lwh13end RET
@sum_residual_sq SET sum_residual_sq_dummy
LOD 0.0
SET sum_residual_sq_s
LOD 0
SET sum_residual_sq_n
@__Lwh14 LOD 40
LES sum_residual_sq_n
JIZ __Lwh14end
LOD sum_residual_sq_n
LDI s_conv
P_LOD sum_residual_sq_n
LDI s_conv
SF_MLT
F_ADD sum_residual_sq_s
SET sum_residual_sq_s
LOD sum_residual_sq_n
ADD 1
SET sum_residual_sq_n
JMP __Lwh14
@__Lwh14end LOD sum_residual_sq_s
RET
@correlate_residual_h SET correlate_residual_h_dummy
LOD 0
SET correlate_residual_h_n
@__Lwh15 LOD 34
LES correlate_residual_h_n
JIZ __Lwh15end
LOD 0.0
SET correlate_residual_h_s
LOD 0
SET correlate_residual_h_k
@__Lwh16 LOD 7
LES correlate_residual_h_k
JIZ __Lwh16end
LOD correlate_residual_h_n
ADD correlate_residual_h_k
LDI s_conv
P_LOD correlate_residual_h_k
LDI h_hat
SF_MLT
F_ADD correlate_residual_h_s
SET correlate_residual_h_s
LOD correlate_residual_h_k
ADD 1
SET correlate_residual_h_k
JMP __Lwh16
@__Lwh16end LOD correlate_residual_h_n
P_LOD correlate_residual_h_s
STI s_grad_x
LOD correlate_residual_h_n
ADD 1
SET correlate_residual_h_n
JMP __Lwh15
@__Lwh15end RET
@correlate_residual_x SET correlate_residual_x_dummy
LOD 0
SET correlate_residual_x_n
@__Lwh17 LOD 7
LES correlate_residual_x_n
JIZ __Lwh17end
LOD 0.0
SET correlate_residual_x_s
LOD 0
SET correlate_residual_x_k
@__Lwh18 LOD 34
LES correlate_residual_x_k
JIZ __Lwh18end
LOD correlate_residual_x_n
ADD correlate_residual_x_k
LDI s_conv
P_LOD correlate_residual_x_k
LDI x_hat
SF_MLT
F_ADD correlate_residual_x_s
SET correlate_residual_x_s
LOD correlate_residual_x_k
ADD 1
SET correlate_residual_x_k
JMP __Lwh18
@__Lwh18end LOD correlate_residual_x_n
P_LOD correlate_residual_x_s
STI s_grad_h
LOD correlate_residual_x_n
ADD 1
SET correlate_residual_x_n
JMP __Lwh17
@__Lwh17end RET
@fista_x SET fista_x_dummy
LOD 0
SET fista_x_ii
@__Lwh19 LOD 34
LES fista_x_ii
JIZ __Lwh19end
LOD fista_x_ii
P_LOD fista_x_ii
LDI x_hat
STI s_z
LOD fista_x_ii
P_LOD fista_x_ii
LDI x_hat
STI s_x_prev
LOD fista_x_ii
ADD 1
SET fista_x_ii
JMP __Lwh19
@__Lwh19end LOD 1.0
SET fista_x_t
LOD 0
CAL lipschitz_x
SET fista_x_L
F_DIV 1.0
NOP
F_QUO
SET fista_x_step
SET fista_x_step_lam
LOD 0
SET fista_x_it
@__Lwh20 LOD 20
LES fista_x_it
JIZ __Lwh20end
LOD 0
CAL conv_zh
LOD 0
CAL residual_sub_y
LOD 0
CAL correlate_residual_h
LOD 0
SET fista_x_ii
@__Lwh21 LOD 34
LES fista_x_ii
JIZ __Lwh21end
LOD fista_x_ii
LDI s_z
P_LOD fista_x_ii
LDI s_grad_x
F_MLT fista_x_step
SF_SU2
F_SU1 fista_x_step_lam
SET fista_x_v
F_GRE 0.0
JIZ __Lif6else
LOD 0.0
SET fista_x_v
@__Lif6else LOD fista_x_ii
P_LOD fista_x_v
STI x_hat
LOD fista_x_ii
ADD 1
SET fista_x_ii
JMP __Lwh21
@__Lwh21end LOD 4.0
F_MLT fista_x_t
F_MLT fista_x_t
F_ADD 1.0
CAL __float_sqrt
F_ADD 1.0
F_MLT 0.5
SET fista_x_tn
LOD fista_x_t
F_SU1 1.0
P_LOD fista_x_tn
SF_DIV
NOP
F_QUO
SET fista_x_mom
LOD 0
SET fista_x_ii
@__Lwh22 LOD 34
LES fista_x_ii
JIZ __Lwh22end
LOD fista_x_ii
LDI x_hat
SET fista_x_xi
LOD fista_x_ii
P_LOD fista_x_ii
LDI s_x_prev
F_SU2 fista_x_xi
F_MLT fista_x_mom
F_ADD fista_x_xi
STI s_z
LOD fista_x_ii
P_LOD fista_x_xi
STI s_x_prev
LOD fista_x_ii
ADD 1
SET fista_x_ii
JMP __Lwh22
@__Lwh22end LOD fista_x_tn
SET fista_x_t
LOD fista_x_it
ADD 1
SET fista_x_it
JMP __Lwh20
@__Lwh20end RET
@update_h SET update_h_dummy
LOD 0
CAL project_h_hat
LOD 0
SET update_h_it
@__Lwh23 LOD 5
LES update_h_it
JIZ __Lwh23end
LOD 0
CAL conv_xh
LOD 0
CAL residual_sub_y
LOD 0
CAL sum_residual_sq
F_MLT 0.5
SET update_h_dt
LOD 0
CAL correlate_residual_x
LOD 0.0
SET_V s_grad_h 3
LOD update_h_dt
SET update_h_best_dt
LOD 0
SET update_h_ii
@__Lwh24 LOD 7
LES update_h_ii
JIZ __Lwh24end
LOD update_h_ii
P_LOD update_h_ii
LDI h_hat
STI s_h_best
LOD update_h_ii
ADD 1
SET update_h_ii
JMP __Lwh24
@__Lwh24end LOD 0.00001
SET update_h_trial
LOD 0
SET update_h_bt
@__Lwh25 LOD 15
LES update_h_bt
JIZ __Lwh25end
LOD 0
SET update_h_ii
@__Lwh26 LOD 7
LES update_h_ii
JIZ __Lwh26end
LOD update_h_ii
P_LOD update_h_ii
LDI h_hat
P_LOD update_h_ii
LDI s_grad_h
F_MLT update_h_trial
SF_SU2
STI s_h_cand
LOD update_h_ii
ADD 1
SET update_h_ii
JMP __Lwh26
@__Lwh26end LOD 0
CAL project_h_cand
LOD 0
CAL conv_x_hcand
LOD 0
CAL residual_sub_y
LOD 0
CAL sum_residual_sq
F_MLT 0.5
SET update_h_cdt
F_GRE update_h_best_dt
JIZ __Lif7else
LOD update_h_cdt
SET update_h_best_dt
LOD 0
SET update_h_ii
@__Lwh27 LOD 7
LES update_h_ii
JIZ __Lwh27end
LOD update_h_ii
P_LOD update_h_ii
LDI s_h_cand
STI s_h_best
LOD update_h_ii
ADD 1
SET update_h_ii
JMP __Lwh27
@__Lwh27end @__Lif7else LOD update_h_trial
F_MLT 0.5
SET update_h_trial
LOD update_h_bt
ADD 1
SET update_h_bt
JMP __Lwh25
@__Lwh25end LOD 0
SET update_h_ii
@__Lwh28 LOD 7
LES update_h_ii
JIZ __Lwh28end
LOD update_h_ii
P_LOD update_h_ii
LDI s_h_best
STI h_hat
LOD update_h_ii
ADD 1
SET update_h_ii
JMP __Lwh28
@__Lwh28end LOD update_h_it
ADD 1
SET update_h_it
JMP __Lwh23
@__Lwh23end LOD 0
CAL project_h_hat
RET
@fit SET fit_dummy
LOD 0
SET fit_ii
@__Lwh29 LOD 34
LES fit_ii
JIZ __Lwh29end
LOD fit_ii
P_LOD 0.0
STI x_hat
LOD fit_ii
ADD 1
SET fit_ii
JMP __Lwh29
@__Lwh29end LOD 0
SET fit_ii
@__Lwh30 LOD 7
LES fit_ii
JIZ __Lwh30end
LOD fit_ii
P_LOD 0.0
STI h_hat
LOD fit_ii
ADD 1
SET fit_ii
JMP __Lwh30
@__Lwh30end LOD 0.5
SET_V h_hat 2
LOD 1.0
SET_V h_hat 3
LOD 0.5
SET_V h_hat 4
LOD 0
SET fit_it
@__Lwh31 LOD 40
LES fit_it
JIZ __Lwh31end
LOD 0
CAL fista_x
LOD 0
CAL update_h
LOD fit_it
ADD 1
SET fit_it
JMP __Lwh31
@__Lwh31end RET
@main LOD 0
SET main_n
@__Lwh32 LOD 40
LES main_n
JIZ __Lwh32end
LOD main_n
PF_INN 0
P_LOD 30000.0
SF_DIV
NOP
F_QUO
STI y
LOD main_n
ADD 1
SET main_n
JMP __Lwh32
@__Lwh32end LOD 0
CAL fit
LOD 0
SET main_j
@__Lwh33 LOD 7
LES main_j
JIZ __Lwh33end
LOD main_j
LDI h_hat
F_MLT 1000000.0
F_ADD 0.5
F2I
SET main_r
OUT 0
LOD main_j
ADD 1
SET main_j
JMP __Lwh33
@__Lwh33end @fim JMP fim

// sqrt function --------------------------------------------------------------

@__float_sqrt     SET __sqrt_num       // get the parameter
              F_ROT                // first estimate (nearest power of 2)

                PSH                // update x
              F_DIV __sqrt_num       // iteration 1
NOP
F_QUO
             SF_ADD
              F_MLT 0.5

                PSH                // update x
              F_DIV __sqrt_num       // iteration 2
NOP
F_QUO
             SF_ADD
              F_MLT 0.5

                PSH                // update x
              F_DIV __sqrt_num       // iteration 3
NOP
F_QUO
             SF_ADD
              F_MLT 0.5

                PSH                // update x
              F_DIV __sqrt_num       // iteration 4
NOP
F_QUO
             SF_ADD
              F_MLT 0.5

                RET
