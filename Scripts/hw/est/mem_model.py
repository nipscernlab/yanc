"""Replace mem_instr / mem_data in the working copy of processor.v by a timing
model of a block RAM: the address ends in a register, the data starts at a
register, and nothing in between. The read data mixes a free-running LFSR
with the address (and, for the data memory, the write port), so no input is
optimised away and no output is a constant."""
import os, re
HERE = os.path.dirname(os.path.abspath(__file__))
p = os.path.join(HERE, 'processor.v')
s = open(p).read()

instr = '''module mem_instr
#(
	parameter NADDRE =  8,
	parameter NBDATA = 12,
	parameter FNAME  = "instr.mif"
)
(
	input                           clk,
	input      [$clog2(NADDRE)-1:0] addr,
	output reg [NBDATA        -1:0] data = 0
);
// timing model of a block RAM (est/mem_model.py)
reg [NBDATA-1:0] lfsr = 1;
always @ (posedge clk) begin
	lfsr <= {lfsr[NBDATA-2:0], lfsr[NBDATA-1] ^ lfsr[NBDATA-2]};
	data <= lfsr ^ addr;
end
endmodule
'''
data = '''module mem_data
#(
	parameter NADDRE =  8,
	parameter NBDATA = 32,
	parameter FNAME  = "data.mif"
)(
	input                                  clk,
	input                                  wr,
	input             [$clog2(NADDRE)-1:0] addr_rd, addr_wr,
	input      signed [NBDATA        -1:0] data_in,
	output reg signed [NBDATA        -1:0] data_out
);
// timing model of a block RAM (est/mem_model.py)
reg [NBDATA-1:0] lfsr = 1;
always @ (posedge clk) begin
	lfsr <= {lfsr[NBDATA-2:0], lfsr[NBDATA-1] ^ lfsr[NBDATA-2]};
	data_out <= lfsr ^ addr_rd ^ addr_wr ^ (wr ? data_in : 0);
end
endmodule
'''
a = s.index('module mem_instr'); b = s.index('endmodule', a) + len('endmodule')
s = s[:a] + instr + s[b:]
a = s.index('module mem_data'); b = s.index('endmodule', a) + len('endmodule')
s = s[:a] + data + s[b:]
open(p, 'w').write(s)
print('ok')
