"""Extra cycles a paused pipeline would cost: each ALU operation takes
k = ceil(D / budget) cycles, D the worst register-to-register delay through
it measured on sapho_all (Quartus, ns), and the core pauses k - 1 cycles each
time it executes. Reads the per-program histograms in mix/."""
import glob, math, os, sys

S = os.path.dirname(os.path.abspath(__file__))
# ula_op code -> (name, worst path delay through the operator in sapho_all, ns).
# None: Quartus kept no instance of its own (merged into other logic): a
# one-cycle operation (pass-through, sign flip, bitwise, compare to zero).
D = {0: ('NOP', None), 1: ('LOD', None), 2: ('ADD', 15.940), 3: ('F_ADD', 44.391),
     4: ('MLT', 19.376), 5: ('F_MLT', 36.084), 6: ('DIV', 86.951), 7: ('F_DIV', 91.779),
     8: ('MOD', 85.345), 9: ('SGN', 40.137), 10: ('F_SGN', None), 11: ('NEG', None),
     12: ('NEG_M', 39.060), 13: ('F_NEG', None), 14: ('F_NEG_M', None), 15: ('ABS', 28.016),
     16: ('ABS_M', 26.897), 17: ('F_ABS', None), 18: ('F_ABS_M', None), 19: ('PST', None),
     20: ('PST_M', 10.833), 21: ('F_PST', None), 22: ('F_PST_M', None), 23: ('NRM', 20.403),
     24: ('NRM_M', 22.320), 25: ('I2F', 40.137), 26: ('I2F_M', 39.060), 27: ('F2I', 23.848),
     28: ('F2I_M', 23.853), 29: ('AND', 15.385), 30: ('ORR', 14.216), 31: ('XOR', None),
     32: ('INV', None), 33: ('INV_M', None), 34: ('LAN', 20.295), 35: ('LOR', 20.295),
     36: ('LIN', None), 37: ('LIN_M', None), 38: ('LES', 26.368), 39: ('F_LES', 37.556),
     40: ('GRE', 26.492), 41: ('F_GRE', 37.556), 42: ('EQU', 26.368), 43: ('SHL', 34.864),
     44: ('SHR', 34.864), 45: ('SRS', 34.864), 46: ('F_ROT', 19.762), 47: ('F_SU1', 44.391),
     48: ('F_SU2', 44.391), 49: ('F_SCL', 20.425), 50: ('XPO', 19.460), 51: ('XPO_M', 19.460)}

def k(code, budget):
    d = D.get(code, ('?', None))[1]
    return 1 if d is None else max(1, math.ceil(d / budget))

def main():
    budgets = [float(b) for b in sys.argv[1:]] or [18.0, 16.0]
    rows = []
    for f in sorted(glob.glob(os.path.join(S, '..', '..', '..', '.smoke', 'hw', 'opmix', 'mix', '*.txt'))):
        cnt = {}
        for line in open(f):
            a, b = line.split()
            if a != 'x': cnt[int(a)] = int(b)
        tot = sum(cnt.values())
        if tot == 0: continue
        extra = [sum(n * (k(c, b) - 1) for c, n in cnt.items()) / tot for b in budgets]
        rows.append((os.path.basename(f)[:-4], tot, extra))
    print('stages per operation (budget %s ns):' % ' / '.join('%g' % b for b in budgets))
    for c, (nm, d) in sorted(D.items()):
        if d is not None and k(c, budgets[0]) > 1:
            print('  %-6s %6.1f ns -> %s' % (nm, d, ' / '.join(str(k(c, b)) for b in budgets)))
    print()
    for b_i, b in enumerate(budgets):
        xs = sorted(r[2][b_i] for r in rows)
        n = len(xs)
        print('budget %g ns: %d programs, extra cycles median %.0f %%, mean %.0f %%, min %.0f %%, max %.0f %%' % (
            b, n, 100 * xs[n // 2], 100 * sum(xs) / n, 100 * xs[0], 100 * xs[-1]))
    print()
    for name, tot, extra in sorted(rows, key=lambda r: -r[2][0]):
        print('  %-24s %9d cycles  %s' % (name, tot, '  '.join('+%3.0f %%' % (100 * e) for e in extra)))

main()
