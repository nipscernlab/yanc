// Counts, cycle by cycle, which ALU operation the core executes (id_ula_op),
// and writes the histogram at $finish. `TB is the testbench top, `CORE the
// core instance under it; both set with -D on the iverilog command line.
module probe;
    integer cnt [0:63];
    integer i, fd, other, done;
    initial begin
        for (i = 0; i < 64; i = i + 1) cnt[i] = 0;
        other = 0; done = 0;
    end
    always @(posedge `TB.clk)
        if (`TB.rst !== 1'b1 && !done) begin
            if (`CORE.instr_fetch.pc_addr == `FIM) begin done = 1; $finish; end   // the program ended: @fim JMP fim
            if (^(`CORE.id_ula_op) === 1'bx) other = other + 1;
            else cnt[`CORE.id_ula_op] = cnt[`CORE.id_ula_op] + 1;
        end
    final begin
        fd = $fopen(`OUTF, "w");
        for (i = 0; i < 64; i = i + 1) if (cnt[i] != 0) $fdisplay(fd, "%0d %0d", i, cnt[i]);
        $fdisplay(fd, "x %0d", other);
        $fclose(fd);
    end
endmodule
