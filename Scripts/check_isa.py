#!/usr/bin/env python3
"""Hold the hand-written copies of the instruction set to Compilers/common/isa.tsv.

The ISA is written down in four places that nothing keeps in step: the
assembler's lexer (mnemonic -> opcode + operand class), `instr_dec.v` and
`ula.v` (opcode -> control signals and ALU operation) and `core.v` (which
names the four flow-control opcodes in localparams). A disagreement
between them is silent: a program assembles and then runs as a different
program. GitHub issue #33.

This checks what can be checked without parsing Verilog beyond recognising a
literal, which is the part that stays true when the HDL is edited:

  1. the table and ASMComp.l list exactly the same mnemonics,
  2. with the same opcode and the same operand class,
  3. the opcodes form a gap-free range starting at 0,
  4. mnemonics that share an opcode differ only in operand class,
  5. core.v's OP_JMP/OP_JIZ/OP_JNZ/OP_CAL/OP_RET/OP_TOM/OP_CAD are the table's,
  6. every row's effect columns follow the naming convention (prefix P_/PF_
     pushes, S_/SF_ pops, suffix _M reads the operand from memory, _V is its
     base with a constant offset) and agree with the operand class,
  7. the copy in Compilers/common/asm_share.c (operand class, operand effect,
     flow, data stack, acc effect, I/O) is the table's,
  8. every row of instr_dec.v's decode table (`generate if ((NAME) != 0)`) carries its
     mnemonic's opcode, and every decoded mnemonic of the table has a row,
  9. asmcomp's usage totals (ISA_PARAMS, ULA_BLOCKS in opcodes.c) are the
     number of opcode parameters in ASMComp.l and of ALU blocks in opcodes.c,
 10. every mnemonic is a three-character name, with at most a prefix
     (P_, F_, PF_, S_, SF_) and a suffix (_M, _V, _P), each after or before an
     underscore: JIZ, F_ADD, P_NEG_M, SF_MLT, LDI_V,
 12. the cycles of the two dividers agree: ula.v's IDIV_STG and FDIV_ALONE,
     the STG defaults of ula_idiv, ula_div_sh and ula_fdiv, and the words the compilers
     give a division (ASM_DIV_WORDS_INT, ASM_DIV_WORDS_FLT in asm_share.h).
     A divider one stage longer than the compilers think makes QUO, REM and
     F_QUO read the result a cycle early, with nothing to say so (#185),
 13. the ALU operation numbers agree: each row of instr_dec.v runs the ALU
     operation its mnemonic names (PF_X and SF_X run F_X, P_X and S_X run X,
     X_V runs X, a few listed in ULA_IRREGULAR), the names being those of
     ula.v's output muxes, and every other 6'dN in ula.v names operation N on
     its line, in the code or in the comment (#174). core.v's flow opcodes
     are checked in every module that writes them (OP_TOM and OP_CAD are in
     two).

It does NOT verify the control lines of each row of instr_dec.v nor what
ula.v computes: that is execution, the instruction-set simulator of issue
#32. Generating the decode rows from the table is issue #33.

usage: python3 Scripts/check_isa.py [repo-root]     (exit 1 on any mismatch)
"""
import os
import re
import sys

CLASS_OF_STATE = {0: 'none', 18: 'data', 19: 'code', 20: 'in',
                  21: 'out', 22: 'offset', 24: 'lea',
                  28: 'data',   # 28: LDI/ILI/STI/ISI, a data name or a raw base number
                  29: 'link'}   # 29: TOM/CAD, the link number
