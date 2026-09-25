#!/usr/bin/env python3
"""Hold the hand-written copies of the instruction set to Compilers/common/isa.tsv.

The ISA is written down in four places that nothing keeps in step: the
assembler's lexer (mnemonic -> opcode + operand class), `instr_dec.v` and
`ula.v` (opcode -> control signals and ALU operation) and `core.v` (which
hard-codes the four flow-control opcodes as 5-bit literals). A disagreement
between them is silent: a program assembles and then runs as a different
program. TODO.md item 10.2.

This checks what can be checked without parsing Verilog beyond recognising a
literal, which is the part that stays true when the HDL is edited:

  1. the table and ASMComp.l list exactly the same mnemonics,
  2. with the same opcode and the same operand class,
  3. the opcodes form a gap-free range starting at 0,
  4. mnemonics that share an opcode differ only in operand class,
  5. core.v's four hard-coded literals are the table's JMP/JIZ/CAL/RET,
  6. every row's effect columns follow the naming convention (prefix P_/PF_
     pushes, S_/SF_ pops, suffix _M reads the operand from memory, _V is its
     base with a constant offset) and agree with the operand class,
  7. the copy in Compilers/common/asm_share.c (operand class, operand effect,
     flow) is the table's.

It does NOT try to verify instr_dec.v or ula.v line by line: matching their
comparisons to opcodes with a regular expression breaks on innocuous edits,
which would make the check noise rather than a guard. Generating all four
copies from the table is the rest of item 10.2.

usage: python3 Scripts/check_isa.py [repo-root]     (exit 1 on any mismatch)
"""
import os
import re
import sys

CLASS_OF_STATE = {0: 'none', 18: 'data', 19: 'code', 20: 'in',
                  21: 'out', 22: 'offset', 24: 'lea'}
FLOW = ('JMP', 'JIZ', 'CAL', 'RET')


effects = {}          # mnemonic -> (acc, stack, opnd, flow, io), filled by read_table


def read_table(path):
    """The table: mnemonic -> (opcode, operand class); effects land in `effects`."""
    out = {}
    for n, line in enumerate(open(path, encoding='utf-8'), 1):
        line = line.rstrip('\n')
        if not line.strip() or line.lstrip().startswith('#'):
            continue
        parts = line.split('\t')
        if len(parts) < 3:
            sys.exit(f'{path}:{n}: expected at least 3 tab-separated fields')
        if len(parts) < 8:
            sys.exit(f'{path}:{n}: expected 8 tab-separated fields '
                     f'(mnemonic opcode operand acc stack opnd flow io), got {len(parts)}')
        mn, op, cls = parts[0].strip(), parts[1].strip(), parts[2].strip()
        eff = tuple(x.strip() for x in parts[3:8])
        if mn in out:
            sys.exit(f'{path}:{n}: mnemonic {mn} listed twice')
        if not op.isdigit():
            sys.exit(f'{path}:{n}: opcode {op!r} is not a number')
        if cls not in CLASS_OF_STATE.values():
            sys.exit(f'{path}:{n}: unknown operand class {cls!r}')
        out[mn] = (int(op), cls)
        effects[mn] = eff
    return out


# ---- the effect columns, re-derived from the names -------------------------
# The ISA names say what an instruction does: P_/PF_ push the accumulator
# before the operation, S_/SF_ take the second operand off the stack, _M reads
# the operand from memory instead of the accumulator, _V adds a constant
# offset. Re-deriving from the name and comparing catches a typo in the table;
# it does NOT confirm the convention itself, which only execution can (the
# simulator of TODO item 10.1).
BINARY = ('ADD', 'MLT', 'DIV', 'MOD', 'SGN', 'AND', 'ORR', 'XOR', 'LAN', 'LOR',
          'LES', 'GRE', 'EQU', 'SHL', 'SHR', 'SRS', 'SU1', 'SU2', 'SCL')
