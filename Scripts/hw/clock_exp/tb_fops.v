// The whole ALU on the operations that go through the normaliser (F_ADD,
// F_SU1, F_SU2, F_MLT, F_DIV, I2F, I2F_M, F_ROT), with operands as programs
// make them; prints every output. Run once with a reference ula.v and once
// with an experiment's: the two prints must be identical.
//   RAW = 0: normalised floats (top mantissa bit set) or the zero word, with
//            exponents often 0, 1 or 2 apart and mantissas often close
//            (cancellation); plus small and large integers, given to I2F
//            and I2F_M only (to the others they are unnormalised floats)
//   RAW = 1: any 32-bit pattern (unnormalised floats too)
module tb_fops;
parameter NUBITS = 32, MAN = 23, EXP = 8, FROUND = 0, RAW = 0, N = 20000;
reg  [5:0]        op;
reg  [NUBITS-1:0] a, b;
wire [NUBITS-1:0] out, out_fast;
wire              jz_nz;
reg               clk = 0;
reg               isint = 0;                    // an integer sample (I2F only)
ula #(.NUBITS(NUBITS), .NBMANT(MAN), .NBEXPO(EXP), .FROUND(FROUND),
      .F_ADD(1), .F_SU1(1), .F_SU2(1), .F_MLT(1), .F_DIV(1), .I2F(1), .I2F_M(1), .F_ROT(1))
    dut (.op(op), .in1(a), .in2(b), .out(out), .jz_nz(jz_nz), .out_fast(out_fast), .clk(clk));

integer seed, k, i, t, de;
reg [5:0] ops [0:7];
reg [63:0] r;

function [NUBITS-1:0] fnum(input integer e, input [MAN-1:0] m, input s);
    fnum = {s, e[EXP-1:0], m};
endfunction

task one;
    begin
        for (i = 0; i < 8; i = i+1) begin
            op = ops[i];
            if (isint && op != 6'd25 && op != 6'd26) ;               // an integer sample: I2F only
            else begin
            if (op == 6'd7) begin
                for (t = 0; t < 6; t = t+1) begin #1 clk = 1; #1 clk = 0; end   // the divider's register cuts
            end
            #1 $display("%0d %h %h %h", op, a, b, out);
            end
        end
    end
endtask

reg [MAN-1:0] ma, mb;
integer ea, eb;
initial begin
    ops[0] = 3; ops[1] = 47; ops[2] = 48; ops[3] = 5; ops[4] = 7; ops[5] = 25; ops[6] = 26; ops[7] = 46;
    seed = 4321 + RAW;
    for (k = 0; k < N; k = k+1) begin
        isint = 0;
        if (RAW) begin
            r = {$random(seed), $random(seed)};
            a = r[NUBITS-1:0];
            r = {$random(seed), $random(seed)};
            b = r[NUBITS-1:0];
        end else begin
            r  = {$random(seed), $random(seed)};
            ma = r[MAN-1:0] | ({{MAN-1{1'b0}}, 1'b1} << (MAN-1));
            ea = $signed(r[63:64-EXP]);                             // any exponent, the zero code too
            if (ea == -(1 << (EXP-1))) ma = 0;                      // the zero code carries a zero mantissa
            t  = {$random(seed)} % 8;
            de = (t < 5) ? (t % 3) : ({$random(seed)} % (2*MAN));   // mostly 0, 1, 2 apart
            if ($random(seed) & 1) de = -de;
            eb = ea + de;
            if (eb >  (1 << (EXP-1)) - 1) eb =  (1 << (EXP-1)) - 1;
            if (eb < -(1 << (EXP-1)) + 1) eb = -(1 << (EXP-1)) + 1;
            r  = {$random(seed), $random(seed)};
            case ({$random(seed)} % 4)                              // the other mantissa
                0, 1: mb = ma ^ (r[MAN-1:0] >> ({$random(seed)} % MAN));   // close: shares the top bits
                2:    mb = r[MAN-1:0];
                3:    mb = ma;
            endcase
            mb = mb | ({{MAN-1{1'b0}}, 1'b1} << (MAN-1));
            if ({$random(seed)} % 50 == 0) begin eb = -(1 << (EXP-1)); mb = 0; end
            a = fnum(ea, ma, r[63]);
            b = fnum(eb, mb, r[62]);
            if ({$random(seed)} % 4 == 0) begin                     // integers for I2F / I2F_M
                r = {$random(seed), $random(seed)};
                b = r[NUBITS-1:0] >> ({$random(seed)} % NUBITS);
                if (r[63]) b = -b;
                a = b;
                isint = 1;
            end
        end
        one;
    end
    $finish;
end
endmodule
