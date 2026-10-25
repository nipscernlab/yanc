"""Which mnemonics take an array name as operand, across every golden.asm.
Usage: python arr_ops.py <root>"""
import sys, re, pathlib, collections

root = pathlib.Path(sys.argv[1])
use = collections.defaultdict(set)   # mnemonic -> files
for f in sorted(root.rglob('golden.asm')):
    lines = f.read_text(errors='replace').splitlines()
    arrays = set()
    for ln in lines:
        m = re.search(r'#array[sd]?\s+(\S+)', ln)
        if m:
            arrays.add(m.group(1))
    for ln in lines:
        ln = ln.split('//')[0]
        ln = re.sub(r'^@\S+\s*', '', ln.strip())   # label
        if not ln or ln.startswith('#'):
            continue
        tok = ln.split()
        if len(tok) >= 2 and tok[1] in arrays:
            use[tok[0]].add(str(f.relative_to(root)))
for k in sorted(use):
    v = sorted(use[k])
    print(f'{k:10s} {len(v):4d} files  e.g. {v[0]}')
