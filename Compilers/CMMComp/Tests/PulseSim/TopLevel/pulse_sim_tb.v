module pulse_sim_tb();

reg clk, rst;

initial begin
	    clk = 0;
	    rst = 1;
	#20 rst = 0;
end

always #10 clk = ~clk;

wire signed [22:0] out;
wire        [1 :0] out_en;

pulse_sim sim (clk, rst, 1'b0, out_en, out);

reg signed [22:0] truth, readout;

always @ (posedge clk) begin
	if (out_en[0]) truth   <= out;
	if (out_en[1]) readout <= out;
end

integer i;
initial begin
	$dumpfile("pulse_sim_tb.vcd");
	$dumpvars(0,pulse_sim_tb);
    for (i = 10; i <= 100; i = i + 10) begin
		#40000;  // Simulate delay
        $display("Progress: %0d%% complete", i);
    end
    $display("Simulation Complete!");
    $finish;
end

endmodule