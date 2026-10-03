# Chip-agnostic delay table (experiment, TODO item 19)

How much longer each group of ALU operators makes the worst register-to-register
path of the whole processor, relative to a base with only the light operators,
on current FPGA families. Used to estimate the clock a paused pipeline would buy.

1. Copy `.smoke/work/sapho_all/Hardware/sapho_all.v` and `SAPHO/*.v` into a work
   folder with these scripts, and run `mem_model.py` there: it replaces the two
   memories of `processor.v` by a block-RAM timing model (registered address and
   data, nothing in between), so the program contents neither inflate nor
   simplify the paths.
2. `python est.py ice40,xc7` synthesizes `sapho_all` variants (base, each heavy
   group alone, full) with Yosys for iCE40 and Xilinx 7-series and reads the
   worst arrival with `sta` (cell delays only, no routing).
3. `python route.py` adds a routing estimate to the Xilinx paths (r ns per net
   outside a carry chain).
4. Cyclone V: the same variants through Quartus (`Scripts/hw/fmax.sh`), real
   routing.

ECP5 and Gowin are out: their carry cells have no timing in Yosys 0.56.
