`timescale 1ns/1ps
// Project pass ITR (Scripts/regress.sh): the #PRACA restart and the #TOAQUI
// pin of Tests/cmm_itr (docs/directives.md). The program passes #PRACA once
// on its own and puts 100, then waits at @fim; the testbench pulls itr up for
// three clocks, twice, once the program waits, and each time the program
// restarts at #PRACA and puts n + 100. Every value on port 0 goes to
// output_itr.txt; the rising edges of cheguei, three a pass, are counted into
// output_cheguei.txt. regress checks them against 100 200 300 and 9.
module itr_tb;

reg clk = 0; always #10 clk = ~clk;
reg rst = 1;
reg itr = 0;

wire [31:0] out;
wire [0:0]  out_en;
wire        cheguei;
cmm_itr P (.clk(clk), .rst(rst), .out(out), .out_en(out_en), .itr(itr), .cheguei(cheguei));

integer f_out, f_chg;
integer n_out = 0, n_chg = 0;
reg     chg_d = 0;

always @(posedge clk) if (!rst && out_en[0]) begin $fdisplay(f_out, "%0d", out); n_out = n_out + 1; end
always @(posedge clk) begin
    chg_d <= cheguei;
    if (!rst && cheguei && !chg_d) n_chg = n_chg + 1;
end

// itr up for three clocks, changed on the falling edge, away from the
// processor's
task pulse; begin @(negedge clk) itr = 1; repeat (3) @(negedge clk); itr = 0; end endtask

task close_all; begin $fdisplay(f_chg, "%0d", n_chg); $fclose(f_out); $fclose(f_chg); end endtask

initial begin
    f_out = $fopen("output_itr.txt", "w");
    f_chg = $fopen("output_cheguei.txt", "w");
    #95 rst = 0;
    wait (n_out == 1); #2000 pulse;    // the program has long reached @fim
    wait (n_out == 2); #2000 pulse;
    wait (n_out == 3); #2000;
    close_all;
    $finish;
end

// a restart that never comes ends here: the outputs are then short
initial begin #200000; close_all; $finish; end

endmodule
