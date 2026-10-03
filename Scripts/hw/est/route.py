"""Add a routing estimate to the Yosys xc7 critical paths: every net on the
path that is not inside a carry chain (CARRY4 CO -> CARRY4 CI) goes through
the routing fabric and costs r ns. Prints delay / base for several r."""
import re, os
HERE = os.path.dirname(os.path.abspath(__file__))

def path(var):
    log = open(os.path.join(HERE, 'runs', 'xc7_' + var, 'r.log'), errors='replace').read()
    i = log.index("Latest arrival time in 'sapho_all'")
    rows = []
    for line in log[i:].split('\n')[1:]:
        m = re.match(r'\s+(\d+)\s+\S+\s+\((\w+)\.(\S+)->(\S+)\)', line)
        if m:
            rows.append((int(m.group(1)), m.group(2), m.group(3), m.group(4)))
        elif line.strip().startswith('0 ') or 'primary input' in line:
            break
        elif rows and not line.startswith(' '):
            break
    return rows

def stats(var):
    rows = path(var)
    total = rows[0][0] if rows else 0
    # rows go from the endpoint back to the start; a net is routed unless it
    # joins two carry cells (the CO of one feeds the CI of the next)
    routed = 0
    for k in range(len(rows) - 1):
        cell, frm, to = rows[k][1], rows[k][2], rows[k][3]
        prev = rows[k + 1][1]
        if cell == 'CARRY4' and frm == 'CI' and prev == 'CARRY4':
            continue
        routed += 1
    return total / 1000.0, routed, len(rows)

vars_ = ['base', 'fadd', 'fmlt', 'div', 'fdiv', 'shift', 'cmp', 'i2f', 'nrm']
st = {v: stats(v) for v in vars_}
for v in vars_:
    print('%-6s logic %6.2f ns  cells %4d  routed nets %4d' % (v, st[v][0], st[v][2], st[v][1]))
print()
print('%-6s %s' % ('r(ns)', '  '.join('%6s' % v for v in vars_)))
for r in (0.0, 0.3, 0.5, 0.8):
    tb = st['base'][0] + r * st['base'][1]
    print('%-6s %s' % (r, '  '.join('%6.2f' % ((st[v][0] + r * st[v][1]) / tb) for v in vars_)))
