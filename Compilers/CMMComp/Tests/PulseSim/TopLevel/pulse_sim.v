module pulse_sim
(
	input clk, rst, itr,
	output [1:0] out_en,
	output signed [22:0] out
);

wire [8:0] rand_out;

generate_random_32 gen_rand(clk, rst, rand_out);

wire proc_req_in;

proc_sim proc(clk, rst, {14'b0, rand_out}, out, proc_req_in, out_en, itr);

endmodule