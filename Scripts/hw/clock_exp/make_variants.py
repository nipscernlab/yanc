#!/usr/bin/env python3
"""make_variants.py: build the clock experiments of 2026-10-15 as copies of
SAPHO/ under .smoke/hw/clock_exp/ (nothing in SAPHO/ changes).

  stg4  both dividers in 4 stages (one more register cut each)
  jz    stg4 + JIZ tests an ALU output without the normaliser
  lzc   jz + the normaliser's leading-zero count as a tree (same results)
  addr  lzc + the indirect read index from that same output, and the
        integer divider in 5 stages
  split addr with the ALU output split, not copied: one mux of the non-normalised
        operations and a 2-input mux after it for the normaliser
  param split with both muxes generated from the opcode parameters (OR of
        one term per operation; the 2-input mux only with a normalised op)
  pcase param with the fast mux as a case with one guarded row per
        operation, default 0 (the instruction decoder's style)
  pnorm param with the normaliser's input mux (norm_mux) guarded the same way

Each later variant contains the earlier ones. Time one with
  HDL=$PWD/.smoke/hw/clock_exp/<name> TAG=<tag> bash Scripts/hw/fmax.sh sapho_all 5CSEBA6U23I7
The results and what each change would still need before it is real are in
docs/design/pipeln-and-division.md section 4.3.
"""
import os
import re
import shutil
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..'))
OUT = os.path.join(ROOT, '.smoke', 'hw', 'clock_exp')


def newline(path):
    return '\r\n' if '\r\n' in open(path, encoding='utf-8', newline='').read() else '\n'


def edit(path, old, new, count=1):
    s = open(path, encoding='utf-8', newline='').read()
    n = s.count(old)
    if n != count:
        sys.exit(f'{path}: expected {count} of {old[:70]!r}, found {n}')
    open(path, 'w', encoding='utf-8', newline='').write(s.replace(old, new))


def fresh(name, src):
    dst = os.path.join(OUT, name)
    shutil.rmtree(dst, ignore_errors=True)
    shutil.copytree(src, dst)
    return dst, os.path.join(dst, 'ula.v'), os.path.join(dst, 'core.v')


def stg4(src):
    d, ula, core = fresh('stg4', src)
    edit(ula, 'div_array #(NUBITS,NUBITS,2,3,PRE10,POST10)', 'div_array #(NUBITS,NUBITS,2,4,PRE10,POST10)')
    edit(ula, 'div_array #(MAN,R,EXP+4,3,PRE10,POST10)', 'div_array #(MAN,R,EXP+4,4,PRE10,POST10)')
    return d


def jz(src):
    d, ula, core = fresh('jz', src)
    nl = newline(ula)
    edit(ula, '\toutput signed [NUBITS-1:0] out,' + nl + '\tinput                      clk',
              '\toutput signed [NUBITS-1:0] out,' + nl +
              '\toutput                     jz_nz,                 // experiment: "out is not zero" for JIZ, the normaliser left out' + nl +
              '\tinput                      clk')
    s = open(ula, encoding='utf-8', newline='').read()
    m = re.search(r'ula_mux #\(NUBITS\) ula_mux \(.*?\.out\(out\)\);', s, re.S)
    if not m:
        sys.exit('main mux instance not found')
    inst = m.group(0)
    twin = (inst.replace('ula_mux #(NUBITS) ula_mux (', 'ula_mux #(NUBITS) ula_mux_jz (')
                .replace('.smx(smx)', ".smx({NUBITS{1'b0}})").replace('.out(out)', '.out(out_jz)'))
    s = s.replace(inst, inst + nl + nl +
                  '// experiment: the same mux without the normaliser, for JIZ only (the' + nl +
                  '// compilers never put JIZ right after an operation that goes through it)' + nl +
                  'wire [NUBITS-1:0] out_jz;' + nl + twin + nl + 'assign jz_nz = |out_jz;', 1)
    open(ula, 'w', encoding='utf-8', newline='').write(s)
    edit(core, 'ula (id_ula_op, ula_data_in1, ula_data_in2, ula_out, clk);',
               'ula (id_ula_op, ula_data_in1, ula_data_in2, ula_out, ula_jz_nz, clk);')
    edit(core, 'wire signed [NUBITS-1:0] ula_out;',
               'wire signed [NUBITS-1:0] ula_out;' + nl + 'wire                     ula_jz_nz;   // experiment')
    edit(core, 'assign  if_acc = |ula_out;', 'assign  if_acc = ula_jz_nz;   // experiment: the normaliser out of the jump')
    return d


