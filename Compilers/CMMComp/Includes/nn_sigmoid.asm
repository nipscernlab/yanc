// sigmoid on a float array, in place: v[k] = 1 / (1 + exp(-v[k])) ---------------
// docs/design/nn-functions.md. Called with __nn_p = the address of the first
// element and the acc = the address after the last; at least one element.
// Needs float_exp.asm.

@__nn_sigmoid   SET   __nn_e                // end
@__nn_sigmoid_t LOD   __nn_p
                PSH                         // the address, for STI
                LDI   0                     // x = v[k]
                F_NEG
                CAL   __float_exp           // exp(-x)
                F_ADD 1.0
                F_DIV 1.0                   // 1 / (1 + exp(-x))   (F_DIV X = X / acc)
                STI   0
                LOD   __nn_p
                ADD   1
                SET   __nn_p
                GRE   __nn_e                // more to go while end > p
                JNZ   __nn_sigmoid_t
                RET
