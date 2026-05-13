// Quartus Prime Verilog Template
// 4-State Moore state machine

// A Moore machine's outputs are dependent only on the current state.
// The output is written only when the state changes.  (State
// transitions are synchronous.)

module maq_estados
(
	input	clk, rst_geral, flag_zc, almost_empty_FIFO,
	output reg [1:0] out // out[1] the enable of the used_w - 1 register; out[0] the rst of the dtw proc ;
);

	// Declare the state register to be "safe" to implement
	// a safe state machine that can recover gracefully from
	// an illegal state (by returning to the reset state).
	(* syn_encoding = "safe" *) reg [1:0] state;

	// Declare states
	parameter S0 = 0, S1 = 1, S2 = 2, S3 = 3;

	// Output depends only on the state
	always @ (state) begin
		case (state)
			S0:
				out = 2'b00; //wait for flag_zc, does not reset proc dtw and does not store used_w - 1: out[1:0] = 00
			S1:
				out = 2'b10; //flag_zc happened, store the value of usedw - 1 in a reg (enable) and still does not assert rst proc dtw
			S2:
				out = 2'b01; //clear the enable of the usedw - 1 reg, and assert rst_proc_dtw
			S3:
				out = 2'b00; //bring rst_proc_dtw back to 0
			default:
				out = 2'b00; 
		endcase
	end

	// Determine the next state
	always @ (posedge clk or posedge rst_geral) begin
		if (rst_geral)
			state <= S0;
		else
			case (state)
				S0:
					if (flag_zc)
						state <= S1;
					else
						state <= S0;
				S1:
					state <= S2;
				S2:
					state <= S3;
				S3:
					if (almost_empty_FIFO)
						state <= S0;
					else
						state <= S3;
			endcase
	end

endmodule