LZC_MODULE = '''// experiment: leading-zero count of a W-bit word, by a tree ----------------
// Pads W to a power of two with ones below (they never add zeros above a
// one), then merges pairs: a pair's count is the left count when the left
// half has a one, else the left half's width plus the right count.

module ula_lzc
#(
	parameter W  = 26,
	parameter CW =  8                      // width of the count
)(
	input  [W -1:0] in,
	output [CW-1:0] out
);

localparam L = $clog2(W);                  // levels
localparam P = 1 << L;                     // padded width

wire [P-1:0] x;
generate if (P > W) begin : pad assign x = {in, {(P-W){1'b1}}}; end
         else       begin : pad assign x = in;                   end endgenerate

// level k holds P>>k nodes, each a count of L+1 bits plus "all zero"
genvar k, j;
generate
for (k = 0; k <= L; k = k+1) begin : lv
	wire [(P>>k)*(L+1)-1:0] cnt;
	wire [(P>>k)      -1:0] zer;
	if (k == 0) begin : leaf
		for (j = 0; j < P; j = j+1) begin : b
			assign zer[j]                = ~x[j];
			assign cnt[j*(L+1) +: (L+1)] = {{L{1'b0}}, ~x[j]};
		end
	end else begin : node
		for (j = 0; j < (P>>k); j = j+1) begin : n
			// left = the more significant child (index 2j+1)
			wire           zl = lv[k-1].zer[2*j+1];
			wire           zr = lv[k-1].zer[2*j  ];
			wire [L:0]     cl = lv[k-1].cnt[(2*j+1)*(L+1) +: (L+1)];
			wire [L:0]     cr = lv[k-1].cnt[(2*j  )*(L+1) +: (L+1)];
			assign zer[j]                = zl & zr;
			assign cnt[j*(L+1) +: (L+1)] = (zl) ? ((1 << (k-1)) + cr) : cl;
		end
	end
end
endgenerate

wire [L:0] total = lv[L].cnt[L:0];
assign out = total;                        // zero-extended to CW

endmodule

'''

CHAIN = '''	wire [EXP-1:0] w [W-1:0];

	wire        [EXP-1:0] sh    =  w[W-2];
	wire        [W-1  :0] nrm_l =  man_l << sh;
	wire signed [EXP+1:0] e_l   =  exp_l - {{2{1'b0}}, sh};

	ula_nmux #(1, EXP) mm1 (man_l[W-1], 1'b0, {{EXP-1{1'b0}}, {1'b1}}, {EXP{1'b0}}, w[0]);

	genvar i;
	for (i = 1; i < W-1; i = i+1) begin : norm
		ula_nmux #(i+1, EXP) mm (man_l[W-1:W-1-i], {i+1{1'b0}}, i[EXP-1:0] + {{EXP-1{1'b0}}, {1'b1}}, w[i-1], w[i]);
	end
'''

TREE = '''	// experiment: the leading zeros counted by a tree (log2 W levels) instead of
	// a chain of W-1 muxes; capped at W-1 as the chain was (a zero mantissa is
	// handled by zman either way)
	wire [EXP-1:0] lz;
	ula_lzc #(W, EXP) lzc (man_l, lz);

	wire        [EXP-1:0] sh    =  (lz > W-1) ? W-1 : lz;
	wire        [W-1  :0] nrm_l =  man_l << sh;
	wire signed [EXP+1:0] e_l   =  exp_l - {{2{1'b0}}, sh};
'''


