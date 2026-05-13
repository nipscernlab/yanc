module ProcDTWv4(input clk, rst_geral, rst_proc,
			input signed [31:0] in0, in1, in2,
			output reg signed [31:0] out0, out1, out2,
			output wire [2:0] req_in,
			output wire [2:0] out_en
		 );
		 
//reg signed [31:0] out0 = 32'd0;
reg signed [31:0] in_proc;

// Multiplex
always @(*)
begin
		case(req_in)
		3'b001: in_proc = in0; //out0[31:0];
		3'b010: in_proc = in1;
		3'b100: in_proc = in2;
		default : in_proc = 31'd0;
		endcase
end

wire signed [31:0] out_proc;


// Processor instance
ProcDTW DTWv4_inst(
								 .clk(clk),
								 .rst(rst_geral),
								 .in(in_proc),
								 .out(out_proc),
								 .req_in(req_in),
								 .out_en(out_en),
								 .itr(rst_proc)
								 );
always @(posedge clk)
begin
	if (rst_geral)
	begin
		out0 <= 32'd0;
		out1 <= 32'd0;
		out2 <= 32'd0;
	end
	else
	begin
		if(out_en[0])
			out0 <= out_proc;
		if(out_en[1])
			out1 <= out_proc;
		if(out_en[2])
			out2 <= out_proc;
	end
end

endmodule