// ----------------------------------------------------------------------------
// relu, sigmoid, tanh, softmax and argmax on an array (nn.c) ------------------
// ----------------------------------------------------------------------------

#ifndef CMM_NN_H
#define CMM_NN_H

#include "ast.h"   // expr

// the function a STMT_NN applies (its op)
enum {
    NN_RELU = 1,   // v[k] = pset(v[k]), int or float, 1D or 2D
    NN_SIGM,       // v[k] = sigmoid(v[k]), float, 1D or 2D
    NN_TANH,       // v[k] = tanh(v[k]), float, 1D or 2D
    NN_SOFTMAX     // v[k] = exp(v[k] - max) / sum, float, 1D
};

// the name a program writes for an NN_ code
const char *nn_name(int fn);

// checks, at parse time, that v fits fn (an array of the right type and
// shape), and stops with an error when it does not
void nn_check(int fn, int v, int line);

// the same for argmax(v): a 1D int or float array
void nn_check_argmax(int v, int line);

// a STMT_NN: the call of its routine (nn_*.asm), emitted at walker time
void nn_emit(int fn, int v);

// argmax(v) inside an expression: the result in the acc, a live partial
// result pushed first, as a call of a function does
expr nn_argmax_emit(int v);

#endif // CMM_NN_H
