#!/usr/bin/env python3
# mux_x_cost.py (2026-10-15): does an x input of ula_mux cost logic? Builds, for each set of used
# operations: A = SAPHO's ula_mux with the unused inputs at x, B = a mux with
# only the used arms, C = ula_mux with every input real. Yosys counts LUT4.
import re, os, subprocess
root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..'))
out = os.path.join(root, '.smoke', 'hw', 'muxx')
os.makedirs(out, exist_ok=True)
src = open(root + '/SAPHO/ula.v', encoding='utf-8').read()
mux = re.search(r'module ula_mux\b.*?endmodule', src, re.S).group(0)
open(out + '/ula_mux.v', 'w').write(mux + '\n')

# every input port of ula_mux and the opcode -> input map
ports = []
for m in re.finditer(r'input\s+\[NUBITS-1:0\]\s*([^;/\n]+)', mux):
    ports += [p.strip() for p in m.group(1).split(',') if p.strip()]
arms = re.findall(r"6'd(\d+)\s*:\s*out\s*=\s*(\w+)\s*;", mux)

SETS = {
    'int':   ['in1', 'in2', 'add', 'les', 'equ', 'ann', 'gre', 'shl'],
    'float': ['in1', 'in2', 'add', 'les', 'equ', 'fles', 'fgre', 'smx', 'mlt', 'f2i', 'fneg'],
}

def top_a(name, used, allreal=False):
    conn = []
    for p in ports:
        if p in used or allreal:
            conn.append(f'.{p}(i_{p})')
        else:
            conn.append(f".{p}({{N{{1'bx}}}})")
    decl = ', '.join(f'i_{p}' for p in (ports if allreal else used))
    return (f'module {name} #(parameter N = 32) (input [5:0] op, input [N-1:0] {decl}, output [N-1:0] out);\n'
            f'ula_mux #(N) m (.op(op), {", ".join(conn)}, .out(out));\nendmodule\n')

def top_b(name, used):
    decl = ', '.join(f'i_{p}' for p in used)
    lines = [f"\t6'd{c} : out = i_{s};" for c, s in arms if s in used]
    return (f'module {name} #(parameter N = 32) (input [5:0] op, input [N-1:0] {decl}, output reg [N-1:0] out);\n'
            'always @ (*) case (op)\n' + '\n'.join(lines) +
            "\n\tdefault: out = {N{1'bx}};\nendcase\nendmodule\n")

res = []
for sname, used in SETS.items():
    for kind in ('A', 'B', 'C'):
        name = f'{kind}_{sname}'
        v = top_a(name, used) if kind == 'A' else top_b(name, used) if kind == 'B' else top_a(name, used, True)
        open(f'{out}/{name}.v', 'w').write(v)
        ys = (f'read_verilog ula_mux.v\nread_verilog {name}.v\n'
              f'synth -top {name} -flatten -noshare\nabc -lut 4\nopt_clean\nstat\n')
        open(f'{out}/{name}.ys', 'w').write(ys)
        r = subprocess.run(['yosys', '-q', '-l', f'{name}.log', f'{name}.ys'], cwd=out,
                           capture_output=True, text=True)
        log = open(f'{out}/{name}.log', encoding='utf-8', errors='replace').read()
        m = re.findall(r'\$lut\s+(\d+)', log)
        res.append((sname, kind, m[-1] if m else 'n/a (' + (r.stderr.strip()[:80]) + ')'))
for s, k, n in res:
    print(f'{s:6} {k}  LUT4 = {n}')
used_n = {s: sum(1 for c, x in arms if x in u) for s, u in SETS.items()}
print('arms used:', used_n, 'of', len(arms))
