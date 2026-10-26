`timescale 1ns/1ps

module pptr_tb();

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

reg  signed [31:0] proc_io_in = 0;
wire signed [31:0] proc_io_out;
wire [0:0] proc_req_in;
wire [0:0] proc_out_en;

/* verilator tracing_on */
pptr proc(clk,rst,proc_io_in,proc_io_out,proc_req_in,proc_out_en);

/* verilator tracing_off */

// input ports ----------------------------------------------------------------

// port 0 variables
integer data_in_0;
reg signed [31:0] in_0 = 0;
reg req_in_0 = 0;

// open a file for reading on each port
initial begin
    data_in_0 = $fopen("./input_0.txt", "r"); // place your input data in this file, beside the source
end

// decode input ports
always @ (*) begin
    proc_io_in = 0;
    // port 0 decoding
    if (proc_req_in[0]) proc_io_in = in_0;
    req_in_0 = proc_req_in[0];
end

// implement reading of the input data
integer scan_result;
always @ (negedge clk) begin  
    // reading port 0
    if (data_in_0 != 0 && proc_req_in[0]) scan_result = $fscanf(data_in_0, "%d", in_0);
end

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

always @ (posedge clk) if (proc.valr10 == 59) begin
    $display("Info: end of program!");
    $finish;
end

initial begin

    $dumpfile("pptr_tb.vcd");

    $dumpvars(0,pptr_tb.clk);
    $dumpvars(0,pptr_tb.rst);
    $dumpvars(0,pptr_tb.proc.req_in_sim_0);
    $dumpvars(0,pptr_tb.proc.in_sim_0);
    $dumpvars(0,pptr_tb.proc.out_en_sim_0);
    $dumpvars(0,pptr_tb.proc.out_sig_0);
    $dumpvars(0,pptr_tb.proc.valr2);
    $dumpvars(0,pptr_tb.proc.linetabs);
    $dumpvars(0,pptr_tb.proc.p_pptr.core.sp.pointeri);
    $dumpvars(0,pptr_tb.proc.p_pptr.core.sp.fl_max);
    $dumpvars(0,pptr_tb.proc.p_pptr.core.sp.fl_full);
    $dumpvars(0,pptr_tb.proc.p_pptr.core.ula.delta_float);
    $dumpvars(0,pptr_tb.proc.p_pptr.core.ula.delta_int);

    if ($test$plusargs("HEADER_ONLY")) begin #1; $dumpflush; $finish; end

    for (chrys = 10; chrys <= 100; chrys = chrys + 10) begin
        #400000.000000;  // wall-clock slice of the total sim time
        $display("Progress: %0d%% complete", chrys);
        $fflush;
    end

    $display("Simulation Complete!");
    $finish;

end

endmodule
