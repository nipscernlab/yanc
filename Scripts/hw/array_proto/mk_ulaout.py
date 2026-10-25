"""A copy of the prototype's SAPHO/ with idea 2 back (the rounded DOT result in
ula_out's place, 65f7b96), for the seed sweep. Usage: mk_ulaout.py <src SAPHO> <dst dir>"""
import sys, shutil, os
src, dst = sys.argv[1], sys.argv[2]
shutil.rmtree(dst, ignore_errors=True)
shutil.copytree(src, dst)

def edit(p, pairs):
    raw = open(p, encoding='utf-8', newline='').read()
    nl = '\r\n' if '\r\n' in raw else '\n'
    for old, new in pairs:
        old = old.replace('\n', nl); new = new.replace('\n', nl)
        if raw.count(old) != 1:
            raise SystemExit(f"{p}: {raw.count(old)} of {old[:70]!r}")
        raw = raw.replace(old, new)
    open(p, 'w', encoding='utf-8', newline='').write(raw)

edit(os.path.join(dst, 'core.v'), [
('''wire signed [NUBITS-1:0] ula_out;
wire        [NUBITS-1:0] ula_fast;   // ula_out without the normalised operations
''', '''wire signed [NUBITS-1:0] ula_out;
wire signed [NUBITS-1:0] ula_alu;    // the ALU's own output
wire        [NUBITS-1:0] ula_fast;   // ula_out without the normalised operations
wire                     dot_wr;     // DOT's result takes the ALU's place
wire        [NUBITS-1:0] dot_res;
'''),
(''') ula (id_ula_op, ula_data_in1, ula_data_in2, ula_out, ula_fast, clk);
''', ''') ula (id_ula_op, ula_data_in1, ula_data_in2, ula_alu, ula_fast, clk);
assign ula_out = (dot_wr) ? dot_res : ula_alu;
'''),
('''			wire [NUBITS-1:0] res;
			arr_dot''', '''			arr_dot'''),
('''arr_data_rd, arr_data_b, res);''', '''arr_data_rd, arr_data_b, dot_res);'''),
('''			assign mem_rd_sel = (mac_rd) ? res : (arr_rd) ? arr_data_rd : mem_data_rd;''',
 '''			assign dot_wr     = mac_rd;
			assign mem_rd_sel = (arr_rd) ? arr_data_rd : mem_data_rd;'''),
('''			assign dot_go      = 1'b0;
			assign dot_pop     = 1'b0;
			assign arr_addr_rd = c_rd;''', '''			assign dot_go      = 1'b0;
			assign dot_pop     = 1'b0;
			assign dot_wr      = 1'b0;
			assign dot_res     = {NUBITS{1'b0}};
			assign arr_addr_rd = c_rd;'''),
('''	end else begin : mem_access
		assign dot_go      = 1'b0;
		assign dot_pop     = 1'b0;''', '''	end else begin : mem_access
		assign dot_go      = 1'b0;
		assign dot_pop     = 1'b0;
		assign dot_wr      = 1'b0;
		assign dot_res     = {NUBITS{1'b0}};'''),
("			st  <= ($signed(nval) > 1) ? RUN : TAIL;", "			st  <= ($signed(nval) > 1) ? RUN : DONE;"),
("				if (cnt == 2) st <= TAIL;", "				if (cnt == 2) st <= DONE;"),
("			TAIL : st <= DONE;\n", ""),
('''wire [NUBITS-1:0] sum;
reg  [NUBITS-1:0] res_r = 0;
mac_a1 #(.NB(NUBITS), .M(NBMANT), .E(NBEXPO), .W(2*NBMANT+8), .FR(FROUND)) mac (clk, clr, rv, qa, qb, sum);
always @ (posedge clk) res_r <= sum;
assign res = res_r;''', '''mac_a1 #(.NB(NUBITS), .M(NBMANT), .E(NBEXPO), .W(2*NBMANT+8), .FR(FROUND)) mac (clk, clr, rv, qa, qb, res);'''),
])
print("ulaout copy in", dst)