def lzc(src):
    d, ula, core = fresh('lzc', src)
    nl = newline(ula)
    edit(ula, CHAIN.replace('\n', nl), TREE.replace('\n', nl))
    anchor = '// normalization of a floating-point number -----------------------------------'
    edit(ula, anchor, LZC_MODULE.replace('\n', nl) + anchor)
    return d


def addr(src):
    d, ula, core = fresh('addr', src)
    nl = newline(ula)
    edit(ula, '\toutput                     jz_nz,',
              '\toutput                     jz_nz,' + nl +
              '\toutput signed [NUBITS-1:0] out_fast,              // experiment: out without the normaliser (JIZ, indirect address)')
    edit(ula, 'assign jz_nz = |out_jz;', 'assign jz_nz = |out_jz;' + nl + 'assign out_fast = out_jz;')
    edit(ula, 'div_array #(NUBITS,NUBITS,2,4,PRE10,POST10)', 'div_array #(NUBITS,NUBITS,2,5,PRE10,POST10)')
    edit(core, 'ula (id_ula_op, ula_data_in1, ula_data_in2, ula_out, ula_jz_nz, clk);',
               'ula (id_ula_op, ula_data_in1, ula_data_in2, ula_out, ula_jz_nz, ula_fast, clk);')
    edit(core, 'wire                     ula_jz_nz;   // experiment',
               'wire                     ula_jz_nz;   // experiment' + nl +
               'wire signed [NUBITS-1:0] ula_fast;    // experiment: the ALU output without the normaliser')
    # the write data stays the full ALU output; only the indirect READ index changes
    edit(core, '\tinput  [NUBITS-1:0] ula,',
               '\tinput  [NUBITS-1:0] ula,' + nl + '\tinput  [NUBITS-1:0] ula_idx,           // experiment: the read index, without the normaliser')
    edit(core, 'ra_rd(ldi, fft, ula[MDATAW-1:0], base_addr, mem_addr_rd);',
               'ra_rd(ldi, fft, ula_idx[MDATAW-1:0], base_addr, mem_addr_rd);')
    old = ('\t\t                                   ula_out,' + nl +
           '\t\t                                   if_operand[MDATAW-1:0], sp_data[MDATAW-1:0],')
    edit(core, old, old.replace('ula_out,', 'ula_out, ula_fast,   // experiment: the index never comes from a normalised result'))
    return d


def split(src):
    """addr with the ALU output split instead of copied (Luciano, 2026-10-15):
    one mux of the operations that do not go through the normaliser (out_fast,
    for JIZ and the indirect index), and a 2-input mux after it choosing the
    normaliser's output for the operations that need it (out, for the
    accumulator). No second copy of the big mux."""
    d, ula, core = fresh('split', src)
    nl = newline(ula)
    s = open(ula, encoding='utf-8', newline='').read()
    m = re.search(r'ula_mux #\(NUBITS\) ula_mux \(.*?\.out\(out\)\);', s, re.S)
    if not m:
        sys.exit('main mux instance not found')
    s = s.replace(m.group(0), '// experiment: the full-width mux is gone, see out_fast below', 1)
    # the copy becomes the only big mux; the normalised operations are don't-care there
    s = s.replace('.smx({NUBITS{1\'b0}}),', ".smx({NUBITS{1'bx}}),   // experiment: never read (the normalised operations take smx below)")
    old = 'assign jz_nz = |out_jz;' + nl + 'assign out_fast = out_jz;'
    if s.count(old) != 1:
        sys.exit('fast output not found')
    new = (old + nl +
           '// experiment: the operations that go through the normaliser (the norm_mux list)' + nl +
           "wire is_norm = (op == 6'd3) | (op == 6'd5) | (op == 6'd7) | (op == 6'd25) | (op == 6'd26) |" + nl +
           "               (op == 6'd46) | (op == 6'd47) | (op == 6'd48);" + nl +
           'assign out = (is_norm) ? smx : out_jz;')
    s = s.replace(old, new)
    open(ula, 'w', encoding='utf-8', newline='').write(s)
    return d