FLOW = ('JMP', 'JIZ', 'JNZ', 'CAL', 'RET', 'TOM', 'CAD')
NAME_PREFIX = ('P', 'F', 'PF', 'S', 'SF')   # rule 10: what may stand before the core
NAME_SUFFIX = ('M', 'V', 'P')               # and after it (SET_P: SET and POP)


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
# simulator of issue #32).
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
    'LEA':   ('w', '-', 'addr', '-', '-'),
    'INC':   ('w', '-', 'rw', '-', '-'),     # k = k + 1 in memory, the acc gets it (#203)
    'INN':   ('w', '-', '-', '-', 'in'),
    'F_INN': ('w', '-', '-', '-', 'in'),
    'P_INN': ('rw', 'push', '-', '-', 'in'),
    'PF_INN': ('rw', 'push', '-', '-', 'in'),
    'OUT':   ('r', '-', '-', '-', 'out'),
    'JMP':   ('-', '-', '-', 'jmp', '-'),
    'JIZ':   ('r', '-', '-', 'cond', '-'),
    'JNZ':   ('r', '-', '-', 'cond', '-'),
    'CAL':   ('-', '-', '-', 'call', '-'),
    'RET':   ('-', '-', '-', 'ret', '-'),
    'NOP':   ('-', '-', '-', '-', '-'),
    # the links between processors (docs/design/toma-and-cade-links.md): each waits
    # in place (the prefetch re-fetches it) and then falls through, so no flow
    # effect for the asm passes; the operand is the link number
    'TOM':   ('r', '-', '-', '-', '-'),
    'CAD':   ('w', '-', '-', '-', '-'),
    # the reads of a division's result (docs/design/pipeln-and-division.md section 4):
    # they write the accumulator and name no data word
    'QUO':   ('w', '-', '-', '-', '-'),
    'REM':   ('w', '-', '-', '-', '-'),
    'F_QUO': ('w', '-', '-', '-', '-'),
}
# operand-less aliases of an opcode that has its own decode row in
# instr_dec.v (the division it reads): they need no row of their own
ALIASES = ('QUO', 'REM', 'F_QUO')


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
    """core.v: its flow-control opcode localparams, name -> every value given.

    OP_TOM and OP_CAD are written in two modules (prefetch and core): each
    copy is returned, so a wrong one cannot hide behind the other (#174)."""
    text = open(path, encoding='utf-8', errors='replace').read()
    out = {}
    for m in re.finditer(r"\bOP_(JMP|JIZ|JNZ|CAL|RET|TOM|CAD)\s*=\s*(\d+)", text):
        out.setdefault(m.group(1), []).append(int(m.group(2)))
    return out


# ---- the ALU operation numbers (#174) ----------------------------------------
# instr_dec.v gives each row an ALU operation, {6'dN, ...}, and ula.v selects
# on op == 6'dN in many places. The names come from ula_mux and norm_mux in
# ula.v: each line `wire [..] tN ... // NAME` (lN, dN in norm_mux) is
# operation N, and NAME is the parameter that gates it (NOP and LOD have none).
ULA_LINE = re.compile(r"^\s*wire\s*\[[^\]]*\]\s*([tld])(\d+)\s*[;=](.*?)//\s*(\w+)")
# the mnemonics whose ALU operation is not the one their name gives: the
# indirect loads and POP load (LOD), F_INN and PF_INN convert the port's word
# (I2F), INC adds 1 on the integer adder (ADD, #203)
ULA_IRREGULAR = {'LDI': 'LOD', 'ILI': 'LOD', 'SET_P': 'LOD', 'POP': 'LOD',
                 'F_INN': 'I2F', 'PF_INN': 'I2F', 'INC': 'ADD', 'P_INC': 'ADD'}


def read_ula_ops(text, bad):
    """ula.v: ALU operation number -> its name, from the output muxes."""
    ops = {}
    for line in text.splitlines():
        m = ULA_LINE.match(line)
        if not m:
            continue
        n, body, name = int(m.group(2)), m.group(3), m.group(4)
        lit = re.findall(r"6'd(\d+)", body)
        if lit and any(int(x) != n for x in lit):
            bad.append(f'ula.v: {m.group(1)}{n} ({name}) selects op {", ".join(lit)}')
        gate = re.search(r"if\s*\(\(\s*(\w+)\s*\)\s*!=\s*0", body)
        if gate and gate.group(1) != name:
            bad.append(f'ula.v: {m.group(1)}{n} is gated by {gate.group(1)} but named {name}')
        if ops.setdefault(n, name) != name:
            bad.append(f'ula.v: op {n} is {ops[n]} in one mux and {name} in another')
    return ops


