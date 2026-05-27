// Params from  [https://www.ams.org/journals/mcom/1999-68-225/S0025-5718-99-00996-5/S0025-5718-99-00996-5.pdf]
module generate_random_32
#(
	parameter a    = 2891336453,  // This will actually have 2**32 cardinality, and will take 2.8 minutes to cycle back on 50MHz
	parameter c    = 12345,
	parameter bits = 32,
	parameter seed = 4123
)
(
	input        clk, rst,
	output [8:0] rand_out
);

reg [bits-1:0] random_uniform;

initial random_uniform <= seed;

wire [31:0] math_temp;

assign math_temp = (random_uniform * a) + c;

always @(posedge clk or posedge rst) begin
	if (rst)
		random_uniform <= seed;
	else
		random_uniform <= math_temp;
end

assign rand_out = random_uniform[bits-5: bits-13];

endmodule