# (code, input, parameter) of every ula_mux arm; NOP and LOD always exist
ARMS = [(0, 'in2', None), (1, 'in1', None), (2, 'add', 'ADD'), (3, 'smx', 'F_ADD'),
        (4, 'mlt', 'MLT'), (5, 'smx', 'F_MLT'), (6, 'div', 'DIV'), (7, 'smx', 'F_DIV'),
        (8, 'mod', 'MOD'), (9, 'sgn', 'SGN'), (10, 'fsgn', 'F_SGN'), (11, 'neg', 'NEG'),
        (12, 'negm', 'NEG_M'), (13, 'fneg', 'F_NEG'), (14, 'fnegm', 'F_NEG_M'),
        (15, 'abs', 'ABS'), (16, 'absm', 'ABS_M'), (17, 'fabs', 'F_ABS'), (18, 'fabsm', 'F_ABS_M'),
        (19, 'pst', 'PST'), (20, 'pstm', 'PST_M'), (21, 'fpst', 'F_PST'), (22, 'fpstm', 'F_PST_M'),
        (23, 'nrm', 'NRM'), (24, 'nrmm', 'NRM_M'), (25, 'smx', 'I2F'), (26, 'smx', 'I2F_M'),
        (27, 'f2i', 'F2I'), (28, 'f2im', 'F2I_M'), (29, 'ann', 'AND'), (30, 'orr', 'ORR'),
        (31, 'cor', 'XOR'), (32, 'inv', 'INV'), (33, 'invm', 'INV_M'), (34, 'lan', 'LAN'),
        (35, 'lor', 'LOR'), (36, 'lin', 'LIN'), (37, 'linm', 'LIN_M'), (38, 'les', 'LES'),
        (39, 'fles', 'F_LES'), (40, 'gre', 'GRE'), (41, 'fgre', 'F_GRE'), (42, 'equ', 'EQU'),
        (43, 'shl', 'SHL'), (44, 'shr', 'SHR'), (45, 'srs', 'SRS'), (46, 'smx', 'F_ROT'),
        (47, 'smx', 'F_SU1'), (48, 'smx', 'F_SU2'), (49, 'fscl', 'F_SCL'), (50, 'xpo', 'XPO'),
        (51, 'xpom', 'XPO_M')]


