# Timing of the DE10-Nano HDMI text screen.
create_clock -name clk50 -period 20.000 [get_ports FPGA_CLK1_50]
derive_pll_clocks
derive_clock_uncertainty

# The pixel clock leaves inverted on HDMI_TX_CLK, and the ADV7513 latches D,
# DE, HS and VS on its rising edge. Setup 1.0 ns and hold 0.7 ns are tVSU and
# tVHLD of the ADV7513 Hardware User's Guide (Rev. 0, AC specifications;
# read from a text extraction whose columns are scrambled, so to be checked
# on the PDF). The board's traces are taken as matched (not modelled).
create_generated_clock -name tx_clk -invert \
	-source [get_pins -compatibility_mode {u_pll|u|auto_generated|generic_pll1~PLL_OUTPUT_COUNTER|divclk}] \
	[get_ports HDMI_TX_CLK]
set_output_delay -clock tx_clk -max  1.0 [get_ports {HDMI_TX_D[*] HDMI_TX_DE HDMI_TX_HS HDMI_TX_VS}]
set_output_delay -clock tx_clk -min -0.7 [get_ports {HDMI_TX_D[*] HDMI_TX_DE HDMI_TX_HS HDMI_TX_VS}]

# The key is debounced on the board and synchronized in the top; the I2C runs
# at 7.9 kHz, one quarter of a bit per video line: no timing to check there.
set_false_path -from [get_ports {KEY[0]}]
set_false_path -to   [get_ports {I2C_SCL I2C_SDA}]
set_false_path -from [get_ports {I2C_SCL I2C_SDA}]
