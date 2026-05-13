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
initial begin
	data_in1 = $fopen("sinal_harm_q.txt", "r");
	$dumpfile("top_level_tb.vcd");
	$dumpvars(0,top_level_tb);
    for (i = 10; i <= 100; i = i + 10) begin
		#450000;  // Simulate delay
        $display("Progress: %0d%% complete", i);
    end
    $display("Simulation Complete!");
    $finish;
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