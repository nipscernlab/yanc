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
  nshift pnorm with a normalizing shifter (count and shift together)
  rpar  pnorm with the mantissa rounding as a carry select
  nsr   nshift + rpar
  dsp   pnorm with the normaliser's shift as a product by 2^k (DSP block)
  alm   pnorm with the leading-zero count as a radix-4 tree
  rshort pnorm with the rounding increment only for a shift of 0 or 1
  lza   pnorm with F_ADD normalised by leading-zero anticipation
  twopath pnorm with a two-path F_ADD
(the last five: docs/design/normalisation-survey.md; eq_fops.sh tests them)
  sepadd pnorm with the add as a separate circuit (own normaliser and
        rounding), a third input of the final mux
  sep2p sepadd with the near/far adder inside

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


NSHIFT_MODULE = '''// experiment: normalizing shifter -- counts the leading zeros and shifts at
// once, stage by stage: a stage of 2^k shifts left by 2^k when the top 2^k
// bits are zero (from the widest stage down). The count is the sum of the
// shifts, capped at W-1 as the chain was (only a zero mantissa reaches more,
// and zman handles it).

module ula_nshift
#(
	parameter W  = 26,
	parameter CW =  8
)(
	input  [W -1:0] in,
	output [W -1:0] out,
	output [CW-1:0] cnt
);

localparam L = $clog2(W);

genvar k;
generate
for (k = L; k >= 0; k = k-1) begin : st
	wire [W-1:0] v;
	wire [L  :0] c;
	if (k == L) begin : top
		assign v = in;
		assign c = {(L+1){1'b0}};
	end else begin : stage
		wire z = (st[k+1].v[W-1 -: (1 << k)] == {(1 << k){1'b0}});
		assign v = (z) ? (st[k+1].v << (1 << k)) : st[k+1].v;
		assign c = st[k+1].c + ((z) ? (1 << k) : 0);
	end
end
endgenerate

assign out = st[0].v;
assign cnt = (st[0].c > W-1) ? W-1 : st[0].c;

endmodule

'''

NSHIFT_USE = '''	// experiment: the normalizing shifter counts and shifts at once
	wire [EXP-1:0] sh;
	wire [W-1  :0] nrm_l;
	ula_nshift #(W, EXP) nsh (man_l, nrm_l, sh);
	wire signed [EXP+1:0] e_l   =  exp_l - {{2{1'b0}}, sh};
'''


def nshift(src):
    """pnorm with the normaliser's leading-zero count and left shift done by a
    normalizing shifter (2026-10-15)."""
    d, ula, core = fresh('nshift', src)
    nl = newline(ula)
    edit(ula, TREE.replace('\n', nl), NSHIFT_USE.replace('\n', nl))
    anchor = '// normalization of a floating-point number -----------------------------------'
    edit(ula, anchor, NSHIFT_MODULE.replace('\n', nl) + anchor)
    return d


def rpar(src, name='rpar'):
    """pnorm with the mantissa rounding as a carry select (2026-10-15): the
    increment does not wait for the round decision, it is chosen at the end,
    as the exponent already was."""
    d, ula, core = fresh(name, src)
    nl = newline(ula)
    edit(ula, '\twire        [MAN  :0] rnd   = {1\'b0, mnt} + {{MAN{1\'b0}}, inc};',
              "\twire        [MAN  :0] mp1   = {1'b0, mnt} + {{MAN{1'b0}}, 1'b1};   // experiment: computed without waiting for inc" + nl +
              "\twire        [MAN  :0] rnd   = (inc) ? mp1 : {1'b0, mnt};")
    return d


ANCHOR_NORM = '// normalization of a floating-point number -----------------------------------'


