#!/usr/bin/env python3
"""make_variants.py: build the four clock experiments of 2026-10-15 as copies of
SAPHO/ under .smoke/hw/clock_exp/ (nothing in SAPHO/ changes).

  stg4  both dividers in 4 stages (one more register cut each)
  jz    stg4 + JIZ tests an ALU output without the normaliser
  lzc   jz + the normaliser's leading-zero count as a tree (same results)
  addr  lzc + the indirect read index from that same output, and the
        integer divider in 5 stages

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


def main():
    os.makedirs(OUT, exist_ok=True)
    d = os.path.join(ROOT, 'SAPHO')
    for step in (stg4, jz, lzc, addr):
        d = step(d)
        print('built', os.path.relpath(d, ROOT))


if __name__ == '__main__':
    main()
