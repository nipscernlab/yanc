// argmax of a int vector: the index of the largest element, in the acc --------
// docs/design/nn-functions.md. Called with __nn_p = the address of the first
// element and the acc = the address after the last; at least one element.
// The first of equal elements stays: only a larger one moves the index.

@__nn_argmax_i   SET   __nn_e                // end
                LOD   __nn_p
                SET   __nn_a                // the first element's address
                SET   __nn_b                // the largest one's
                LDI   0
                SET   __nn_m                // m = v[0]
@__nn_argmax_i_t LOD   __nn_p
                ADD   1
                SET   __nn_p
                GRE   __nn_e
                JIZ   __nn_argmax_i_e
                LOD   __nn_p
                LDI   0
                SET   __nn_x
                LES   __nn_m                // m < v[k]
                JIZ   __nn_argmax_i_t
                LOD   __nn_x
                SET   __nn_m
                LOD   __nn_p
                SET   __nn_b
                JMP   __nn_argmax_i_t
@__nn_argmax_i_e LOD   __nn_a                // b - a, the index
                NEG
                ADD   __nn_b
                RET
