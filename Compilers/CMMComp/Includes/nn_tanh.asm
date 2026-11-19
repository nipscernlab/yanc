// tanh on a float array, in place: v[k] = (exp(2x) - 1) / (exp(2x) + 1) ----------
// docs/design/nn-functions.md. Called with __nn_p = the address of the first
// element and the acc = the address after the last; at least one element.
// Needs float_exp.asm.

@__nn_tanh      SET   __nn_e                // end
@__nn_tanh_t    LOD   __nn_p
                PSH                         // the address, for STI
                LDI   0                     // x = v[k]
                F_MLT 2.0
                CAL   __float_exp           // exp(2x)
                SET   __nn_te
                F_ADD -1.0
                SET   __nn_tn               // exp(2x) - 1
                LOD   __nn_te
                F_ADD 1.0                   // exp(2x) + 1
                F_DIV __nn_tn               // (F_DIV X = X / acc)
                STI   0
                LOD   __nn_p
                ADD   1
                SET   __nn_p
                GRE   __nn_e                // more to go while end > p
                JNZ   __nn_tanh_t
                RET