def dsp(src):
    """pnorm with the normaliser's left shift as a product by 2^sh, for a DSP
    block (survey method 6, 2026-10-15). 2^sh is the lowest set bit of the
    bit-reversed mantissa (r & -r), so no decoder from the count; the count
    still comes from the tree, for the exponent."""
    d, ula, core = fresh('dsp', src)
    nl = newline(ula)
    edit(ula, '\twire        [W-1  :0] nrm_l =  man_l << sh;',
         ('\t// experiment: the shift as a product by 2^sh (a DSP block); 2^sh is the\n'
          '\t// lowest set bit of the bit-reversed mantissa, r & -r, no decoder\n'
          '\treg  [W-1  :0] rev;\n'
          '\tinteger rb;\n'
          '\talways @ (*) for (rb = 0; rb < W; rb = rb+1) rev[rb] = man_l[W-1-rb];\n'
          "\twire [W-1  :0] p2    = rev & (~rev + {{W-1{1'b0}}, 1'b1});\n"
          '\twire [2*W-1:0] prod  = man_l * p2;\n'
          '\twire [W-1  :0] nrm_l = prod[W-1:0];').replace('\n', nl))
    return d


LZC4_MODULE = '''// experiment: leading-zero count as a radix-4 tree (survey method 3): each
// leaf takes 4 bits, each node 4 children, so fewer levels than the binary
// tree and node functions of up to 7 inputs (one ALM). Same output as
// ula_lzc: the input is padded with ones below, so a zero input counts W.

module ula_lzc4
#(
	parameter W  = 26,
	parameter CW =  8
)(
	input  [W -1:0] in,
	output [CW-1:0] out
);

localparam L = (W <= 4) ? 1 : (W <= 16) ? 2 : (W <= 64) ? 3 : 4;   // levels
localparam P = 1 << (2*L);                                          // padded width

wire [P-1:0] x;
generate if (P > W) begin : pad assign x = {in, {(P-W){1'b1}}}; end
         else       begin : pad assign x = in;                   end endgenerate

genvar k, j;
generate
for (k = 1; k <= L; k = k+1) begin : lv
	wire [(P>>(2*k))*(2*k)-1:0] cnt;
	wire [(P>>(2*k))      -1:0] zer;
	for (j = 0; j < (P>>(2*k)); j = j+1) begin : n
		wire [3:0] z;
		wire [2*k-1:0] c;
		if (k == 1) begin : leaf
			wire [3:0] g = x[4*j +: 4];
			assign z = ~g;
			assign c = (g[3]) ? 2'd0 : (g[2]) ? 2'd1 : (g[1]) ? 2'd2 : 2'd3;
		end else begin : node
			// child 3 is the most significant
			wire [2*k-3:0] c3 = lv[k-1].cnt[(4*j+3)*(2*k-2) +: (2*k-2)];
			wire [2*k-3:0] c2 = lv[k-1].cnt[(4*j+2)*(2*k-2) +: (2*k-2)];
			wire [2*k-3:0] c1 = lv[k-1].cnt[(4*j+1)*(2*k-2) +: (2*k-2)];
			wire [2*k-3:0] c0 = lv[k-1].cnt[(4*j  )*(2*k-2) +: (2*k-2)];
			assign z = lv[k-1].zer[4*j +: 4];
			assign c = (!z[3]) ? {2'd0, c3} : (!z[2]) ? {2'd1, c2} : (!z[1]) ? {2'd2, c1} : {2'd3, c0};
		end
		assign zer[j]              = &z;
		assign cnt[j*(2*k) +: 2*k] = c;
	end
end
endgenerate

assign out = lv[L].cnt[2*L-1:0];           // zero-extended (or cut: the count never exceeds W) to CW

endmodule

'''


def alm(src):
    """pnorm with the leading-zero count as a radix-4 tree (survey method 3)."""
    d, ula, core = fresh('alm', src)
    nl = newline(ula)
    edit(ula, '\tula_lzc #(W, EXP) lzc (man_l, lz);', '\tula_lzc4 #(W, EXP) lzc (man_l, lz);   // experiment: radix 4')
    edit(ula, ANCHOR_NORM, LZC4_MODULE.replace('\n', nl) + ANCHOR_NORM)
    return d


