`timescale 1ns/1ps
// The RTL side of the instruction-set simulator's comparison (#32): runs a
// generated processor and prints, cycle by cycle, what Scripts/iss/sapho_iss.c
// prints for the same program:
//
//     <cycle> F <addr>          the instruction address fetched
//     <cycle> W <addr> <word>   a data-memory write
//     <cycle> I <port>          an input read
//     <cycle> O <port> <word>   an output
//     <cycle> D <words>         the data stack's depth, when it changes
//     <cycle> R <words>         the instruction stack's depth, when it changes
//
// The depths are the stacks' own counters (fl_cnt, simulation only): a word
// left on a stack shows here even when no instruction ever reads it.
// Cycle 0 is the first cycle after the first clock edge out of reset. The
// signals are read in the middle of each cycle (falling edge), from the
// generated top (<proc>.v), whose simulation wires carry the fetch address and
// the memory write (YANC_SIM_VIS: Icarus defines it, Verilator gets
// +define+YANC_TRACE).
//
// Defines: PROC (the processor's module), PINST (its processor instance,
// dut.p_<proc>), NU (NUBITS), NI and NO (the widths of its req_in and out_en
// ports), CAL when it has an instruction stack, ITR when it has an interrupt
// (#ITRAD). Plusargs: +N=<cycles> to run, +T=<file> to write, +ITR0=<c> to
// +ITR7=<c>: the cycles in which the interrupt pin is high.
module tb;

reg clk = 0, rst = 1, itr = 0;
always #5 clk = ~clk;

wire [31:0]     in_w;
wire [`NU-1:0]  out;
wire [`NI-1:0]  req_in;
wire [`NO-1:0]  out_en;

// a top has an itr port only with #ITRAD (define ITR then)
`PROC dut (.clk(clk), .rst(rst), .in(in_w[`NU-1:0]), .out(out), .req_in(req_in), .out_en(out_en)
`ifdef ITR
           , .itr(itr)
`endif
          );

// the input: the k-th read of the run at port p, as sapho_iss.c's iss_in
function [31:0] iss_in (input [31:0] k, input [31:0] p);
	reg [31:0] h;
	begin
		h = (k * 32'd2654435761) ^ (p * 32'd40503);
		h = h ^ (h >> 15); h = h * 32'd2246822519; h = h ^ (h >> 13);
		if (h[1:0] == 2'd0) iss_in = ((h >> 8) % 33) - 16;
		else                iss_in = h;
	end
endfunction

reg [31:0] nin = 0;
assign in_w = iss_in(nin, {31'd0, dut.addr_in});
always @ (posedge clk) if (!rst && dut.proc_req_in) nin <= nin + 1;

// the interrupt cycles
integer nitr = 0, itrc [0:7], iv, ncyc, fd;
reg [8*256-1:0] tfile;

function integer in_itr (input integer c);
	integer q;
	begin
		in_itr = 0;
		for (q = 0; q < nitr; q = q + 1) if (itrc[q] == c) in_itr = 1;
	end
endfunction

integer cnext = 0;           // the cycle the next rising edge starts
reg     run   = 0;

initial begin
	if (!$value$plusargs("N=%d", ncyc)) ncyc = 1000;
	if (!$value$plusargs("T=%s", tfile)) tfile = "rtl.trace";
	if ($value$plusargs("ITR0=%d", iv)) begin itrc[0] = iv; nitr = 1; end
	if ($value$plusargs("ITR1=%d", iv)) begin itrc[1] = iv; nitr = 2; end
	if ($value$plusargs("ITR2=%d", iv)) begin itrc[2] = iv; nitr = 3; end
	if ($value$plusargs("ITR3=%d", iv)) begin itrc[3] = iv; nitr = 4; end
	if ($value$plusargs("ITR4=%d", iv)) begin itrc[4] = iv; nitr = 5; end
	if ($value$plusargs("ITR5=%d", iv)) begin itrc[5] = iv; nitr = 6; end
	if ($value$plusargs("ITR6=%d", iv)) begin itrc[6] = iv; nitr = 7; end
	if ($value$plusargs("ITR7=%d", iv)) begin itrc[7] = iv; nitr = 8; end
	fd = $fopen(tfile, "w");
	repeat (3) @ (posedge clk);
	@ (negedge clk) rst = 0;
end

// a rising edge out of reset starts cycle cnext; the interrupt pin is set
// just after it, for that cycle
always @ (posedge clk) if (!rst) begin
	run <= 1'b1;
	#1;
	itr = in_itr(cnext);
	cnext = cnext + 1;
	if (cnext > ncyc) begin $fclose(fd); $finish; end
end

integer last_d = -1, last_r = -1;

always @ (negedge clk) if (run) begin
	if (`PINST.core.sp.fl_cnt != last_d) begin
		$fdisplay(fd, "%0d D %0h", cnext - 1, `PINST.core.sp.fl_cnt); last_d = `PINST.core.sp.fl_cnt;
	end
`ifdef CAL
	if (`PINST.core.instr_fetch.isp_blk.isp.fl_cnt != last_r) begin
		$fdisplay(fd, "%0d R %0h", cnext - 1, `PINST.core.instr_fetch.isp_blk.isp.fl_cnt); last_r = `PINST.core.instr_fetch.isp_blk.isp.fl_cnt;
	end
`else
	if (last_r != 0) begin $fdisplay(fd, "%0d R 0", cnext - 1); last_r = 0; end
`endif
	$fdisplay(fd, "%0d F %0h", cnext - 1, dut.pc_sim_val);
	if (dut.mem_wr)      $fdisplay(fd, "%0d W %0h %0h", cnext - 1, dut.mem_addr_wr, out);
	if (dut.proc_req_in) $fdisplay(fd, "%0d I %0h", cnext - 1, dut.addr_in);
	if (dut.proc_out_en) $fdisplay(fd, "%0d O %0h %0h", cnext - 1, dut.addr_out, out);
end

endmodule
