// relu on an int array, in place: v[k] = v[k], or 0 when negative ---------------
// docs/design/nn-functions.md. Called with __nn_p = the address of the first
// element and the acc = the address after the last; at least one element.

@__nn_relu_i    SET   __nn_e                // end
@__nn_relu_i_t  LOD   __nn_p
                PSH                         // the address, for STI
                LDI   0                     // v[k]
                PST                         // 0 when negative
                STI   0
                LOD   __nn_p
                ADD   1
                SET   __nn_p
                GRE   __nn_e                // more to go while end > p
                JNZ   __nn_relu_i_t
                RET