RSHORT_RNE = '''	// experiment: rounding only on the short shifts (survey method 7). After a
	// shift of 2 or more, guard and sticky are zero (deduced from the
	// operators), so the increment is computed on the unshifted input for a
	// shift of 0 and of 1, beside the count, and only picked at the end
	wire        [MAN  :0] rnd;
	if (G != 0) begin : rne
		wire [MAN-1:0] mnt0 = src_r[W-1:G];
		wire [MAN-1:0] mnt1 = src_r[W-2:G-1];
		wire           inc0 = src_r[G-1] & ((|src_r[G-2:0]) | mnt0[0]);
		wire           inc1 = src_r[G-2] & ((|{src_r[G-3:0], 1'b0}) | mnt1[0]);   // G = 3
		wire [MAN  :0] rnd0 = {1'b0, mnt0} + {{MAN{1'b0}}, inc0};
		wire [MAN  :0] rnd1 = {1'b0, mnt1} + {{MAN{1'b0}}, inc1};
		assign rnd = (k_r == 0) ? rnd0 : (k_r == 1) ? rnd1 : {1'b0, nrm[W-1:G]};
	end else begin : trunc
		assign rnd = {1'b0, nrm[W-1:G]};
	end
'''


def rshort(src):
    """pnorm with the rounding increment only for a shift of 0 or 1 (survey
    method 7): correct only if no operator hands in non-zero guard bits that a
    longer shift would keep."""
    d, ula, core = fresh('rshort', src)
    nl = newline(ula)
    edit(ula, 'wire                  zman;' + nl + nl + 'generate if (LZC != 0) begin : lzc',
              'wire                  zman;' + nl +
              'wire        [W-1  :0] src_r;                                          // experiment: the mantissa before the shift' + nl +
              'wire        [EXP-1:0] k_r;                                            // experiment: the shift' + nl + nl +
              'generate if (LZC != 0) begin : lzc')
    edit(ula, "\t\tassign zman  = (use_d) ? (in_d[W-1:0] == {W{1'b0}}) : (man_l == {W{1'b0}});",
              "\t\tassign zman  = (use_d) ? (in_d[W-1:0] == {W{1'b0}}) : (man_l == {W{1'b0}});" + nl +
              "\t\tassign src_r = (use_d) ? in_d[W-1:0] : man_l;" + nl +
              "\t\tassign k_r   = (use_d) ? {EXP{1'b0}} : sh;")
    edit(ula, "\t\tassign zman  = (man_l == {W{1'b0}});",
              "\t\tassign zman  = (man_l == {W{1'b0}});" + nl +
              "\t\tassign src_r = man_l;" + nl + "\t\tassign k_r   = sh;")
    edit(ula, "\tassign zman  = (in_d[W-1:0] == {W{1'b0}});",
              "\tassign zman  = (in_d[W-1:0] == {W{1'b0}});" + nl +
              "\tassign src_r = in_d[W-1:0];" + nl + "\tassign k_r   = {EXP{1'b0}};")
    s = open(ula, encoding='utf-8', newline='').read()
    a = s.index('\twire        [MAN-1:0] mnt = nrm[W-1:G];')
    b_txt = "\twire        [MAN  :0] rnd   = {1'b0, mnt} + {{MAN{1'b0}}, inc};" + nl
    b = s.index(b_txt) + len(b_txt)
    s = s[:a] + RSHORT_RNE.replace('\n', nl) + s[b:]
    open(ula, 'w', encoding='utf-8', newline='').write(s)
    return d