UNARY = ('NEG', 'ABS', 'PST', 'NRM', 'I2F', 'F2I', 'INV', 'LIN', 'XPO', 'ROT')
# the ones the naming convention cannot describe, taken from ASMComp.l's own
# per-mnemonic comments
IRREGULAR = {
    'LOD':   ('w', '-', 'r', '-', '-'),
    'SET':   ('r', '-', 'w', '-', '-'),
    'PSH':   ('r', 'push', '-', '-', '-'),
    'POP':   ('w', 'pop', '-', '-', '-'),
    'SET_P': ('rw', 'pop', 'w', '-', '-'),
    'LDI':   ('rw', '-', 'base_r', '-', '-'),
    'ILI':   ('rw', '-', 'base_r', '-', '-'),
    'STI':   ('r', 'pop', 'base_w', '-', '-'),
    'ISI':   ('r', 'pop', 'base_w', '-', '-'),
    'LDA':   ('rw', '-', '-', '-', '-'),
    'STA':   ('r', 'pop', '-', '-', '-'),
    'LEA':   ('w', '-', 'addr', '-', '-'),
    'INN':   ('w', '-', '-', '-', 'in'),
    'F_INN': ('w', '-', '-', '-', 'in'),
    'P_INN': ('rw', 'push', '-', '-', 'in'),
    'PF_INN': ('rw', 'push', '-', '-', 'in'),
    'OUT':   ('r', '-', '-', '-', 'out'),
    'JMP':   ('-', '-', '-', 'jmp', '-'),
    'JIZ':   ('r', '-', '-', 'jz', '-'),
    'CAL':   ('-', '-', '-', 'call', '-'),
    'RET':   ('-', '-', '-', 'ret', '-'),
    'NOP':   ('-', '-', '-', '-', '-'),
}


def derive(mn, cls):
    """The effects the mnemonic's own name implies, or None if it says nothing."""
    if mn in IRREGULAR:
        return IRREGULAR[mn]
    m = mn
    if m.endswith('_V'):                       # a constant offset changes no effect
        m = m[:-2]
    if m in IRREGULAR:
        return IRREGULAR[m]
    push = pop = False
    if m.startswith('PF_'):
        push, m = True, m[3:]
    elif m.startswith('P_'):
        push, m = True, m[2:]
    if m.startswith('SF_'):
        pop, m = True, m[3:]
    elif m.startswith('S_'):
        pop, m = True, m[2:]
    if m in IRREGULAR:                         # a decorated irregular, e.g. P_LOD
        acc, stack, opnd, flow, io = IRREGULAR[m]
        if push:
            acc, stack = 'rw', 'push'
        if pop:
            stack = 'pop'
        return (acc, stack, opnd, flow, io)
    if m.startswith('F_'):                     # the float flavour has the same shape
        m = m[2:]
    from_mem = m.endswith('_M')
    if from_mem:
        m = m[:-2]
    if m in BINARY:
        acc = 'rw'
        opnd = '-' if pop else ('r' if cls in ('data', 'offset') else '-')
    elif m in UNARY:
        acc = 'w' if from_mem else 'rw'
        opnd = 'r' if from_mem else '-'
    else:
        return None
    if push:
        acc = 'rw'
    return (acc, 'push' if push else ('pop' if pop else '-'), opnd, '-', '-')


def read_lexer(path):
    """ASMComp.l: mnemonic -> (opcode, operand class), from eval_opcode()."""
    out = {}
    text = open(path, encoding='utf-8', errors='replace').read()
    for m in re.finditer(r'"([A-Z_0-9]+)"\s*eval_opcode\(\s*(\d+)\s*,\s*(\d+)', text):
        mn, op, st = m.group(1), int(m.group(2)), int(m.group(3))
        if st not in CLASS_OF_STATE:
            sys.exit(f'{path}: {mn} uses parser state {st}, which this script '
                     f'does not know: teach it the operand class it means.')
        out[mn] = (op, CLASS_OF_STATE[st])
    return out


def read_core_flow(path):
    """core.v: the 5-bit literals it compares the opcode against, in order."""
    text = open(path, encoding='utf-8', errors='replace').read()
    seen = []
    for m in re.finditer(r"5'd(\d+)", text):
        v = int(m.group(1))
        if v not in seen:
            seen.append(v)
    return seen


