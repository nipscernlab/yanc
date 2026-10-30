#!/usr/bin/env python3
"""The expected output of Compilers/CMMComp/Tests/dirac_shapes, computed apart
from YANC: the same statements in plain Python, on the same small integers.

Every value of the fixture is an integer that the SAPHO float holds exactly,
and every sum and product of them is too, so the order of the operations and
the rounding do not matter and the result is the exact one. fout() truncates
an integer-valued float to that integer.

usage: python3 Scripts/dirac_ref.py [repo-root]           print the output
       python3 Scripts/dirac_ref.py [repo-root] --check   compare it with
                                                          golden_sim/output_0.txt
       python3 Scripts/dirac_ref.py [repo-root] --write   write golden_sim
"""
import os
import sys

a = [1, 2, 3]
e = [4, 5, 6]
b = [10, 30]
B = [[1, 2], [3, 4], [5, 7]]
M = [[1, 2, 0], [0, 1, 3], [4, 0, 1]]
P = [[0, 1, 0], [0, 0, 1], [1, 0, 0]]
ia = [1, 2, 3]
IM = [[2, 0, 1], [1, 3, 0], [0, 1, 2]]
inp = [7, 8, 9, 1, 2, 3]


def mv(X, x):
    return [sum(X[r][j] * x[j] for j in range(len(x))) for r in range(len(X))]


def vm(x, X):
    return [sum(x[i] * X[i][c] for i in range(len(x))) for c in range(len(X[0]))]


def mm(X, Y):
    return [[sum(X[r][j] * Y[j][c] for j in range(len(Y))) for c in range(len(Y[0]))]
            for r in range(len(X))]


def dot(x, y):
    return sum(p * q for p, q in zip(x, y))


def outer(x, y):
    return [[p * q for q in y] for p in x]


def flat(X):
    return [v for row in X for v in row]


def output():
    o = []
    o += flat(outer(a, b))
    o += flat([[B[r][c] - a[r] * b[c] for c in range(2)] for r in range(3)])
    o += flat([[B[r][c] + 2 * a[r] * b[c] for c in range(2)] for r in range(3)])
    o += mv(P, a)
    o += [p + q for p, q in zip(a, e)]
    o += [p - q for p, q in zip(a, e)]
    o += [-p for p in a]
    o += [p - 2 * q for p, q in zip(a, e)]
    o += [p + 2 * q for p, q in zip(a, e)]
    o += [dot(a, mv(M, e))]
    o += [1 + 2 * dot(a, e)]
    o += [dot(a, mv(M, mv(M, e)))]
    o += vm(a, B)
    o += vm(a, M)
    o += mv(B, b)
    o += [p + q for p, q in zip(mv(M, a), e)]
    o += mv(M, mv(M, a))
    o += mv(M, mv(P, a))
    o += flat(mm(M, P))
    o += flat([[M[r][c] - 2 * P[r][c] for c in range(3)] for r in range(3)])
    o += flat([[3 if r == c else 0 for c in range(3)] for r in range(3)])
    o += flat([[2 if r == c else 0 for c in range(2)] for r in range(3)])
    o += flat([[a[r] * e[c] + M[r][c] for c in range(3)] for r in range(3)])
    o += mv(IM, ia)
    n = 5
    o += [n * p for p in ia]
    o += [n * p for p in a]
    o += [dot(ia, e)]
    o += [n + 1, ia[0], ia[1]]
    o += inp[0:3]
    o += [2 * p for p in inp[3:6]]
    o += [0, 0, 0]
    o += flat(mm(M, P))
    o += vm(a, M)
    o += [2 * p for p in ia]
    o += ia
    o += ia
    o += [2 * p for p in a]
    o += mv(IM, ia)
    o += mv(M, mv(M, a))
    x = 3 * 3 + dot(a, mv(M, mv(M, mv(M, e))))
    o += [x]
    o += [p + 3 * q for p, q in zip(mv(M, mv(M, a)), e)]   # tw(2.0) = 3
    o += [2 * p + (x - 1) * q for p, q in zip(a, e)]       # tw(1.0) = 2
    o += mv(M, mv(M, [2 * q for q in e]))                  # w after tw(1.0)
    return o


def main():
    args = [x for x in sys.argv[1:] if not x.startswith('--')]
    root = args[0] if args else os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
    gold = os.path.join(root, 'Compilers', 'CMMComp', 'Tests', 'dirac_shapes', 'golden_sim', 'output_0.txt')
    text = ''.join('%d\n' % v for v in output())
    if '--write' in sys.argv:
        os.makedirs(os.path.dirname(gold), exist_ok=True)
        open(gold, 'w', newline='\n').write(text)
    elif '--check' in sys.argv:
        got = open(gold).read().replace('\r', '')
        if got != text:
            sys.exit('dirac_ref: golden_sim/output_0.txt of dirac_shapes is not what the '
                     'statements compute')
        print('dirac_ref: golden_sim of dirac_shapes is the reference (%d values)' % len(output()))
    else:
        sys.stdout.write(text)


if __name__ == '__main__':
    main()