FADD_LZA_MODULE = '''// experiment: F_ADD with leading-zero anticipation (survey method 8). The
// count is predicted from the operands beside the adder, then the result is
// shifted by it and by one more bit when the prediction was one short; it
// leaves already normalised, for the normaliser's direct path.
//   subtraction a - b, a >= b: digit d_i = a_i - b_i. The leading one is at
//   the end of the run of -1 digits after the first non-zero digit, or one
//   below: f_i = (a_i ^ b_i) & ~(d_(i-1) == -1), its leading one predicts it.
//   addition: the leading one of a | b, or one above (the carry).

module ula_fadd_lza
#(
	parameter MAN    = 23,
	parameter EXP    =  8,
	parameter FROUND =  0,
	parameter G      =  0
)(
	input  signed [EXP-1     :0] e_in,
	input                        s_big, s_small,
	input         [MAN+G-1   :0] m_big, m_small,
	output        [MAN+EXP+G+2:0] out
);

localparam W = MAN+G;

wire [MAN+EXP+G+2:0] raw;
ula_fadd #(MAN,EXP,FROUND,G) add (e_in, s_big, s_small, m_big, m_small, raw);
wire                  s_r = raw[MAN+EXP+G+2  ];
wire signed [EXP+1:0] e_r = raw[MAN+EXP+G+1:W];
wire        [W-1  :0] m_r = raw[W        -1:0];

wire       same = (s_big == s_small);
wire [W:0] dbs  = {1'b0, m_big} - {1'b0, m_small};
wire       bneg = dbs[W];                                     // m_big < m_small (the adder has the same)

wire [W-1:0] f_bs = (m_big ^ m_small) & ~{(~m_big[W-2:0] &  m_small[W-2:0]), 1'b0};
wire [W-1:0] f_sb = (m_big ^ m_small) & ~{( m_big[W-2:0] & ~m_small[W-2:0]), 1'b0};

wire [EXP-1:0] lz_bs, lz_sb, lz_or;
ula_lzc #(W, EXP) l1 (f_bs, lz_bs);
ula_lzc #(W, EXP) l2 (f_sb, lz_sb);
ula_lzc #(W, EXP) l3 (m_big | m_small, lz_or);

wire [EXP-1:0] lz_f = (bneg) ? lz_sb : lz_bs;
wire [EXP-1:0] pred;
generate if (FROUND == 0) begin : legacy
	// the adder always drops one bit (m[W:1]): one more leading zero
	assign pred = (same) ? lz_or : lz_f + {{EXP-1{1'b0}}, 1'b1};
end else begin : keep
	assign pred = (same) ? ((lz_or == 0) ? {EXP{1'b0}} : lz_or - {{EXP-1{1'b0}}, 1'b1}) : lz_f;
end endgenerate

wire        [W-1  :0] s1  = m_r << pred;
wire                  c   = ~s1[W-1];                         // the prediction was one short
wire        [W-1  :0] nrm = (c) ? (s1 << 1) : s1;
wire        [EXP-1:0] cnt = pred + {{EXP-1{1'b0}}, c};
wire signed [EXP+1:0] e_o = e_r - {2'b00, cnt};

assign out = {s_r, e_o, nrm};

endmodule

'''

