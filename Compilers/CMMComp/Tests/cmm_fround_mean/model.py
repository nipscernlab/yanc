"""Expected output of cmm_fround_mean, computed from the rounding definition.

Usage: python3 model.py [level]    level 2 (default) = round to nearest even,
                                   level 1 = truncation
Prints the 32 lines of one turn of the program's while (1): the accumulated
error acc - exact every 32 additions, in units of 2^-8 (64 units = 1 ULP of
acc, which stays in [2^20, 2^21)). Integer arithmetic only: acc is kept as a
count of quarters (ULP = 0.25 = 64 units), x = r / 256 as r units.
"""
import sys

level = int(sys.argv[1]) if len(sys.argv) > 1 else 2

r = 1
acc = (1 << 20) * 256        # acc in units of 2^-8, always a multiple of 64
exact = acc
for k in range(32):
    for n in range(32):
        r ^= (r << 7) & 0xFFFF
        r ^= r >> 9
        r ^= (r << 8) & 0xFFFF
        s = acc + r                    # exact sum, in units of 2^-8
        q, rem = divmod(s, 64)         # rem = the 6 dropped bits
        if level >= 2 and (rem > 32 or (rem == 32 and q & 1)):
            q += 1                     # round to nearest, ties to even
        acc = q * 64
        exact += r
    print(acc - exact)
