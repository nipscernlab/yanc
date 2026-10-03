"""Read a Quartus `report_timing -detail full_path` file and print its data
path grouped by block: memory read, operand select, each ALU operator, the
ALU output mux, the jump decision, the PC. A path of 900 cells reads as a
handful of stages, each with the time it takes. Used by fmax.sh.

usage: python3 path_blocks.py <worst_path_full.txt>
"""
import re
import sys

# the instance where a stage ends: an ALU operator (op_*), the output mux, or
# one of the core's blocks
STOP = ('ula_mux', 'pf', 'pc', 'uic1', 'uic2', 'sp', 'isp', 'mem_rtl_0', 'id')


def block(elem):
    parts = [re.sub(r':.*', '', p) for p in elem.split('|')]
    parts = [p for p in parts if not p.startswith('p_') and p != 'processor']
    keep = []
    for p in parts[:-1]:
        keep.append(p)
        if p.startswith('op_') or p in STOP:
            break
    return '/'.join(keep) or parts[-1]


def main(path):
    rows, started, in_data = [], False, False
    for line in open(path, encoding='utf-8', errors='replace'):
        if 'Data Arrival Path' in line:
            started = True
        if 'Data Required Path' in line:
            break
        if not started:
            continue
        f = [c.strip() for c in line.split(';')]
        if len(f) < 8 or not re.match(r'^[0-9.]+$', f[1] or 'x'):
            continue
        if f[7] == 'data path':               # the clock path comes before it
            rows, in_data = [], True
            continue
        if not in_data or '|' not in f[7]:
            continue
        rows.append((float(f[1]), f[7]))
    if not rows:
        print('path_blocks: no data path in', path)
        return 1
    stages, prev, t0 = [], None, rows[0][0]
    for t, e in rows:
        b = block(e)
        if b != prev:
            if prev is not None:
                stages.append((prev, t0, t))
            prev, t0 = b, t
    stages.append((prev, t0, rows[-1][0]))
    total = rows[-1][0] - rows[0][0]
    for b, a, z in stages:
        print('  %8.3f -> %8.3f  %7.3f ns  %3.0f %%  %s' % (a, z, z - a, 100 * (z - a) / total, b))
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1]))