FADD_2P_MODULE = '''// experiment: two-path F_ADD (survey method 9). Near path: an effective
// subtraction with exponents at most 1 apart -- alignment by 0 or 1 bit, then
// the full leading-zero count and shift. Far path: everything else -- the full
// alignment (ula_denorm, outside), then at most 2 bits of normalising (2 only
// at FROUND 0, where the adder drops a bit). Assumes normalised operands. The
// result leaves normalised, for the normaliser's direct path.

module ula_fadd2p
#(
	parameter MAN    = 23,
	parameter EXP    =  8,
	parameter FROUND =  0,
	parameter G      =  0
)(
	input                        neg1, neg2,
	input         [MAN+EXP   :0] in1,  in2,
	input  signed [EXP-1     :0] e_far,                       // ula_denorm's outputs (far path)
	input                        s_big_f, s_small_f,
	input         [MAN+G-1   :0] m_big_f, m_small_f,
	output        [MAN+EXP+G+2:0] out
);

localparam W = MAN+G;

// far path
wire [MAN+EXP+G+2:0] far;
ula_fadd #(MAN,EXP,FROUND,G) add_f (e_far, s_big_f, s_small_f, m_big_f, m_small_f, far);
wire        [W-1  :0] m_f  = far[W-1:0];
wire        [1    :0] sh_f = (m_f[W-1]) ? 2'd0 : (m_f[W-2]) ? 2'd1 : 2'd2;
wire        [W-1  :0] n_f  = m_f << sh_f;
wire signed [EXP+1:0] e_f  = $signed(far[MAN+EXP+G+1:W]) - {{EXP{1'b0}}, sh_f};

// near path: its own alignment by 0 or 1
wire                  s1_in = (neg1) ? ~in1[MAN+EXP] : in1[MAN+EXP];
wire                  s2_in = (neg2) ? ~in2[MAN+EXP] : in2[MAN+EXP];
wire signed [EXP-1:0] e1_in = in1[MAN+EXP-1:MAN];
wire signed [EXP-1:0] e2_in = in2[MAN+EXP-1:MAN];
wire signed [EXP  :0] d12   = e1_in - e2_in;
wire signed [EXP  :0] d21   = e2_in - e1_in;
wire                  swap  = d12[EXP];
wire        [EXP  :0] shift = (swap) ? d21 : d12;
wire                  sb_n  = (swap) ? s2_in : s1_in;
wire                  ss_n  = (swap) ? s1_in : s2_in;
wire                  near  = (shift <= 1) & (sb_n != ss_n);
wire        [MAN-1:0] mb    = (swap) ? in2[MAN-1:0] : in1[MAN-1:0];
wire        [MAN-1:0] ms    = (swap) ? in1[MAN-1:0] : in2[MAN-1:0];
wire        [W-1  :0] mb_x, ms_x;
generate if (G != 0) begin : ext
	assign mb_x = {mb, {G{1'b0}}};
	assign ms_x = {ms, {G{1'b0}}};
end else begin : noext
	assign mb_x = mb;
	assign ms_x = ms;
end endgenerate
wire        [W-1  :0] ms_a  = (shift[0]) ? (ms_x >> 1) : ms_x;   // G = 0: the bit is lost, as in ula_denorm
wire signed [EXP-1:0] e_nr  = (swap) ? e2_in : e1_in;

wire [MAN+EXP+G+2:0] nr;
ula_fadd #(MAN,EXP,FROUND,G) add_n (e_nr, sb_n, ss_n, mb_x, ms_a, nr);
wire        [W-1  :0] m_n  = nr[W-1:0];
wire        [EXP-1:0] lz;
ula_lzc #(W, EXP) lzn (m_n, lz);
wire        [EXP-1:0] sh_n = (lz > W-1) ? W-1 : lz;
wire        [W-1  :0] n_n  = m_n << sh_n;
wire signed [EXP+1:0] e_n  = $signed(nr[MAN+EXP+G+1:W]) - {2'b00, sh_n};

assign out = (near) ? {nr[MAN+EXP+G+2], e_n, n_n} : {far[MAN+EXP+G+2], e_f, n_f};

endmodule

'''


