module al (

input  clk, rst,
output signed [31:0] out,
output [0:0] out_en);

/* verilator tracing_off */

wire [31:0] in = 0;
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
wire [3:0] mem_addr_wr;
wire [5:0] pc_sim_val;
`endif

processor#(.NUBITS(32),
.NBMANT(23),
.NBEXPO(8),
.NBOPER(6),
.NUGAIN(1),
.MDATAS(12),
.MINSTS(44),
.SDEPTH(2),
.DDEPTH(2),
.NBIOIN(1),
.NBIOOU(1),
.FFTSIZ(3),
.FROUND(0),
.LOD(1),
.MLT(1),
.ADD(1),
.P_LOD(1),
.STI(1),
.SET(1),
.F_MLT(1),
.SF_ADD(1),
.F2I(1),
.OUT(1),
.DFILE("al_data.mif"),
.IFILE("al_inst.mif"))

`ifdef YANC_SIM_VIS
p_al (clk, rst, in, out, addr_in, addr_out, proc_req_in, proc_out_en, itr, cheguei, lk_toma, lk_cade, lk_valeu, lk_taqui, lk_cade_dado, lk_toma_dado, mem_wr, mem_addr_wr,pc_sim_val);
`else
p_al (clk, rst, in, out, addr_in, addr_out, proc_req_in, proc_out_en, itr, cheguei, lk_toma, lk_cade, lk_valeu, lk_taqui, lk_cade_dado, lk_toma_dado);
`endif

assign out_en = proc_out_en;

/* verilator tracing_on */

// ----------------------------------------------------------------------------
// Simulation -----------------------------------------------------------------
// ----------------------------------------------------------------------------

`ifdef YANC_SIM_VIS

// I/O ------------------------------------------------------------------------

reg signed [31:0] out_sig_0 = 0;
reg out_en_sim_0 = 0;

always @ (posedge clk) begin
   if (out_en[0]) out_sig_0 <= out;
end
always @ (*) begin
   out_en_sim_0 = out_en[0];
end

// variables ------------------------------------------------------------------


always @ (posedge clk) begin
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

reg [19:0] min [0:44-1];

/* verilator tracing_off */ reg signed [19:0] linetab /* verilator public_flat */ =-1; /* verilator tracing_on */
reg signed [19:0] linetabs=-1;

initial	$readmemb("pc_al_mem.txt",min);

always @ (posedge clk) begin
if (pc_sim_val < 44) linetab <= min[pc_sim_val];
linetabs <= linetab;   
valr1    <= {{(26){1'b0}}, pc_sim_val};
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