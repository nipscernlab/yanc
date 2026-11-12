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
#SHARE main_j fista_x_step
#SHARE project_h_cand_k project_h_hat_k
#SHARE lipschitz_x_s project_h_hat_k
#SHARE lipschitz_x_L project_h_hat_k
#SHARE conv_zh_n project_h_hat_k
#SHARE conv_zh_ii project_h_hat_k
#SHARE conv_zh_zi lipschitz_x_k
#SHARE conv_xh_n project_h_hat_k
#SHARE conv_xh_ii project_h_hat_k
#SHARE conv_xh_xi lipschitz_x_k
#SHARE conv_xh_j conv_zh_j
#SHARE conv_x_hcand_n project_h_hat_k
#SHARE conv_x_hcand_ii project_h_hat_k
#SHARE conv_x_hcand_xi lipschitz_x_k
#SHARE conv_x_hcand_j conv_zh_j
#SHARE residual_sub_y_n project_h_hat_k
#SHARE sum_residual_sq_s project_h_hat_k
#SHARE sum_residual_sq_n lipschitz_x_k
#SHARE correlate_residual_h_n project_h_hat_k
#SHARE correlate_residual_h_s lipschitz_x_k
#SHARE correlate_residual_h_k conv_zh_j
#SHARE correlate_residual_x_n project_h_hat_k
#SHARE correlate_residual_x_s lipschitz_x_k
#SHARE correlate_residual_x_k conv_zh_j
#SHARE fista_x_ii project_h_hat_k
#SHARE fista_x_v lipschitz_x_k
#SHARE fista_x_tn lipschitz_x_k
#SHARE fista_x_mom fista_x_t
#SHARE fista_x_xi conv_zh_j
#SHARE update_h_ii project_h_hat_k
#SHARE update_h_cdt project_h_hat_k
#SHARE fit_ii main_n
#SHARE fit_it main_n
#SHARE __sqrt_num project_h_hat_k
@main LOD 0
SET main_n
@__Lwh32top LOD main_n
PF_INN 0
P_LOD 30000.0
SF_DIV
NOP
NOP
F_QUO
STI y
INC main_n
LES 39
JIZ __Lwh32top
@__Lwh32end CAL fit
LOD 0
SET main_j
@__Lwh33top LOD main_j
LDI h_hat
F_MLT 1000000.0
F_ADD 0.5
F2I
OUT 0
INC main_j
LES 6
JIZ __Lwh33top
@__Lwh33end @fim JMP fim
@project_h_hat LOD 0
SET project_h_hat_k
@__Lwh1top LOD project_h_hat_k
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
@__Lif2else INC project_h_hat_k
LES 6
JIZ __Lwh1top
@__Lwh1end LOD 1.0
SET_V h_hat 3
RET
@project_h_cand LOD 0
SET project_h_cand_k
@__Lwh2top LOD project_h_cand_k
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
@__Lif4else INC project_h_cand_k
LES 6
JIZ __Lwh2top
@__Lwh2end LOD 1.0
SET_V s_h_cand 3
RET
@lipschitz_x LOD 0.0
SET lipschitz_x_s
LOD 0
SET lipschitz_x_k
@__Lwh3top LOD lipschitz_x_k
LDI h_hat
F_ADD lipschitz_x_s
SET lipschitz_x_s
INC lipschitz_x_k
LES 6
JIZ __Lwh3top
@__Lwh3end LOD lipschitz_x_s
F_MLT lipschitz_x_s
SET lipschitz_x_L
F_GRE 0.000000000001
JIZ __Lif5else
LOD 0.000000000001
SET lipschitz_x_L
@__Lif5else LOD lipschitz_x_L
RET
@conv_zh LOD 0
SET conv_zh_n
@__Lwh4top LOD conv_zh_n
P_LOD 0.0
STI s_conv
INC conv_zh_n
LES 39
JIZ __Lwh4top
@__Lwh4end LOD 0
SET conv_zh_ii
@__Lwh5top LOD conv_zh_ii
LDI s_z
SET conv_zh_zi
LOD 0
SET conv_zh_j
@__Lwh6top LOD conv_zh_ii
ADD conv_zh_j
PSH
LDI s_conv
P_LOD conv_zh_j
LDI h_hat
F_MLT conv_zh_zi
SF_ADD
STI s_conv
INC conv_zh_j
LES 6
JIZ __Lwh6top
@__Lwh6end INC conv_zh_ii
LES 33
JIZ __Lwh5top
@__Lwh5end RET
@conv_xh LOD 0
SET conv_xh_n
@__Lwh7top LOD conv_xh_n
P_LOD 0.0
STI s_conv
INC conv_xh_n
LES 39
JIZ __Lwh7top
@__Lwh7end LOD 0
SET conv_xh_ii
@__Lwh8top LOD conv_xh_ii
LDI x_hat
SET conv_xh_xi
LOD 0
SET conv_xh_j
@__Lwh9top LOD conv_xh_ii
ADD conv_xh_j
PSH
LDI s_conv
P_LOD conv_xh_j
LDI h_hat
F_MLT conv_xh_xi
SF_ADD
STI s_conv
INC conv_xh_j
LES 6
JIZ __Lwh9top
@__Lwh9end INC conv_xh_ii
LES 33
JIZ __Lwh8top
@__Lwh8end RET
@conv_x_hcand LOD 0
SET conv_x_hcand_n
@__Lwh10top LOD conv_x_hcand_n
P_LOD 0.0
STI s_conv
INC conv_x_hcand_n
LES 39
JIZ __Lwh10top
@__Lwh10end LOD 0
SET conv_x_hcand_ii
@__Lwh11top LOD conv_x_hcand_ii
LDI x_hat
SET conv_x_hcand_xi
LOD 0
SET conv_x_hcand_j
@__Lwh12top LOD conv_x_hcand_ii
ADD conv_x_hcand_j
PSH
LDI s_conv
P_LOD conv_x_hcand_j
LDI s_h_cand
F_MLT conv_x_hcand_xi
SF_ADD
STI s_conv
INC conv_x_hcand_j
LES 6
JIZ __Lwh12top
@__Lwh12end INC conv_x_hcand_ii
LES 33
JIZ __Lwh11top
@__Lwh11end RET
@residual_sub_y LOD 0
SET residual_sub_y_n
@__Lwh13top LOD residual_sub_y_n
P_LOD residual_sub_y_n
LDI s_conv
P_LOD residual_sub_y_n
LDI y
SF_SU2
STI s_conv
INC residual_sub_y_n
LES 39
JIZ __Lwh13top
@__Lwh13end RET
@sum_residual_sq LOD 0.0
SET sum_residual_sq_s
LOD 0
SET sum_residual_sq_n
@__Lwh14top LOD sum_residual_sq_n
LDI s_conv
PSH
SF_MLT
F_ADD sum_residual_sq_s
SET sum_residual_sq_s
INC sum_residual_sq_n
LES 39
JIZ __Lwh14top
@__Lwh14end LOD sum_residual_sq_s
RET
@correlate_residual_h LOD 0
SET correlate_residual_h_n
@__Lwh15top LOD 0.0
SET correlate_residual_h_s
LOD 0
SET correlate_residual_h_k
@__Lwh16top LOD correlate_residual_h_n
ADD correlate_residual_h_k
LDI s_conv
P_LOD correlate_residual_h_k
LDI h_hat
SF_MLT
F_ADD correlate_residual_h_s
SET correlate_residual_h_s
INC correlate_residual_h_k
LES 6
JIZ __Lwh16top
@__Lwh16end LOD correlate_residual_h_n
P_LOD correlate_residual_h_s
STI s_grad_x
INC correlate_residual_h_n
LES 33
JIZ __Lwh15top
@__Lwh15end RET
@correlate_residual_x LOD 0
SET correlate_residual_x_n
@__Lwh17top LOD 0.0
SET correlate_residual_x_s
LOD 0
SET correlate_residual_x_k
@__Lwh18top LOD correlate_residual_x_n
ADD correlate_residual_x_k
LDI s_conv
P_LOD correlate_residual_x_k
LDI x_hat
SF_MLT
F_ADD correlate_residual_x_s
SET correlate_residual_x_s
INC correlate_residual_x_k
LES 33
JIZ __Lwh18top
@__Lwh18end LOD correlate_residual_x_n
P_LOD correlate_residual_x_s
STI s_grad_h
INC correlate_residual_x_n
LES 6
JIZ __Lwh17top
@__Lwh17end RET
@fista_x LOD 0
SET fista_x_ii
@__Lwh19top LOD fista_x_ii
P_LOD fista_x_ii
LDI x_hat
STI s_z
LOD fista_x_ii
P_LOD fista_x_ii
LDI x_hat
STI s_x_prev
INC fista_x_ii
LES 33
JIZ __Lwh19top
@__Lwh19end LOD 1.0
SET fista_x_t
CAL lipschitz_x
F_DIV 1.0
NOP
NOP
F_QUO
SET fista_x_step
SET fista_x_step_lam
LOD 0
SET fista_x_it
@__Lwh20top CAL conv_zh
CAL residual_sub_y
CAL correlate_residual_h
LOD 0
SET fista_x_ii
@__Lwh21top LOD fista_x_ii
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
INC fista_x_ii
LES 33
JIZ __Lwh21top
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
NOP
F_QUO
SET fista_x_mom
LOD 0
SET fista_x_ii
@__Lwh22top LOD fista_x_ii
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
INC fista_x_ii
LES 33
JIZ __Lwh22top
@__Lwh22end LOD fista_x_tn
SET fista_x_t
INC fista_x_it
LES 19
JIZ __Lwh20top
@__Lwh20end RET
@update_h CAL project_h_hat
LOD 0
SET update_h_it
@__Lwh23top CAL conv_xh
CAL residual_sub_y
CAL sum_residual_sq
F_MLT 0.5
SET update_h_dt
CAL correlate_residual_x
LOD 0.0
SET_V s_grad_h 3
LOD update_h_dt
SET update_h_best_dt
LOD 0
SET update_h_ii
@__Lwh24top LOD update_h_ii
P_LOD update_h_ii
LDI h_hat
STI s_h_best
INC update_h_ii
LES 6
JIZ __Lwh24top
@__Lwh24end LOD 0.00001
SET update_h_trial
LOD 0
SET update_h_bt
@__Lwh25top LOD 0
SET update_h_ii
@__Lwh26top LOD update_h_ii
P_LOD update_h_ii
LDI h_hat
P_LOD update_h_ii
LDI s_grad_h
F_MLT update_h_trial
SF_SU2
STI s_h_cand
INC update_h_ii
LES 6
JIZ __Lwh26top
@__Lwh26end CAL project_h_cand
CAL conv_x_hcand
CAL residual_sub_y
CAL sum_residual_sq
F_MLT 0.5
SET update_h_cdt
F_GRE update_h_best_dt
JIZ __Lif7else
LOD update_h_cdt
SET update_h_best_dt
LOD 0
SET update_h_ii
@__Lwh27top LOD update_h_ii
P_LOD update_h_ii
LDI s_h_cand
STI s_h_best
INC update_h_ii
LES 6
JIZ __Lwh27top
@__Lwh27end @__Lif7else LOD update_h_trial
F_MLT 0.5
SET update_h_trial
INC update_h_bt
LES 14
JIZ __Lwh25top
@__Lwh25end LOD 0
SET update_h_ii
@__Lwh28top LOD update_h_ii
P_LOD update_h_ii
LDI s_h_best
STI h_hat
INC update_h_ii
LES 6
JIZ __Lwh28top
@__Lwh28end INC update_h_it
LES 4
JIZ __Lwh23top
@__Lwh23end JMP project_h_hat
@fit LOD 0
SET fit_ii
@__Lwh29top LOD fit_ii
P_LOD 0.0
STI x_hat
INC fit_ii
LES 33
JIZ __Lwh29top
@__Lwh29end LOD 0
SET fit_ii
@__Lwh30top LOD fit_ii
P_LOD 0.0
STI h_hat
INC fit_ii
LES 6
JIZ __Lwh30top
@__Lwh30end LOD 0.5
SET_V h_hat 2
LOD 1.0
SET_V h_hat 3
LOD 0.5
SET_V h_hat 4
LOD 0
SET fit_it
@__Lwh31top CAL fista_x
CAL update_h
INC fit_it
LES 39
JIZ __Lwh31top
@__Lwh31end RET

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