def fadd_direct(ula, nl):
    """F_ADD/F_SU1/F_SU2 arrive normalised: route them to the normaliser's
    direct path and take them out of the leading-zero path."""
    edit(ula, "wire               use_d = (FROUND != 0) & ((op == 6'd5) | (op == 6'd7));",
              "wire               use_d = ((FROUND != 0) & ((op == 6'd5) | (op == 6'd7))) | (op == 6'd3) | (op == 6'd47) | (op == 6'd48);   // experiment: F_ADD arrives normalised")
    for code, p in (("6'd3  ", 'P_FADD'), ("6'd47 ", 'P_FSU1'), ("6'd48 ", 'P_FSU2')):
        edit(ula, f"\t{code} : if ({p}) imux_l =  fadd ;", f"\t{code} : if ({p}) imux_d =  fadd ;   // experiment: direct path")
    s = open(ula, encoding='utf-8', newline='').read()
    # the three rows now write imux_d: move them into the imux_d case
    rows = [l for l in s.split(nl) if 'imux_d =  fadd ;   // experiment' in l]
    for l in rows:
        s = s.replace(l + nl, '', 1)
    anchor = "\t6'd5   : if (P_FMLT) imux_d =  fmlt ;"
    s = s.replace(anchor, nl.join(rows) + nl + anchor, 1)
    open(ula, 'w', encoding='utf-8', newline='').write(s)
    edit(ula, '((I2F | I2F_M | F_ADD | F_SU1 | F_SU2 | F_ROT) != 0) || (FROUND == 0 && (F_MLT | F_DIV) != 0),  // LZC',
              '((I2F | I2F_M | F_ROT) != 0) || (FROUND == 0 && (F_MLT | F_DIV) != 0),  // LZC (experiment: F_ADD not)')
    edit(ula, '(FROUND != 0) && ((F_MLT | F_DIV) != 0),                                                             // DIR',
              '((FROUND != 0) && ((F_MLT | F_DIV) != 0)) || ((F_ADD | F_SU1 | F_SU2) != 0),                         // DIR (experiment: + F_ADD)')


def lza(src):
    """pnorm with F_ADD normalised by leading-zero anticipation (survey method 8)."""
    d, ula, core = fresh('lza', src)
    nl = newline(ula)
    edit(ula, 'ula_fadd #(NBMANT,NBEXPO,FROUND,G) my_fadd(', 'ula_fadd_lza #(NBMANT,NBEXPO,FROUND,G) my_fadd(')
    edit(ula, ANCHOR_NORM, FADD_LZA_MODULE.replace('\n', nl) + ANCHOR_NORM)
    fadd_direct(ula, nl)
    return d


def twopath(src):
    """pnorm with a two-path F_ADD (survey method 9)."""
    d, ula, core = fresh('twopath', src)
    nl = newline(ula)
    edit(ula, 'ula_fadd #(NBMANT,NBEXPO,FROUND,G) my_fadd(e_out,',
              'ula_fadd2p #(NBMANT,NBEXPO,FROUND,G) my_fadd(su1, su2, in1, in2, e_out,')
    edit(ula, ANCHOR_NORM, FADD_2P_MODULE.replace('\n', nl) + ANCHOR_NORM)
    fadd_direct(ula, nl)
    return d