def alu_name(mn):
    """The ALU operation a mnemonic's name implies: PF_X and SF_X are F_X,
    P_X and S_X are X, X_V is X."""
    if mn in ULA_IRREGULAR:
        return ULA_IRREGULAR[mn]
    b = mn
    for pre, to in (('PF_', 'F_'), ('SF_', 'F_'), ('P_', ''), ('S_', '')):
        if b.startswith(pre):
            b = to + b[len(pre):]
            break
    return b[:-2] if b.endswith('_V') else b


# a row of instr_dec.v: generate if ((NAME) != 0) ... (opcode == 7'dN) ? {6'dALU, ...}
DEC_ROW = r"generate\s+if\s*\(\(\s*(\w+)\s*\)\s*!=\s*0\)\s*begin\s*:\s*\w+\s+assign\s+\w+\s*=\s*\(opcode\s*==\s*7'd(\d+)\s*\)\s*\?\s*\{6'd(\d+)"


def read_decoder(path):
    """instr_dec.v: its decode table, mnemonic -> the opcode of its row."""
    text = open(path, encoding='utf-8', errors='replace').read()
    rows = {}
    for m in re.finditer(DEC_ROW, text):
        rows.setdefault(m.group(1), []).append(int(m.group(2)))
    return rows


def main():
    root = sys.argv[1] if len(sys.argv) > 1 else \
        os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    table_p = os.path.join(root, 'Compilers', 'common', 'isa.tsv')
    lexer_p = os.path.join(root, 'Compilers', 'ASMComp', 'Sources', 'ASMComp.l')
    core_p = os.path.join(root, 'SAPHO', 'core.v')
    dec_p = os.path.join(root, 'SAPHO', 'instr_dec.v')
    for p in (table_p, lexer_p, core_p, dec_p):
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

    # 5: core.v's flow-control localparams
    got = read_core_flow(core_p)
    for m in FLOW:
        if m not in table:
            bad.append(f'the table is missing {m}')
        elif not got.get(m):
            bad.append(f'core.v has no OP_{m}, the table puts {m} at {table[m][0]}')
        else:
            for v in got[m]:
                if v != table[m][0]:
                    bad.append(f'core.v has OP_{m} = {v}, the table puts {m} at {table[m][0]}')
    # an opcode must fit the 7 bits of the instruction word (NBITS_OPC in
    # asmcomp's eval.c, NBOPCO in SAPHO/): core.v compares the flow opcodes
    # NBOPCO bits wide, so any number that fits is safe there
    for m, (op, _) in table.items():
        if op >= 128:
            bad.append(f'{m} is opcode {op}: it does not fit the 7-bit opcode field')

    # 8: instr_dec.v's decode table
    rows = read_decoder(dec_p)
    for mn, ops in sorted(rows.items()):
        if mn not in table:
            bad.append(f'instr_dec.v decodes {mn}, which is not in the table')
        elif len(ops) != 1:
            bad.append(f'instr_dec.v has {len(ops)} rows for {mn}')
        elif ops[0] != table[mn][0]:
            bad.append(f'instr_dec.v decodes {mn} at {ops[0]}, the table puts it at {table[mn][0]}')
    decoded_in_core = set(FLOW) | {'NOP'}
    for mn, (op, cls) in sorted(table.items()):
        if cls in ('lea', 'offset') or mn in decoded_in_core:
            continue                       # share a base opcode / decoded in core.v
        if mn in ALIASES and any(table.get(r, (None,))[0] == op for r in rows):
            continue                       # the division's row decodes it
        if mn not in rows:
            bad.append(f'{mn} (opcode {op}) has no row in instr_dec.v')

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

    # 7: asm_share.c's copy of the operand class, operand effect, flow, stack, acc and I/O
    share_p = os.path.join(root, 'Compilers', 'common', 'asm_share.c')
    if os.path.exists(share_p):
        text = open(share_p, encoding='utf-8', errors='replace').read()
        rows = {m.group(1): m.group(2, 3, 4, 5, 6, 7) for m in re.finditer(
            r'\{"([A-Z_0-9]+)",\s*"([a-z]+)",\s*"([a-z_-]+)",\s*"([a-z-]+)",\s*"([a-z-]+)",'
            r'\s*"([a-z-]+)",\s*"([a-z-]+)"\}', text)}
        for mn in sorted(set(table) | set(rows)):
            if mn not in rows:
                bad.append(f'{mn}: in the table but not in asm_share.c')
            elif mn not in table:
                bad.append(f'{mn}: in asm_share.c but not in the table')
            else:
                want = (table[mn][1], effects[mn][2], effects[mn][3], effects[mn][1],
                        effects[mn][0], effects[mn][4])
                if rows[mn] != want:
                    bad.append(f'{mn}: asm_share.c says {"/".join(rows[mn])}, '
                               f'the table says {"/".join(want)}')

    # 11: asm_share.c's list of the mnemonics whose ALU operation goes through
    # the float normaliser (asm_normnop puts a NOP between one of them and a
    # JIZ, LDI or ILI right after it): every mnemonic whose opcode has an instr_dec.v row with
    # ALU operation 3, 5, 7, 25, 26, 46, 47 or 48, aliases included
    if os.path.exists(share_p):
        dec_text = open(os.path.join(root, 'SAPHO', 'instr_dec.v'), encoding='utf-8', errors='replace').read()
        norm_ops = {int(m.group(2)) for m in re.finditer(DEC_ROW, dec_text)
            if int(m.group(3)) in (3, 5, 7, 25, 26, 46, 47, 48)}
        want = {mn for mn in table if table[mn][0] in norm_ops}
        m = re.search(r'norm_mn\[\]\s*=\s*\{(.*?)\};', text, re.S)
        have = set(re.findall(r'"([A-Z_0-9]+)"', m.group(1))) if m else set()
        for mn in sorted(want - have):
            bad.append(f'{mn}: goes through the normaliser but is not in asm_share.c norm_mn')
        for mn in sorted(have - want):
            bad.append(f'{mn}: in asm_share.c norm_mn but does not go through the normaliser')

    # 10: asmcomp's first pass lists the mnemonics too (it counts the
    # instructions that size the instruction memory): a mnemonic it does not
    # know is not counted, and MINSTS comes out short (seen with TOM/CAD,
    # 2026-10-07)
    app_p = os.path.join(root, 'Compilers', 'ASMComp', 'Sources', 'pass1.l')
    if os.path.exists(app_p):
        app_text = open(app_p, encoding='utf-8', errors='replace').read()
        app_mn = set(re.findall(r'"([A-Z_0-9]+)"\s*p1_(?:opcode|divop)\(', app_text))   # p1_divop: a division (#151)
        asm_mn = set(lexer)
        for mn in sorted(asm_mn - app_mn):
            bad.append(f'{mn}: in ASMComp.l but not in pass1.l (the first pass would not count it)')
        for mn in sorted(app_mn - asm_mn):
            bad.append(f'{mn}: in pass1.l but not in ASMComp.l')

    # 10: three characters at the core of every name
    for mn in sorted(table):
        parts = mn.split('_')
        while len(parts) > 1 and parts[0] in NAME_PREFIX:
            parts = parts[1:]
        while len(parts) > 1 and parts[-1] in NAME_SUFFIX:
            parts = parts[:-1]
        if len(parts) != 1 or not re.fullmatch(r'[A-Z][A-Z0-9]{2}', parts[0]):
            bad.append(f'{mn}: a mnemonic is three characters, with only the '
                       f'prefixes {"/".join(NAME_PREFIX)} and the suffixes '
                       f'{"/".join(NAME_SUFFIX)} around them')

    # 9: the totals asmcomp's usage report divides by
    opc_p = os.path.join(root, 'Compilers', 'ASMComp', 'Sources', 'opcodes.c')
    lex = open(lexer_p, encoding='utf-8', errors='replace').read()
    opc = open(opc_p, encoding='utf-8', errors='replace').read()
    n_par = len(set(re.findall(r'eval_opcode\(\s*\d+\s*,\s*\d+\s*,\s*yytext\s*,\s*"(\w+)"', lex)))
    n_blk = len(set(re.findall(r'printf\((MSG_INFO_\w+)\);\s*u_count\+\+', opc)))
    for name, want in (('ISA_PARAMS', n_par), ('ULA_BLOCKS', n_blk)):
        m = re.search(r'#define\s+' + name + r'\s+(\d+)', opc)
        if not m:
            bad.append(f'opcodes.c defines no {name}')
        elif int(m.group(1)) != want:
            bad.append(f'opcodes.c has {name} {m.group(1)}, the code has {want}')

    # 13: the ALU operation numbers: instr_dec.v's {6'dN, ...} and every
    # 6'dN in ula.v, against the output muxes' names (#174)
    ula = open(os.path.join(root, 'SAPHO', 'ula.v'), encoding='utf-8', errors='replace').read()
    ops = read_ula_ops(ula, bad)
    by_name = {v: k for k, v in ops.items()}
    dec_text = open(dec_p, encoding='utf-8', errors='replace').read()
    for m in re.finditer(DEC_ROW, dec_text):
        mn, n = m.group(1), int(m.group(3))
        want = by_name.get(alu_name(mn), 0)
        if n != want:
            bad.append(f'instr_dec.v: {mn} runs ALU op {n} ({ops.get(n, "unknown")}), '
                       f'its name says {want} ({ops.get(want, "?")})')
    # every other 6'dN in ula.v names its operation on the same line (in the
    # code or in the comment), so a wrong number shows as a name that is not
    # there
    for k, line in enumerate(ula.splitlines(), 1):
        for lit in re.findall(r"6'd(\d+)", line):
            name = ops.get(int(lit))
            if name is None:
                bad.append(f'ula.v:{k}: op 6\'d{lit} is no ALU operation')
            elif not re.search(r'(?<![A-Z_])' + name + r'(?![A-Z_0-9])', line):
                bad.append(f'ula.v:{k}: op 6\'d{lit} is {name}, which the line does not name')

    # 12: the dividers' cycle counts, in the HDL and in the compilers
    ash = open(os.path.join(root, 'Compilers', 'common', 'asm_share.h'),
               encoding='utf-8', errors='replace').read()
    # (the float group is F_DIV on its own array; in a program with an integer
    # division too it shares the integer's, ula_div_sh, and takes its cycles)
    for kind, lp, mods, define in (('integer', 'IDIV_STG', ('ula_idiv', 'ula_div_sh'), 'ASM_DIV_WORDS_INT'),
                                   ('float', 'FDIV_ALONE', ('ula_fdiv',), 'ASM_DIV_WORDS_FLT')):
        got = {}
        m = re.search(r'localparam\s+' + lp + r'\s*=\s*(\d+)\s*;', ula)
        got[f'ula.v {lp}'] = m and int(m.group(1))
        for mod in mods:
            m = re.search(r'module\s+' + mod + r'\b.*?parameter\s+STG\s*=\s*(\d+)', ula, re.S)
            got[f'ula.v {mod} STG default'] = m and int(m.group(1))
        m = re.search(r'#define\s+' + define + r'\s+(\d+)', ash)
        got[f'asm_share.h {define}'] = m and int(m.group(1))
        missing = [k for k, v in got.items() if v is None]
        if missing:
            bad.append(f"the {kind} divider's cycles: not found in " + ', '.join(missing))
        elif len(set(got.values())) != 1:
            bad.append(f"the {kind} divider's cycles disagree: " +
                       ', '.join(f'{k} {v}' for k, v in got.items()))

    if bad:
        print('check_isa: the instruction set does not agree with itself', file=sys.stderr)
        for b in bad:
            print(f'  {b}', file=sys.stderr)
        return 1
    print(f'check_isa: {len(table)} mnemonics, {len(opcodes)} opcodes '
          f'(0..{opcodes[-1]}); ASMComp.l, core.v, instr_dec.v, asm_share.c, the effect columns, '
          f"the ALU operation numbers and the dividers' cycles agree")
    return 0


if __name__ == '__main__':
    sys.exit(main())
