"""Check that a generated processor instantiates every optional block.

Usage: python3 Scripts/check_blocks.py <proc.v> <SAPHO/processor.v> [exceptions]

Every opcode parameter of `processor` (the ones that gate a `generate` block:
from P_LOD to the last one) must be passed as (1) in the generated top, and FFTSIZ,
ITRADD and TOAQUIADDR must be set, unless the parameter is listed in the
exceptions file (`NAME  reason`, '#' comments). An exception that IS
instantiated is also an error, so the list cannot go stale. Used by
regress.sh for the sapho_all fixture.
"""
import re, sys

proc_v, hdl_v = sys.argv[1], sys.argv[2]
exc = {}
if len(sys.argv) > 3:
    for ln in open(sys.argv[3], encoding='utf-8'):
        ln = ln.split('#')[0].strip()
        if ln:
            name, _, why = ln.partition(' ')
            exc[name] = why.strip()

hdl = open(hdl_v, encoding='utf-8').read()
head = hdl[hdl.index('module processor'):]
head = head[:head.index(');')]
params = re.findall(r'parameter\s+(\w+)\s*=', head)
ops = params[params.index('P_LOD'):]

top = open(proc_v, encoding='utf-8').read()
given = dict(re.findall(r'\.(\w+)\s*\(\s*([^)]*?)\s*\)', top))

bad = []
for p in ops:
    on = given.get(p) == '1'
    if p in exc:
        if on: bad.append(f'{p}: listed as an exception but instantiated')
    elif not on:
        bad.append(f'{p}: not instantiated')
for p in ('FFTSIZ', 'ITRADD', 'TOAQUIADDR'):
    v = given.get(p)
    if p in exc:
        continue
    if v is None or v.strip() in ('0', ''):
        bad.append(f'{p}: not set')
for p in exc:
    if p not in ops and p not in ('FFTSIZ', 'ITRADD', 'TOAQUIADDR'):
        bad.append(f'{p}: exception names no processor parameter')

if bad:
    print('\n'.join(bad))
    sys.exit(1)
print(f'check_blocks: {len(ops) - len(exc)} of {len(ops)} opcode blocks instantiated, '
      f'{len(exc)} excepted; FFT, #PRACA and #TOAQUI set')
