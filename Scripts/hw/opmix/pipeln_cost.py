"""The measured factor table (period of the variant over the base's, on the
current Altera and Xilinx families) and what it gives a #PIPELN 1 processor:
the latency k of each ALU operation and the extra cycles over the regress
programs (histograms from run_mix.sh).

    python3 pipeln_cost.py [threshold]

Operations whose factor is at most `threshold` (both families) take 1 cycle
and set the clock: period = base x the largest of their factors. Every other
operation takes k = ceil(factor / that).
"""
import glob, math, os, sys

BASE = {'cv': 53.82, 'zynq': 45.39}                     # MHz
FMAX = {                                                # group: (Cyclone V, Zynq-7010) MHz
    'fadd': (33.84, 32.35), 'fmlt': (42.78, 36.64), 'i2f': (45.89, 41.97),
    'f2i': (52.41, 45.78), 'shift': (52.08, 46.20), 'cmp': (51.61, 46.23),
    'fcmp': (49.46, 46.34), 'nrm': (51.26, 43.65), 'sgn': (52.57, 44.98),
    'frot': (48.67, 42.35), 'mlt': (49.47, 44.95), 'div': (12.03, 9.50),
    'mod': (11.86, 9.33), 'fdiv': (10.61, 9.71)}
# ALU operation code (id_ula_op) -> group; codes absent here are light (1 cycle)
CODE = {3: 'fadd', 47: 'fadd', 48: 'fadd', 5: 'fmlt', 25: 'i2f', 26: 'i2f',
        27: 'f2i', 28: 'f2i', 43: 'shift', 44: 'shift', 45: 'shift',
        38: 'cmp', 40: 'cmp', 42: 'cmp', 39: 'fcmp', 41: 'fcmp', 23: 'nrm', 24: 'nrm',
        9: 'sgn', 10: 'sgn', 46: 'frot', 49: 'frot', 50: 'frot', 51: 'frot',
        4: 'mlt', 6: 'div', 8: 'mod', 7: 'fdiv'}

def factor(g):
    return max(BASE['cv'] / FMAX[g][0], BASE['zynq'] / FMAX[g][1])

def plan(threshold):
    one = [g for g in FMAX if factor(g) <= threshold]
    clock = max([1.0] + [factor(g) for g in one])
    k = {g: (1 if g in one else math.ceil(factor(g) / clock)) for g in FMAX}
    return clock, k

def main():
    threshold = float(sys.argv[1]) if len(sys.argv) > 1 else 1.10
    clock, k = plan(threshold)
    print('threshold %.2f: clock = base / %.2f  (Cyclone V %.1f MHz, Zynq-7010 %.1f MHz)'
          % (threshold, clock, BASE['cv'] / clock, BASE['zynq'] / clock))
    for g in sorted(FMAX, key=factor):
        print('  %-6s factor %.2f -> k %d' % (g, factor(g), k[g]))
    root = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..', '.smoke', 'hw', 'opmix', 'mix')
    rows = []
    for f in sorted(glob.glob(os.path.join(root, '*.txt'))):
        cnt = {}
        for line in open(f):
            a, b = line.split()
            if a != 'x': cnt[int(a)] = int(b)
        tot = sum(cnt.values())
        if tot:
            extra = sum(n * (k[CODE[c]] - 1) for c, n in cnt.items() if c in CODE)
            rows.append((os.path.basename(f)[:-4], tot, extra / tot))
    for lang in ('cmm', 'cpp'):
        xs = sorted(r[2] for r in rows if r[0].startswith(lang))
        print('%s: %d programs, extra cycles median %.0f %%, mean %.0f %%, max %.0f %%'
              % (lang, len(xs), 100 * xs[len(xs) // 2], 100 * sum(xs) / len(xs), 100 * xs[-1]))
    for name in ('cpp_test48', 'cpp_test50', 'cpp_test46', 'cmm_proc_fft', 'cmm_ProcDTW', 'cmm_sapho_all', 'cmm_cmm_comp_sqrt'):
        for r in rows:
            if r[0] == name: print('  %-18s +%3.0f %%' % (name, 100 * r[2]))

main()
