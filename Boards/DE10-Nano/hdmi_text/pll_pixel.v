// 50 MHz -> 25.1786 MHz (50 x 141 / 280), 0.014% above the nominal 25.175
// MHz of 640x480 at 60 Hz. The altpll megafunction written by hand, so the
// project needs no generated IP; on a Cyclone V Quartus maps it to a
// fractional PLL in integer mode (VCO 881.25 MHz, C = 35; the fit report's
// "PLL Usage Summary" shows it). The same parameters gave 25.178511 MHz on
// the DE10-Nano in Dênis's project (2026-11-05). An altera_pll written by
// hand with output_clock_frequency0 = "25.175000 MHz" was tried and left its
// counters uncomputed (0.877 MHz out, Quartus 23.1): its wizard has to run.
module pll_pixel (input refclk, output outclk, output locked);
	wire [4:0] clk;
	altpll #(
		.intended_device_family ("Cyclone V"),
		.operation_mode         ("NORMAL"),
		.inclk0_input_frequency (20000),        // ps
		.clk0_multiply_by       (141),
		.clk0_divide_by         (280),
		.clk0_duty_cycle        (50),
		.clk0_phase_shift       ("0"),
		.port_locked            ("PORT_USED"),
		.pll_type               ("AUTO"),
		.lpm_type               ("altpll")
	) u (
		.inclk  ({1'b0, refclk}),
		.clk    (clk),
		.locked (locked)
	);
	assign outclk = clk[0];
endmodule