def sepadd(src, name='sepadd', two_path=False):
    """pnorm with F_ADD/F_SU1/F_SU2 as a separate circuit (Luciano,
    2026-10-16): its own normaliser and rounding (a private ula_norm), its
    result a third input of the final mux, out of the shared normaliser's
    input mux and never into the fast mux (JIZ, indirect index). two_path:
    the near/far adder of twopath inside, then only the rounding."""
    d, ula, core = fresh(name, src)
    nl = newline(ula)
    if two_path:
        edit(ula, 'ula_fadd #(NBMANT,NBEXPO,FROUND,G) my_fadd(e_out,',
                  'ula_fadd2p #(NBMANT,NBEXPO,FROUND,G) my_fadd(su1, su2, in1, in2, e_out,')
        edit(ula, ANCHOR_NORM, FADD_2P_MODULE.replace('\n', nl) + ANCHOR_NORM)
        # aligned already: the private normaliser only rounds and checks the range
        norm = "ula_norm #(NBMANT,NBEXPO,FROUND,G,0,1) fadd_norm ({NUBITS+G+2{1'b0}}, fadd, 1'b1, fadd_n);"
    else:
        norm = "ula_norm #(NBMANT,NBEXPO,FROUND,G,1,0) fadd_norm (fadd, {NUBITS+G+2{1'b0}}, 1'b0, fadd_n);"
    s = open(ula, encoding='utf-8', newline='').read()
    i = s.index('begin : op_fadd ')
    j = s.index(nl, i) + len(nl)
    s = (s[:j] + nl +
         '// experiment: the add normalised and rounded by its own circuit, out of the shared normaliser' + nl +
         'wire [NUBITS-1:0] fadd_n;' + nl +
         'generate if ((F_ADD | F_SU1 | F_SU2) != 0) begin : op_faddn ' + norm +
         " end else begin : op_faddn assign fadd_n = {NUBITS{1'b0}}; end endgenerate" + nl + s[j:])
    open(ula, 'w', encoding='utf-8', newline='').write(s)
    edit(ula, 'generate if ((I2F | I2F_M | F_ADD | F_SU1 | F_SU2 | F_MLT | F_DIV | F_ROT) != 0) begin : op_smx',
              'generate if ((I2F | I2F_M | F_MLT | F_DIV | F_ROT) != 0) begin : op_smx')
    edit(ula, '((I2F | I2F_M | F_ADD | F_SU1 | F_SU2 | F_ROT) != 0) || (FROUND == 0 && (F_MLT | F_DIV) != 0),  // LZC',
              '((I2F | I2F_M | F_ROT) != 0) || (FROUND == 0 && (F_MLT | F_DIV) != 0),  // LZC (experiment: the add has its own)')
    edit(ula, '(F_ADD != 0), (F_SU1 != 0), (F_SU2 != 0), (F_MLT != 0)', "0, 0, 0, (F_MLT != 0)")
    edit(ula, "\twire is_norm = (((F_ADD) != 0) & (op == 6'd3)) | (((F_MLT) != 0) & (op == 6'd5)) | (((F_DIV) != 0) & (op == 6'd7)) | (((I2F) != 0) & (op == 6'd25)) | (((I2F_M) != 0) & (op == 6'd26)) | (((F_ROT) != 0) & (op == 6'd46)) | (((F_SU1) != 0) & (op == 6'd47)) | (((F_SU2) != 0) & (op == 6'd48));" + nl +
              "\tassign out = (is_norm) ? smx : out_jz;",
              "\twire is_add  = (((F_ADD) != 0) & (op == 6'd3)) | (((F_SU1) != 0) & (op == 6'd47)) | (((F_SU2) != 0) & (op == 6'd48));" + nl +
              "\twire is_norm = (((F_MLT) != 0) & (op == 6'd5)) | (((F_DIV) != 0) & (op == 6'd7)) | (((I2F) != 0) & (op == 6'd25)) | (((I2F_M) != 0) & (op == 6'd26)) | (((F_ROT) != 0) & (op == 6'd46));" + nl +
              "\tassign out = (is_add) ? fadd_n : (is_norm) ? smx : out_jz;   // experiment: the add's own circuit, a third input")
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
        variants[step.__name__] = step(variants['param'])
        print('built', os.path.relpath(variants[step.__name__], ROOT))
    # the normaliser alternatives, each from pnorm (2026-10-15)
    variants['nshift'] = nshift(variants['pnorm'])
    variants['rpar'] = rpar(variants['pnorm'])
    variants['nsr'] = rpar(variants['nshift'], 'nsr')
    # the methods of docs/design/normalisation-survey.md, each from pnorm
    for step in (dsp, alm, rshort, lza, twopath, sepadd):
        variants[step.__name__] = step(variants['pnorm'])
    variants['sep2p'] = sepadd(variants['pnorm'], 'sep2p', two_path=True)
    for v in ('nshift', 'rpar', 'nsr', 'dsp', 'alm', 'rshort', 'lza', 'twopath', 'sepadd', 'sep2p'):
        print('built', os.path.relpath(variants[v], ROOT))


if __name__ == '__main__':
    main()
