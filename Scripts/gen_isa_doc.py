#!/usr/bin/env python3
"""Write docs/reference/instructions.md from Compilers/common/isa.tsv.

    python3 Scripts/gen_isa_doc.py [repo root]           # write the page
    python3 Scripts/gen_isa_doc.py [repo root] --check   # fail if it is stale

The table is the source of truth for the mnemonics, opcodes, operand classes
and effects; this script only lays it out. The column "computes" comes from
COMPUTES below, which was read off SAPHO/ula.v (2026-10-13): the ALU's input
in1 is the operand (the data word, or the stack top popped by an S_ form)
and in2 is the accumulator; the result goes to the accumulator. A mnemonic
missing from COMPUTES gets the table's description alone.
"""
import os, sys

# what each base operation computes; x = the operand (data word, popped stack
# top for S_/SF_, the data word for _M), a = the accumulator before
BINARY = {
    'ADD':   'a + x',
    'MLT':   'x * a',
    'DIV':   'starts x / a (integer quotient, toward zero); read it with QUO two words later',
    'MOD':   'starts x % a (remainder, the sign of x); read it with REM two words later',
    'SGN':   'a with the sign of x',
    'F_ADD': 'x + a',
    'F_SU1': 'a - x',
    'F_SU2': 'x - a',
    'F_MLT': 'x * a',
    'F_DIV': 'starts x / a; read it with F_QUO two words later',
    'F_SGN': 'a with the sign of x',
    'F_SCL': 'a * 2^x (x an int, a a float)',
    'AND':   'x & a (bitwise)',
    'ORR':   'x | a (bitwise)',
    'XOR':   'x ^ a (bitwise)',
    'LAN':   'x && a (1 or 0)',
    'LOR':   'x || a (1 or 0)',
    'LES':   'x < a (1 or 0)',
    'GRE':   'x > a (1 or 0)',
    'EQU':   'x == a (1 or 0)',
    'F_LES': 'x < a, floats (1 or 0)',
    'F_GRE': 'x > a, floats (1 or 0)',
    'SHL':   'x << a',
    'SHR':   'x >> a (logical: zeros enter)',
    'SRS':   'x >> a (arithmetic: the sign enters)',
}
UNARY = {                    # {v} = a for the plain form, x for the _M forms
    'NEG':   '-{v}',
    'ABS':   '|{v}|',
    'PST':   '{v}, or 0 when {v} is negative',
    'NRM':   '{v} / NUGAIN (the #NUGAIN directive, a power of two)',
    'INV':   '~{v} (bitwise not)',
    'LIN':   '!{v} (1 or 0)',
    'F_NEG': '-{v}, float',
    'F_ABS': '|{v}|, float',
    'F_PST': '{v}, or 0 when {v} is negative, float',
    'XPO':   'the base-2 exponent of the float {v}, as an int',
    'I2F':   '{v} converted from int to float',
    'F2I':   '{v} converted from float to int',
}
OTHER = {
    'NOP': 'nothing', 'LOD': 'a = x', 'P_LOD': 'push a; a = x',
    'LDI': 'a = mem[x + a] (x a base: an array, or a number)',
    'ILI': 'a = mem[x + bit-reversed a] (FFT addressing)',
    'SET': 'x = a', 'SET_P': 'x = a; a = pop',
    'STI': 'mem[x + pop] = a (the index is popped)',
    'ISI': 'mem[x + bit-reversed pop] = a (FFT addressing)',
    'PSH': 'push a', 'POP': 'a = pop',
    'INN': 'a = input port x (int)', 'F_INN': 'a = input port x (float)',
    'P_INN': 'push a; a = input port x', 'PF_INN': 'push a; a = input port x (float)',
    'OUT': 'output port x = a',
    'JMP': 'go to x', 'JIZ': 'go to x when a is 0', 'CAL': 'call x (the return address is pushed on the subroutine stack)',
    'RET': 'return to the address the last CAL pushed',
    'TOM': 'offer a on link x; wait here until the partner takes it',
    'CAD': 'wait here until link x offers a word; a = that word',
    'F_ROT': 'the power of two nearest to the square root of the float a',
    'QUO': 'a = the quotient of the DIV two words before',
    'REM': 'a = the remainder of the MOD two words before',
    'F_QUO': 'a = the quotient of the F_DIV two words before',
    'LEA': 'a = the address of x (a constant)',
}

FAMILIES = [
    ('Memory and stack',      lambda op: op <= 10),
    ('Input and output',      lambda op: 11 <= op <= 15),
    ('Flow control and links', lambda op: 16 <= op <= 21),
    ('Integer arithmetic',    lambda op: 22 <= op <= 31),
    ('Float arithmetic',      lambda op: 32 <= op <= 45),
    ('Integer unary',         lambda op: 46 <= op <= 63),
    ('Float unary',           lambda op: 64 <= op <= 75),
    ('Conversions',           lambda op: 76 <= op <= 81),
    ('Logic',                 lambda op: 82 <= op <= 91),
    ('Comparisons',           lambda op: 92 <= op <= 101),
    ('Shifts',                lambda op: 102 <= op <= 107),
]

