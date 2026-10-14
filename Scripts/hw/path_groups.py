#!/usr/bin/env python3
"""path_groups.py <report> [n]: the worst path of each destination block.

A long timing report (Quartus `report_timing -detail summary`, or Vivado
`report_timing -nworst 1`) lists thousands of paths, and the first hundreds
can all end at the same register bank: the second block that limits the
clock stays hidden. This groups the paths by their destination, with the bit
indices and the fitter's duplicate suffixes taken out, and prints the worst
slack and data delay of each group, worst first (n groups, default 20).
"""
import re
import sys


def group_of(node):
    node = re.sub(r'(_OTERM\d+|~DUPLICATE|~reg\d*|_RNI\w+)', '', node)
    node = re.sub(r'\[\d+\]$', '', node)                 # the bit index of the register itself (keeps cut[2])
    node = re.sub(r'[a-z_]+:(?=[\w.\[\]]+(\||$))', '', node)  # Quartus "module:instance" -> instance
    return node


def quartus(lines):
    for ln in lines:
        f = [x.strip() for x in ln.split(';')]
        if len(f) >= 9 and re.match(r'^-?\d+\.\d+$', f[1]):
            yield float(f[1]), f[3], float(f[8])


def vivado(text):
    for blk in re.split(r'\n(?=Slack)', text):
        m = re.match(r'Slack \(\w+\)\s*:\s*(-?[\d.]+)', blk)
        d = re.search(r'Destination:\s*(\S+)', blk)
        dl = re.search(r'Data Path Delay:\s*([\d.]+)', blk)
        if m and d:
            yield float(m.group(1)), d.group(1), float(dl.group(1)) if dl else 0.0


def main():
    path = sys.argv[1]
    n = int(sys.argv[2]) if len(sys.argv) > 2 else 20
    text = open(path, encoding='utf-8', errors='replace').read()
    rows = list(vivado(text)) if 'Data Path Delay' in text else list(quartus(text.splitlines()))
    best = {}
    for slack, to, delay in rows:
        g = group_of(to)
        if g not in best or slack < best[g][0]:
            best[g] = (slack, delay, to)
    print(f'{len(rows)} paths, {len(best)} destination groups')
    print(f'{"slack ns":>9} {"delay ns":>9}  destination')
    for g, (slack, delay, _) in sorted(best.items(), key=lambda kv: kv[1][0])[:n]:
        print(f'{slack:9.3f} {delay:9.3f}  {g}')


if __name__ == '__main__':
    main()
