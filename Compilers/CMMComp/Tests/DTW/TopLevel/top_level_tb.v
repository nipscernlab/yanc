`timescale 1ns/1ps

module top_level_tb();

// Instance variables
reg clk, rst_geral, rst_proc;
reg wrreq = 1'b0;
reg [13:0] cont_rst_proc;
reg signed [15:0] data = 16'd0;
wire signed [31:0] out1, out2, out3, out4, out1_DTW, out2_DTW;
wire [1:0] req_in;
wire [4:0] out_en;
wire [2:0] out_en_DTW;
wire empty, full, almost_empty;
wire signed [15:0] q;
wire signed [6:0] usedw;
// Intermediate variables for reading

// Clock
always #2 clk <= ~clk;

// Reset test
initial
fork
	clk <= 1'b0;
	rst_proc <= 1'b0;
	rst_geral <= 1'b0;
	#20 rst_geral <= 1'b1;
	#40 rst_geral <= 1'b0;
join

integer i, data_in1;
integer fd_zc_1, fd_zc_2, fd_zc_3, fd_zc_4, fd_dtw_1, fd_dtw_2;
reg [4:0] out_en_dly     = 5'd0;
reg [2:0] out_en_DTW_dly = 3'd0;

initial begin
	data_in1 = $fopen("sinal_harm_q.txt", "r");
	// per-port logs used by regress.sh as goldens (relative paths -> vvp cwd)
	fd_zc_1  = $fopen("output_zc_1.txt" , "w");
	fd_zc_2  = $fopen("output_zc_2.txt" , "w");
	fd_zc_3  = $fopen("output_zc_3.txt" , "w");
	fd_zc_4  = $fopen("output_zc_4.txt" , "w");
	fd_dtw_1 = $fopen("output_dtw_1.txt", "w");
	fd_dtw_2 = $fopen("output_dtw_2.txt", "w");
	// waveform dump is opt-in via +WAVE. Dumping every signal of this heavy
	// multi-proc sim through the FST writer intermittently crashed vvp on
	// Windows (exit 1, no message) -- and the regression never reads the
	// waveform, only the output_*.txt logs. go_proj(.bat)/go_proj_vl(.bat)
	// pass +WAVE to get the GTKWave trace; regress runs without it.
	// +HEADER_ONLY also dumps, then bails after one tick: gen_gtkw only needs the
	// VCD header (the signal list). $dumpvars at t=0 merely registers scopes, so
	// advance #1 and $dumpflush before $finish or the VCD has $scope but no $var.
	// This gives the formatter the signal list without writing the multi-GB body.
	if ($test$plusargs("WAVE") || $test$plusargs("HEADER_ONLY")) begin
		$dumpfile("top_level_tb.vcd");
		$dumpvars(0,top_level_tb);
	end
	if ($test$plusargs("HEADER_ONLY")) begin
		#1; $dumpflush; $finish;
	end
    for (i = 10; i <= 100; i = i + 10) begin
		#450000;  // Simulate delay
        $display("Progress: %0d%% complete", i);
    end
    $display("Simulation Complete!");
    // force the per-port logs to disk before exit -- on this heavy sim vvp
    // does not always flush the file buffers on $finish, which left the
    // golden outputs intermittently empty (regress DTW flake)
    $fflush(fd_zc_1); $fflush(fd_zc_2); $fflush(fd_zc_3); $fflush(fd_zc_4);
    $fflush(fd_dtw_1); $fflush(fd_dtw_2);
    $finish;
end

// log each port the cycle after its enable was asserted (when outN holds the
// latched value driven by the wrapper module's nonblocking assignment)
always @(posedge clk) begin
    out_en_dly     <= out_en;
    out_en_DTW_dly <= out_en_DTW;
    if (out_en_dly[1])     $fdisplay(fd_zc_1 , "%0d", out1);
    if (out_en_dly[2])     $fdisplay(fd_zc_2 , "%0d", out2);
    if (out_en_dly[3])     $fdisplay(fd_zc_3 , "%0d", out3);
    if (out_en_dly[4])     $fdisplay(fd_zc_4 , "%0d", out4);
    if (out_en_DTW_dly[1]) $fdisplay(fd_dtw_1, "%0d", out1_DTW);
    if (out_en_DTW_dly[2]) $fdisplay(fd_dtw_2, "%0d", out2_DTW);
end

integer scan_result;
always @(posedge clk)
begin
//	if (req_in[1] == 1'b1)
	if (rst_proc)
	begin
		// Place the data on the bus
		scan_result = $fscanf(data_in1, "%d", data);
		// Pulse on FIFO write req
		wrreq <= 1'b1;
		#5 wrreq<= 1'b0;
	end
end

// Proc reset counter
always@(posedge clk or posedge rst_geral)
begin
	if (rst_geral == 1'b1)
	begin
		cont_rst_proc <= 14'd0;
	end
	
	else 
	begin
		cont_rst_proc <= cont_rst_proc + 14'd1;
	end
	
end

always@(posedge clk)
begin
	if (cont_rst_proc == 14'd16383)
	begin
		rst_proc <= 1'b1;
	end
	
	else 
	begin
		rst_proc <= 1'b0;
	end
end

top_level top_level_inst(
					          .data(data),
					          .wrreq(wrreq),
					          .clk(clk),
					          .rst_geral(rst_geral),
					          .rst_proc(rst_proc),
					          .out1(out1),
					          .out2(out2),
					          .out3(out3),
					          .out4(out4),
					          .out1_DTW(out1_DTW),
					          .out2_DTW(out2_DTW),
					          .empty(empty),
					          .full(full),
					          .almost_empty(almost_empty),
					          .q(q),
					          .usedw(usedw),
					          .req_in(req_in),
					          .out_en(out_en),
					          .out_en_DTW(out_en_DTW)
					          );


endmodule