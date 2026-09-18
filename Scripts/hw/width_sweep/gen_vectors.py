"""Oracle for the width sweep: Python big integers model Verilog semantics
exactly, at any width. Writes one line per vector, all fields as W-bit hex:

  a b add sub mul div mod udiv umod shl shr sra lt gt ult eq

Signed ops follow Verilog's rules for a W-bit signed context: results wrap
modulo 2^W, `/` truncates toward zero (INT_MIN / -1 wraps to INT_MIN), `%`
takes the dividend's sign, `>>` is logical, `>>>` arithmetic, and a shift
amount is unsigned (>= W gives 0, or all ones for `>>>` of a negative).
udiv/umod/ult are the same operands read as unsigned.

usage: gen_vectors.py W N seed out.txt   (run by Scripts/hw/width_sweep.sh)
"""
import random, sys

W, N, seed, out = int(sys.argv[1]), int(sys.argv[2]), int(sys.argv[3]), sys.argv[4]
MASK = (1 << W) - 1
MIN, MAX = -(1 << (W - 1)), (1 << (W - 1)) - 1
DIG = W // 4

def wrap(v):        return v & MASK                      # unsigned W-bit pattern
def to_s(u):        return u - (1 << W) if u >> (W - 1) else u
def tdiv(a, b):     q = abs(a) // abs(b); return -q if (a < 0) != (b < 0) else q
def hx(u):          return format(u & MASK, '0%dx' % DIG)

rng = random.Random(seed)

def rand_s():
    r = rng.random()
    if r < 0.15:  return rng.choice([MIN, MAX, -1, 0, 1, 2, -2, MIN + 1, MAX - 1])
    if r < 0.30:  return rng.randint(-1000, 1000)                       # small
    if r < 0.45:  return to_s(1 << rng.randint(0, W - 1))               # powers of two
    if r < 0.60:  return to_s(rng.getrandbits(W) >> rng.randint(0, W - 1))  # varied magnitude
    return to_s(rng.getrandbits(W))                                     # full width

edges = [(MIN, -1), (MIN, 1), (-1, 1), (MAX, MAX), (MIN, MIN), (0, 1), (1, W - 1),
         (-1, W), (MIN, W - 1), (7, MASK), (-7, 1 << (W - 2)), (MAX, 2), (MIN, 2), (-5, 3), (5, -3)]

lines = []
i = 0
while len(lines) < N:
    a, b = edges[i] if i < len(edges) else (rand_s(), rand_s())
    i += 1
    a, b = to_s(wrap(a)), to_s(wrap(b))
    if b == 0: b = 1                                        # / by zero is x in Verilog: not measured here
    au, bu = wrap(a), wrap(b)
    n = bu                                                  # shift amount, unsigned
    q = tdiv(a, b)
    row = [
        hx(a), hx(b),
        hx(a + b), hx(a - b), hx(a * b),
        hx(q), hx(a - b * q),
        hx(au // bu), hx(au % bu),
        hx(au << n if n < W else 0),
        hx(au >> n if n < W else 0),
        hx(a >> min(n, W)),                                 # Python >> is arithmetic; >= W -> 0 / -1
        '1' if a < b else '0', '1' if a > b else '0', '1' if au < bu else '0', '1' if a == b else '0',
    ]
    lines.append(' '.join(row))

with open(out, 'w') as f:
    f.write('\n'.join(lines) + '\n')
print('W=%d: %d vectors -> %s' % (W, len(lines), out))
