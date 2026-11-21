// softmax on a float vector, in place: v[k] = exp(v[k] - max) / sum -------------
// docs/design/nn-functions.md. Called with __nn_p = the address of the first
// element and the acc = the address after the last; at least one element.
// The maximum goes first: the SAPHO float has no infinity, and exp passes the
// largest float above about 103. One division, then a product per element.
// Needs float_exp.asm.

@__nn_softmax   SET   __nn_e                // end
                LOD   __nn_p
                SET   __nn_a                // the first element's address
                LDI   0
                SET   __nn_m                // m = v[0]
@__nn_softmax_m LOD   __nn_p                // m = the largest element
                ADD   1
                SET   __nn_p
                GRE   __nn_e
                JIZ   __nn_softmax_e
                LOD   __nn_p
                LDI   0
                SET   __nn_x
                F_LES __nn_m                // m < v[k]
                JIZ   __nn_softmax_m
                LOD   __nn_x
                SET   __nn_m
                JMP   __nn_softmax_m
@__nn_softmax_e LOD   0.0                   // v[k] = exp(v[k] - m), s = their sum
                SET   __nn_s
                LOD   __nn_a
                SET   __nn_p
@__nn_softmax_x LOD   __nn_p
                PSH
                LDI   0
                F_SU1 __nn_m                // v[k] - m
                CAL   __float_exp
                STI   0                     // keeps the acc
                F_ADD __nn_s
                SET   __nn_s
                LOD   __nn_p
                ADD   1
                SET   __nn_p
                GRE   __nn_e
                JNZ   __nn_softmax_x
                LOD   __nn_s
                F_DIV 1.0                   // 1 / s   (F_DIV X = X / acc)
                SET   __nn_s
                LOD   __nn_a
                SET   __nn_p
@__nn_softmax_d LOD   __nn_p                // v[k] = v[k] / s
                PSH
                LDI   0
                F_MLT __nn_s
                STI   0
                LOD   __nn_p
                ADD   1
                SET   __nn_p
                GRE   __nn_e
                JNZ   __nn_softmax_d
                RET