def main():
    root = sys.argv[1] if len(sys.argv) > 1 else \
        os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    table_p = os.path.join(root, 'Compilers', 'common', 'isa.tsv')
    lexer_p = os.path.join(root, 'Compilers', 'ASMComp', 'Sources', 'ASMComp.l')
    core_p = os.path.join(root, 'HDL', 'core.v')
    for p in (table_p, lexer_p, core_p):
        if not os.path.exists(p):
            sys.exit(f'check_isa: missing {p}')

    table = read_table(table_p)
    lexer = read_lexer(lexer_p)
    bad = []

    # 1 + 2: same mnemonics, same opcode, same operand class
    for mn in sorted(set(table) | set(lexer)):
        t, l = table.get(mn), lexer.get(mn)
        if t is None:
            bad.append(f'{mn}: in ASMComp.l (opcode {l[0]}, {l[1]}) but not in the table')
        elif l is None:
            bad.append(f'{mn}: in the table (opcode {t[0]}, {t[1]}) but not in ASMComp.l')
        elif t != l:
            bad.append(f'{mn}: table says opcode {t[0]} / {t[1]}, '
                       f'ASMComp.l says opcode {l[0]} / {l[1]}')

    # 3: a gap-free range from 0
    opcodes = sorted({op for op, _ in table.values()})
    if opcodes and opcodes[0] != 0:
        bad.append(f'opcodes start at {opcodes[0]}, expected 0')
    missing = sorted(set(range(opcodes[0], opcodes[-1] + 1)) - set(opcodes)) if opcodes else []
    if missing:
        bad.append(f'no mnemonic for opcode(s) {missing} inside the used range')

    # 4: mnemonics sharing an opcode must differ in operand class
    share = {}
    for mn, (op, cls) in table.items():
        share.setdefault(op, []).append((mn, cls))
    for op, lst in sorted(share.items()):
        classes = [c for _, c in lst]
        if len(lst) > 1 and len(set(classes)) != len(classes):
            names = ', '.join(m for m, _ in lst)
            bad.append(f'opcode {op}: {names} share an opcode AND an operand '
                       f'class, so the assembler cannot tell them apart')

    # 5: core.v's flow-control literals
    want = [table[m][0] for m in FLOW if m in table]
    got = read_core_flow(core_p)
    if len(want) != len(FLOW):
        bad.append(f'the table is missing one of {", ".join(FLOW)}')
    elif sorted(got) != sorted(want):
        bad.append(f'core.v compares the opcode against {sorted(got)}, but the '
                   f'table puts {", ".join(FLOW)} at {want}')

    # 6: the effect columns must be what the mnemonic's own name implies, and
    # must agree with the operand class
    for mn in sorted(table):
        op, cls = table[mn]
        got = effects.get(mn)
        want = derive(mn, cls)
        if want is None:
            bad.append(f'{mn}: its name follows no known pattern, so the effect '
                       f'columns cannot be checked: add it to IRREGULAR in this '
                       f'script, with the reason')
        elif got != want:
            bad.append(f'{mn}: table says {"/".join(got)}, the name implies '
                       f'{"/".join(want)}')
        if cls in ('data', 'offset') and got and got[2] == '-':
            bad.append(f'{mn}: takes a data operand but the table says it does '
                       f'nothing with it')
        if cls == 'none' and got and got[2] != '-':
            bad.append(f'{mn}: takes no operand but the table gives it the '
                       f'operand effect {got[2]}')
        if cls == 'code' and got and got[3] == '-':
            bad.append(f'{mn}: takes a code address but the table gives it no '
                       f'flow effect')
        if cls == 'in' and got and got[4] != 'in':
            bad.append(f'{mn}: reads an input port but the table says io={got[4]}')
        if cls == 'out' and got and got[4] != 'out':
            bad.append(f'{mn}: writes an output port but the table says io={got[4]}')

    # 7: asm_share.c's copy of the operand class, operand effect and flow
    share_p = os.path.join(root, 'Compilers', 'common', 'asm_share.c')
    if os.path.exists(share_p):
        text = open(share_p, encoding='utf-8', errors='replace').read()
        rows = {m.group(1): (m.group(2), m.group(3), m.group(4)) for m in re.finditer(
            r'\{"([A-Z_0-9]+)",\s*"([a-z]+)",\s*"([a-z_-]+)",\s*"([a-z-]+)"\}', text)}
        for mn in sorted(set(table) | set(rows)):
            if mn not in rows:
                bad.append(f'{mn}: in the table but not in asm_share.c')
            elif mn not in table:
                bad.append(f'{mn}: in asm_share.c but not in the table')
            else:
                want = (table[mn][1], effects[mn][2], effects[mn][3])
                if rows[mn] != want:
                    bad.append(f'{mn}: asm_share.c says {"/".join(rows[mn])}, '
                               f'the table says {"/".join(want)}')

    if bad:
        print('check_isa: the instruction set does not agree with itself', file=sys.stderr)
        for b in bad:
            print(f'  {b}', file=sys.stderr)
        return 1
    print(f'check_isa: {len(table)} mnemonics, {len(opcodes)} opcodes '
          f'(0..{opcodes[-1]}); ASMComp.l, core.v, asm_share.c and the effect columns agree')
    return 0


if __name__ == '__main__':
    sys.exit(main())
