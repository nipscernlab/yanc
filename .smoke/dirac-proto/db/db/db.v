module db (

input  clk, rst,
input  signed [31:0] in ,
output signed [31:0] out,
output [1:0] req_in,
output [1:0] out_en);

/* verilator tracing_off */

wire itr = 1'b0;
wire cheguei;
wire [0:0] lk_toma, lk_cade, lk_valeu, lk_taqui;
wire [31:0] lk_cade_dado;
wire [31:0] lk_toma_dado;
assign lk_valeu[0] = 1'b0;
assign lk_taqui[0] = 1'b0;
assign lk_cade_dado[31:0] = 0;
wire proc_req_in, proc_out_en;
wire [0:0] addr_in;
wire [0:0] addr_out;

`ifdef __ICARUS__
 `ifndef YANC_SIM_VIS
  `define YANC_SIM_VIS
 `endif
`endif
`ifdef YANC_TRACE
 `ifndef YANC_SIM_VIS
  `define YANC_SIM_VIS
 `endif
`endif

`ifdef YANC_SIM_VIS
wire mem_wr;
wire [5:0] mem_addr_wr;
wire [8:0] pc_sim_val;
`endif

processor#(.NUBITS(32),
.NBMANT(23),
.NBEXPO(8),
.NBOPER(9),
.NUGAIN(1),
.MDATAS(46),
.MINSTS(368),
.SDEPTH(128),
.DDEPTH(128),
.NBIOIN(1),
.NBIOOU(1),
.FFTSIZ(3),
.FROUND(0),
.SET(1),
.LOD(1),
.F_MLT(1),
.P_LOD(1),
.SF_ADD(1),
.F_SU2(1),
.F_ADD(1),
.F_DIV(1),
.F_NEG(1),
.LES(1),
.JIZ(1),
.F_INN(1),
.INN(1),
.I2F(1),
.CAL(1),
.F2I(1),
.OUT(1),
.ADD(1),
.MLT(1),
.LDI(1),
.DFILE("db_data.mif"),
.IFILE("db_inst.mif"))

`ifdef YANC_SIM_VIS
p_db (clk, rst, in, out, addr_in, addr_out, proc_req_in, proc_out_en, itr, cheguei, lk_toma, lk_cade, lk_valeu, lk_taqui, lk_cade_dado, lk_toma_dado, mem_wr, mem_addr_wr,pc_sim_val);
`else
p_db (clk, rst, in, out, addr_in, addr_out, proc_req_in, proc_out_en, itr, cheguei, lk_toma, lk_cade, lk_valeu, lk_taqui, lk_cade_dado, lk_toma_dado);
`endif

addr_dec #(2) dec_in (proc_req_in, addr_in , req_in);
addr_dec #(2) dec_out(proc_out_en, addr_out, out_en);

/* verilator tracing_on */

// ----------------------------------------------------------------------------
// Simulation -----------------------------------------------------------------
// ----------------------------------------------------------------------------

`ifdef YANC_SIM_VIS

// I/O ------------------------------------------------------------------------

reg signed [31:0] in_sim_0 = 0;
reg req_in_sim_0 = 0;
reg signed [31:0] in_sim_1 = 0;
reg req_in_sim_1 = 0;

reg signed [31:0] out_sig_0 = 0;
reg out_en_sim_0 = 0;
reg signed [31:0] out_sig_1 = 0;
reg out_en_sim_1 = 0;

always @ (posedge clk) begin
   if (req_in[0]) in_sim_0 <= in;
   if (req_in[1]) in_sim_1 <= in;
end
always @ (*) begin
   req_in_sim_0 = req_in[0];
   req_in_sim_1 = req_in[1];
end

always @ (posedge clk) begin
   if (out_en[0]) out_sig_0 <= out;
   if (out_en[1]) out_sig_1 <= out;
end
always @ (*) begin
   out_en_sim_0 = out_en[0];
   out_en_sim_1 = out_en[1];
end

// variables ------------------------------------------------------------------

/* verilator tracing_off */  // float decode helpers (not traced)
reg signed [23:0] sm_me2; always @ (*) sm_me2 = (out[31]) ? -$signed({1'b0, out[22:0]}) : $signed({1'b0, out[22:0]});
reg signed [7:0] e_me2; always @ (*)  e_me2 = $signed(out[30:23]);
/* verilator tracing_on */

real me2_f_rls_update_v_d_e_ = 0.0;
real me2_f_rls_update_v_e_e_ = 0.0;
real me2_f_rls_update_v_g_e_ = 0.0;
reg [31:0] me1_f_main_v_j_e_ = 0;
reg [31:0] me1_f_main_v_r_e_ = 0;
reg [31:0] me1_f_main_v_c_e_ = 0;
reg [31:0] me1_f_main_v_ib_e_ = 0;

always @ (posedge clk) begin
   if (mem_wr && (pc_sim_val == 3)) me2_f_rls_update_v_d_e_ <= sm_me2*$pow(2.0,e_me2);
   if (mem_wr && (pc_sim_val == 16)) me2_f_rls_update_v_e_e_ <= sm_me2*$pow(2.0,e_me2);
   if (mem_wr && (pc_sim_val == 81)) me2_f_rls_update_v_g_e_ <= sm_me2*$pow(2.0,e_me2);
   if (mem_wr && (pc_sim_val == 263 || pc_sim_val == 333)) me1_f_main_v_j_e_ <= out;
   if (mem_wr && (pc_sim_val == 336 || pc_sim_val == 353 || pc_sim_val == 357 || pc_sim_val == 365)) me1_f_main_v_r_e_ <= out;
   if (mem_wr && (pc_sim_val == 338 || pc_sim_val == 348)) me1_f_main_v_c_e_ <= out;
   if (mem_wr && (pc_sim_val == 343 || pc_sim_val == 360)) me1_f_main_v_ib_e_ <= out;
end

// instructions ---------------------------------------------------------------

reg [31:0] valr2=0;
/* verilator tracing_off */
reg [31:0] valr1 /* verilator public_flat */=0;
reg [31:0] valr3 /* verilator public_flat */=0;
reg [31:0] valr4 /* verilator public_flat */=0;
reg [31:0] valr5 /* verilator public_flat */=0;
reg [31:0] valr6 /* verilator public_flat */=0;
reg [31:0] valr7 /* verilator public_flat */=0;
reg [31:0] valr8 /* verilator public_flat */=0;
reg [31:0] valr9 /* verilator public_flat */=0;
reg [31:0] valr10 /* verilator public_flat */=0;
/* verilator tracing_on */

reg [19:0] min [0:368-1];

/* verilator tracing_off */ reg signed [19:0] linetab /* verilator public_flat */ =-1; /* verilator tracing_on */
reg signed [19:0] linetabs=-1;

initial	$readmemb("pc_db_mem.txt",min);

always @ (posedge clk) begin
if (pc_sim_val < 368) linetab <= min[pc_sim_val];
linetabs <= linetab;   
valr1    <= {{(23){1'b0}}, pc_sim_val};
valr2    <= valr1;
valr3    <= valr2;
valr4    <= valr3;
valr5    <= valr4;
valr6    <= valr5;
valr7    <= valr6;
valr8    <= valr7;
valr9    <= valr8;
valr10   <= valr9;
end

`endif

endmodule