`timescale 1ns/1ps

module al_tb();

// clock and reset generation -------------------------------------------------

reg clk, rst;

initial begin
    clk = 0;
    rst = 1;
    #10.000000;
    rst = 0;
end

always #5.000000 clk = ~clk;

// processor instance ---------------------------------------------------------

/* verilator tracing_off */

wire signed [31:0] proc_io_out;
wire [0:0] proc_out_en;

/* verilator tracing_on */
al proc(clk,rst,proc_io_out,proc_out_en);

/* verilator tracing_off */

// output ports ---------------------------------------------------------------

// port 0 variables
integer data_out_0;
reg signed [31:0] out_sig_0 = 0;
reg out_en_0 = 0;

// open a file for writing on each port
initial begin
    data_out_0 = $fopen("output_0.txt", "w"); // check the output data in this file
end

// decode output ports
always @ (*) begin
    // port 0 decoding
    out_sig_0 = proc_io_out;
    out_en_0  = proc_out_en[0];
end

// implement writing to the file
always @ (posedge clk) begin
    // write to port 0
    if (out_en_0 == 1'b1) begin $fdisplay(data_out_0, "%0d", out_sig_0); $fflush(data_out_0); end
end

// signal registration, progress bar and finish ------------------------------

integer chrys;

always @ (posedge clk) if (proc.valr10 == 43) begin
    $display("Info: end of program!");
    $finish;
end

initial begin

    $dumpfile("al_tb.vcd");

    $dumpvars(0,al_tb.clk);
    $dumpvars(0,al_tb.rst);
    $dumpvars(0,al_tb.proc.out_en_sim_0);
    $dumpvars(0,al_tb.proc.out_sig_0);
    $dumpvars(0,al_tb.proc.valr2);
    $dumpvars(0,al_tb.proc.linetabs);
    $dumpvars(0,al_tb.proc.p_al.core.sp.pointeri);
    $dumpvars(0,al_tb.proc.p_al.core.sp.fl_max);
    $dumpvars(0,al_tb.proc.p_al.core.sp.fl_full);
    $dumpvars(0,al_tb.proc.p_al.core.ula.delta_float);
    $dumpvars(0,al_tb.proc.p_al.core.ula.delta_int);

    if ($test$plusargs("HEADER_ONLY")) begin #1; $dumpflush; $finish; end

    for (chrys = 10; chrys <= 100; chrys = chrys + 10) begin
        #3000.000000;  // wall-clock slice of the total sim time
        $display("Progress: %0d%% complete", chrys);
        $fflush;
    end

    $display("Simulation Complete!");
    $finish;

end

endmodule