def param(src):
    """split with both muxes generated from the opcode parameters (Luciano,
    2026-10-15): the fast mux has one term per non-normalised operation the
    program has, OR-ed (no x arms at all); the 2-input mux and is_norm exist
    only when the program has a normalised operation."""
    d, ula, core = fresh('param', src)
    nl = newline(ula)
    s = open(ula, encoding='utf-8', newline='').read()
    m = re.search(r'ula_mux #\(NUBITS\) ula_mux_jz \(.*?\.out\(out_jz\)\);', s, re.S)
    if not m:
        sys.exit('fast mux instance not found')
    terms, names = [], []
    for code, inp, par in ARMS:
        if inp == 'smx':
            continue
        t = f't{code}'
        names.append(t)
        sel = f"(op == 6'd{code}) ? {inp} : {{NUBITS{{1'b0}}}}"
        if par is None:
            terms.append(f'wire [NUBITS-1:0] {t} = {sel};')
        else:
            terms.append(f'wire [NUBITS-1:0] {t}; generate if (({par}) != 0) begin : g{code} assign {t} = {sel}; end '
                         f"else begin : g{code} assign {t} = {{NUBITS{{1'b0}}}}; end endgenerate")
    fast = (nl.join(['// experiment: the fast mux, one term per non-normalised operation the',
                     '// program has (a term that is not generated is a constant 0)'] + terms) + nl +
            'wire [NUBITS-1:0] out_jz = ' + ' | '.join(names) + ';')
    s = s.replace(m.group(0), fast, 1)
    s = s.replace('wire [NUBITS-1:0] out_jz;' + nl, '', 1)
    norm_codes = [(c, p) for c, i, p in ARMS if i == 'smx']
    old = ("wire is_norm = (op == 6'd3) | (op == 6'd5) | (op == 6'd7) | (op == 6'd25) | (op == 6'd26) |" + nl +
           "               (op == 6'd46) | (op == 6'd47) | (op == 6'd48);" + nl +
           'assign out = (is_norm) ? smx : out_jz;')
    if s.count(old) != 1:
        sys.exit('2-input mux not found')
    any_norm = ' | '.join(p for c, p in norm_codes)
    is_norm = ' | '.join(f"((({p}) != 0) & (op == 6'd{c}))" for c, p in norm_codes)
    new = (f'generate if (({any_norm}) != 0) begin : norm_out' + nl +
           f'\twire is_norm = {is_norm};' + nl +
           '\tassign out = (is_norm) ? smx : out_jz;' + nl +
           'end else begin : norm_out' + nl +
           '\tassign out = out_jz;                  // no normalised operation: no 2-input mux' + nl +
           'end endgenerate')
    s = s.replace(old, new)
    open(ula, 'w', encoding='utf-8', newline='').write(s)
    return d


def pcase(src):
    """param with the fast mux in the instruction decoder's style (Luciano,
    2026-10-15): a function with one case row per non-normalised operation,
    each row guarded by its opcode parameter, default 0, driven by a
    continuous assignment (as instr_dec.v: an always @ (*) would stay X in
    Icarus until an input changes)."""
    d, ula, core = fresh('pcase', src)
    nl = newline(ula)
    s = open(ula, encoding='utf-8', newline='').read()
    a = s.index('// experiment: the fast mux, one term per non-normalised operation the')
    b = s.index('wire [NUBITS-1:0] out_jz = ')
    b = s.index(';', b) + 1
    rows = [(c, i, p) for c, i, p in ARMS if i != 'smx']
    ins = []
    for c, i, p in rows:
        if i not in ins:
            ins.append(i)
    args = ', '.join(f'a_{i}' for i in ins)
    lines = ['// experiment: the fast mux in the instruction decoder\'s style: one row per',
             '// non-normalised operation, guarded by its opcode parameter, default 0',
             f'function [NUBITS-1:0] fast_mux(input [5:0] o, input [NUBITS-1:0] {args});',
             'begin',
             "\tfast_mux = {NUBITS{1'b0}};",
             '\tcase (o)']
    for c, i, p in rows:
        guard = '' if p is None else f'if ({p}) '
        lines.append(f"\t6'd{c:<2}: {guard}fast_mux = a_{i};")
    lines += ['\tdefault: ;', '\tendcase', 'end', 'endfunction',
              'wire [NUBITS-1:0] out_jz = fast_mux(op, ' + ', '.join(ins) + ');']
    s = s[:a] + nl.join(lines) + s[b:]
    open(ula, 'w', encoding='utf-8', newline='').write(s)
    return d