def read_rows(path):
    rows = []
    for line in open(path, encoding='utf-8'):
        line = line.rstrip('\n')
        if not line.strip() or line.lstrip().startswith('#'):
            continue
        p = [x.strip() for x in line.split('\t')]
        rows.append(dict(mn=p[0], op=int(p[1]), cls=p[2], acc=p[3], stack=p[4],
                         opnd=p[5], flow=p[6], io=p[7], desc=p[8] if len(p) > 8 else ''))
    return rows

def computes(mn):
    if mn in OTHER:
        return OTHER[mn]
    base = mn
    for suf in ('_V',):
        if base.endswith(suf):
            return f'as {base[:-2]}, x at a constant offset from the name'
    pre = ''
    for p in ('PF_', 'SF_', 'P_', 'S_'):
        if base.startswith(p):
            pre, base = p, ('F_' if p in ('PF_', 'SF_') else '') + base[len(p):]
            break
    m = base.endswith('_M')
    if m:
        base = base[:-2]
    push = 'push a; ' if pre in ('P_', 'PF_') else ''
    if base in BINARY:
        b = BINARY[base]
        return b if b.startswith('starts') else push + 'a = ' + b
    if base in UNARY:
        return push + 'a = ' + UNARY[base].format(v='x' if m else 'a')
    return ''

def render(rows):
    out = []
    w = out.append
    w('# Instruction set')
    w('')
    w('<!-- GENERATED by Scripts/gen_isa_doc.py from Compilers/common/isa.tsv: do not edit by hand. -->')
    w('')
    w('SAPHO is an accumulator machine: almost every instruction combines the')
    w('accumulator `a` with one operand `x` and leaves the result in `a`. The')
    w('operand is a data word named in the instruction (`ADD k`), or for the')
    w('`S_`/`SF_` forms the top of the data stack, which the instruction pops.')
    w('The order matters for the operations that are not commutative: `LES k`')
    w('gives `k < a`, `F_SU2 k` gives `k - a`, `DIV k` divides `k` by `a`.')
    w('')
    w('The names follow one convention: `F` marks a float operation; a `P_`/`PF_`')
    w('prefix pushes the accumulator first; an `S_`/`SF_` prefix takes the operand')
    w('off the stack; an `_M` suffix applies a unary operation to the data word')
    w('instead of the accumulator; a `_V` suffix adds a constant offset to the')
    w('address (an array element at a known index). Several mnemonics share an')
    w('opcode and differ only in how the assembler reads the operand.')
    w('')
    w('A processor contains only the hardware of the instructions its program')
    w('uses. A division takes three words, `DIV k; NOP; QUO` (also `MOD`/`REM`,')
    w('`F_DIV`/`F_QUO`): the compilers write it that way, because the dividers')
    w('take three cycles.')
    w('')
    w('Columns: **operand**, what the operand field holds (`data` a data address,')
    w('`code` an instruction address, `in`/`out` a port, `link` a link number,')
    w('`offset` an address plus a constant, `lea` an address taken as a constant);')
    w('**a**, what the instruction does to the accumulator (`r` reads, `w` writes);')
    w('**stack**, `push` or `pop` of one data-stack word; **x**, what it does to the')
    w('data word named (`r`, `w`, `base_r`/`base_w` an unknown word of the array')
    w('named, `addr` its address only).')
    w('')
    for title, test in FAMILIES:
        fam = [r for r in rows if test(r['op'])]
        if not fam:
            continue
        w(f'## {title}')
        w('')
        w('| mnemonic | opcode | operand | a | stack | x | computes |')
        w('|---|---|---|---|---|---|---|')
        for r in sorted(fam, key=lambda r: (r['op'], r['mn'])):
            c = computes(r['mn']) or r['desc']
            c = c.replace('|', '\\|')
            w(f"| `{r['mn']}` | {r['op']} | {r['cls']} | {r['acc']} | {r['stack']} | {r['opnd']} | {c} |")
        w('')
    return '\n'.join(out)

def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    root = args[0] if args else os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
    check = '--check' in sys.argv
    rows = read_rows(os.path.join(root, 'Compilers', 'common', 'isa.tsv'))
    text = render(rows)
    dest = os.path.join(root, 'docs', 'reference', 'instructions.md')
    if check:
        old = open(dest, encoding='utf-8').read().replace('\r\n', '\n') if os.path.exists(dest) else ''
        if old != text:
            sys.exit('gen_isa_doc: docs/reference/instructions.md is stale; run Scripts/gen_isa_doc.py')
        print(f'gen_isa_doc: docs/reference/instructions.md matches isa.tsv ({len(rows)} mnemonics)')
        return
    os.makedirs(os.path.dirname(dest), exist_ok=True)
    open(dest, 'w', encoding='utf-8', newline='\n').write(text)
    print(f'gen_isa_doc: wrote {dest} ({len(rows)} mnemonics)')

if __name__ == '__main__':
    main()
