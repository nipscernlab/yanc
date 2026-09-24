`timescale 1ns/1ps

// Directed reset fixture: run ResetCheck to the end of its output burst
// (program then spins in while(1)), pulse rst for ONE clock cycle, and
// require the exact same burst again, five times at five loop phases. Locks in:
//   - the synchronous global reset works from a single-cycle pulse
//   - PC restarts from 0, instruction/data stack pointers rewind
//   - racc / ula_op / en_out reset cleanly (a spurious out_en would log a
//     stray value and break the golden compare)
// Data memory is NOT reset (RAM keeps contents) -- the program only outputs
// values it assigns at runtime before use, so the second burst must be
// bit-identical to the first. regress.sh additionally asserts that every
// sixth of output_reset.txt equals the first, independent of the golden file.

module reset_tb();

reg clk, rst;

always #5 clk = ~clk;

wire signed [15:0] out;
wire [0:0] out_en;

ResetCheck proc(clk, rst, out, out_en);

integer fd;
integer nout   = 0;   // outputs seen in total
integer quiet  = 0;   // clock cycles since the last output
integer bursts = 1;   // resets issued so far (1 = the boot reset)
integer armed  = 0;   // an output since the last reset: the next quiet spell may reset

initial begin
    fd  = $fopen("output_reset.txt", "w");
    clk = 0;
    rst = 1;
    #10 rst = 0;
    // waveform is opt-in (debug only); the regression reads output_reset.txt
    if ($test$plusargs("WAVE")) begin
        $dumpfile("reset_tb.vcd");
        $dumpvars(0, reset_tb);
    end
end

// log outputs exactly like the generated tb: value on `out` while out_en high
always @ (posedge clk) begin
    if (out_en == 1'b1) begin
        $fdisplay(fd, "%0d", out);
        nout  = nout + 1;
        quiet = 0;
        armed = 1;
    end else begin
        quiet = quiet + 1;
    end
end

// Once the program has been quiet after a burst (spinning in while(1)), pulse
// rst across exactly one posedge; after the sixth burst goes quiet, finish.
// Each reset waits one cycle longer (100, 101, ... 104), so the pulse lands on
// a different instruction of the spin loop every time: a reset used to fetch
// at the old PC, and when that word was the loop's JMP the program went back
// to spinning -- a single fixed delay caught it only in some loop phases.
// Driving on negedge keeps the pulse cleanly centered on a single rising edge
// -- the minimal stimulus a synchronous reset must obey.
always @ (negedge clk) begin
    if (armed && quiet == 99 + bursts) begin
        armed = 0;       // quiet keeps counting past the pulse: fire once per burst
        if (bursts == 6) begin
            $display("Info: six bursts captured (%0d outputs)", nout);
            $fflush(fd);   // vvp does not always flush buffers on $finish
            $finish;
        end
        bursts = bursts + 1;
        rst = 1;
        @(negedge clk) rst = 0;
    end
end

// watchdog: never hang the regression; a short/empty output then fails the
// golden compare with a useful message
initial begin
    #2000000;
    $display("Error: reset_tb watchdog timeout (%0d outputs)", nout);
    $fflush(fd);
    $finish;
end

endmodule