def pnorm(src):
    """param with the normaliser's input mux (norm_mux) generated from the
    opcode parameters too (2026-10-15): each row guarded by its operation's
    parameter, default 0 instead of x. Built from param, not pcase."""
    d, ula, core = fresh('pnorm', src)
    nl = newline(ula)
    edit(ula, '\tparameter DIR    =  0' + nl + ')(' + nl + '\t input [         5:0] op  ,' + nl +
              '\t input [NUBITS+G+1:0] fadd,',
              '\tparameter DIR    =  0,' + nl +
              '\t// experiment: which operations the program has (default: all, as before)' + nl +
              '\tparameter P_FADD = 1, P_FSU1 = 1, P_FSU2 = 1, P_FMLT = 1, P_FDIV = 1,' + nl +
              '\tparameter P_I2F  = 1, P_I2FM = 1, P_FROT = 1' + nl +
              ')(' + nl + '\t input [         5:0] op  ,' + nl + '\t input [NUBITS+G+1:0] fadd,')
    rows = [("6'd3   : imux_l =  fadd ;", 'P_FADD'), ("6'd5   : imux_l =  fmlt ;", 'P_FMLT'),
            ("6'd7   : imux_l =  fdiv ;", 'P_FDIV'), ("6'd25  : imux_l =   i2f ;", 'P_I2F'),
            ("6'd26  : imux_l =   i2fm;", 'P_I2FM'), ("6'd46  : imux_l =  frot ;", 'P_FROT'),
            ("6'd47  : imux_l =  fadd ;", 'P_FSU1'), ("6'd48  : imux_l =  fadd ;", 'P_FSU2'),
            ("6'd5   : imux_d =  fmlt ;", 'P_FMLT'), ("6'd7   : imux_d =  fdiv ;", 'P_FDIV')]
    for row, p in rows:
        code, rest = row.split(':', 1)
        edit(ula, row, f'{code}: if ({p}) {rest.strip()}')
    edit(ula, "\tdefault: imux_l = {NUBITS+G+2{1'bx}};", "\tdefault: imux_l = {NUBITS+G+2{1'b0}};")
    edit(ula, "\tdefault: imux_d = {NUBITS+G+2{1'bx}};", "\tdefault: imux_d = {NUBITS+G+2{1'b0}};")
    # the case rows above leave imux_l unassigned when a guard is off: give it
    # a value first, so it never becomes a latch
    edit(ula, 'always @ (*) case (op)' + nl + "\t6'd3   : if (P_FADD)",
              "always @ (*) begin imux_l = {NUBITS+G+2{1'b0}}; case (op)" + nl + "\t6'd3   : if (P_FADD)")
    edit(ula, "\tdefault: imux_l = {NUBITS+G+2{1'b0}};" + nl + 'endcase',
              "\tdefault: imux_l = {NUBITS+G+2{1'b0}};" + nl + 'endcase end')
    edit(ula, 'always @ (*) case (op)' + nl + "\t6'd5   : if (P_FMLT) imux_d",
              "always @ (*) begin imux_d = {NUBITS+G+2{1'b0}}; case (op)" + nl + "\t6'd5   : if (P_FMLT) imux_d")
    edit(ula, "\tdefault: imux_d = {NUBITS+G+2{1'b0}};" + nl + 'endcase',
              "\tdefault: imux_d = {NUBITS+G+2{1'b0}};" + nl + 'endcase end')
    edit(ula, '(FROUND != 0) && ((F_MLT | F_DIV) != 0))                                                            // DIR: someone takes the direct path',
              '(FROUND != 0) && ((F_MLT | F_DIV) != 0),                                                             // DIR: someone takes the direct path' + nl +
              '\t(F_ADD != 0), (F_SU1 != 0), (F_SU2 != 0), (F_MLT != 0), (F_DIV != 0), (I2F != 0), (I2F_M != 0), (F_ROT != 0))   // experiment')
    return d


def main():
    os.makedirs(OUT, exist_ok=True)
    d = os.path.join(ROOT, 'SAPHO')
    variants = {}
    for step in (stg4, jz, lzc, addr, split, param):
        d = step(d)
        variants[step.__name__] = d
        print('built', os.path.relpath(d, ROOT))
    for step in (pcase, pnorm):                  # both start from param
        print('built', os.path.relpath(step(variants['param']), ROOT))


if __name__ == '__main__':
    main()
