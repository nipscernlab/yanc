#!/usr/bin/env python3
"""cmm_nn and test117 against relu, sigmoid, tanh, softmax and argmax computed apart from YANC.

Compilers/CMMComp/Tests/cmm_nn applies the functions of a neural network's
layers (docs/design/nn-functions.md) to arrays it declares and writes each
result to port 0: an int as it is, a float as trunc(scale * x).
Compilers/CPPComp/Tests/test117 is the same program in C++, with cppcomp's
built-ins, and five values more (exp, std::exp, tanh, std::tanh and sigmoid
of a number). This computes them in Python, in double precision, and
compares them with the fixtures' recorded outputs. An int must be equal.
A float may differ in its last digits, because the SAPHO float has a 23-bit
mantissa and exp is a polynomial evaluated in that format
(docs/library.md): a value scaled by
10^6 may differ by TOL_E6, one scaled by 10^3 by TOL_E3 (measured on
2026-11-18: the largest difference is 0; the tolerances leave room for a
change of the exp routine within its documented accuracy).

What it can see (checked on the same day): a compiler whose softmax does not
subtract the maximum first gives the same values for s, but 0, 0 and 1 for
big instead of 0.731, 0.269 and 0, because exp(200) has no SAPHO float; and,
by changing the computation here, a sum that leaves out the last element,
the last of two equal elements as argmax, an argmax computed once before the
while, sigmoid of -x, tanh of 2x and a relu that clears nothing each make
the check fail.

The recorded outputs are the regression's sim goldens (cmm_nn's golden_sim,
test117's golden.txt), so this checks that each golden is these functions',
and the regression checks that the simulation still gives the golden.

usage: python3 Scripts/nn_ref.py [repo-root]    (exit 1 on a difference)
"""
import math
import os
import sys

TOL_E6, TOL_E3 = 2, 1


def f2i(v):
    return int(math.trunc(v))


def relu(x):
    return x if x > 0 else 0 * x


def sigmoid(x):
    return 1.0 / (1.0 + math.exp(-x))


def softmax(v):
    m = max(v)
    e = [math.exp(x - m) for x in v]
    s = sum(e)
    return [x / s for x in e]


def argmax(v):
    return v.index(max(v))                      # the first of equal ones


def program():
    """[(value, scale)]: scale 0 for an int written as it is"""
    out = []
    put = lambda vals, sc: out.extend((x, sc) for x in vals)

    v = [relu(x) for x in [-2.0, -0.5, 0.0, 0.75, 3.0]]
    put(v, 1e3)
    iv = [relu(x) for x in [-3, 5, -1, 2]]
    put(iv, 0)
    m = [[relu(x) for x in row] for row in [[-1.0, 2.0, -3.0], [4.0, -5.0, 6.0]]]
    put(m[0], 1e3)
    put(m[1], 1e3)

    put([sigmoid(x) for x in [-1.0, 0.0, 1.0]], 1e6)
    put([math.tanh(x) for x in [-1.0, 0.0, 1.0]], 1e6)

    s = softmax([1.0, 2.0, 3.0, 2.0, 1.0])
    put(s, 1e6)
    put(softmax([200.0, 199.0, 0.0]), 1e6)
    one = softmax([-3.0])
    put(one, 1e3)

    tie, neg = [1.0, 5.0, 5.0, -2.0], [-4.0, -1.0, -2.0]
    put([argmax(s), argmax(iv), argmax(tie), argmax(one), argmax(neg)], 0)
    cnt, k = [0, 0, 9], 0
    while argmax(cnt) != 0:
        cnt[0] += 5
        k += 1
    put([k, argmax([0.5, -0.5, 2.5]) * 10, argmax(tie) * 10 + argmax(neg)], 0)

    x = 0.5
    put([sigmoid(x)], 1e6)
    put([relu(-x)], 1e3)
    put([relu(-7), relu(7)], 0)
    return out


def cpp_more():
    """test117's last values: exp, std::exp, tanh, std::tanh and sigmoid of a number"""
    return [(math.exp(1.0), 1e6), (math.exp(-1.0), 1e6), (math.tanh(0.5), 1e6),
            (math.tanh(0.5), 1e6), (sigmoid(2), 1e6)]


def check(name, path, ref):
    got = [int(v) for v in open(path).read().split()]
    if len(got) != len(ref):
        sys.exit('nn_ref: %s wrote %d values, the reference %d' % (name, len(got), len(ref)))
    worst = 0
    for k, (g, (x, sc)) in enumerate(zip(got, ref)):
        want = int(x) if sc == 0 else f2i(sc * x)
        tol = 0 if sc == 0 else (TOL_E6 if sc == 1e6 else TOL_E3)
        if abs(g - want) > tol:
            sys.exit('nn_ref: %s value %d is %d, the reference gives %d' % (name, k, g, want))
        worst = max(worst, abs(g - want))
    print('nn_ref: %s computes the functions (%d values, largest difference %d)' % (name, len(ref), worst))


def main():
    root = sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
    check('cmm_nn', os.path.join(root, 'Compilers', 'CMMComp', 'Tests', 'cmm_nn', 'golden_sim', 'output_0.txt'),
          program())
    check('test117', os.path.join(root, 'Compilers', 'CPPComp', 'Tests', 'test117', 'golden.txt'),
          program() + cpp_more())


if __name__ == '__main__':
    main()
