# Clock experiments (2026-10-15)

Changes to the processor, timed on `sapho_all` to see how far the clock
goes without the dividers and the float path limiting it. They are
**experiments**: `make_variants.py` builds them as copies of `SAPHO/` under
`.smoke/hw/clock_exp/` and nothing in `SAPHO/` changes. Results, and what
each change still needs before it can be real, in
[docs/design/pipeln-and-division.md](../../../docs/design/pipeln-and-division.md)
section 4.3 and TODO item 29.

| variant | change (each contains the ones above) |
|---|---|
| `stg4` | both dividers in 4 stages (`div_array` `STG` 3 -> 4) |
| `jz` | `JIZ` tests a copy of the ALU output mux with the normaliser input tied to 0 |
| `lzc` | the normaliser's leading-zero count as a tree (`ula_lzc`) instead of a chain of W-1 muxes; same results |
| `addr` | the indirect read index (`LDI`/`ILI`) from that same fast output; the integer divider in 5 stages |
| `split` | `addr` with the ALU output split instead of copied: one mux of the non-normalised operations (`out_fast`) and a 2-input mux choosing the normaliser for the accumulator |

```sh
python3 Scripts/hw/clock_exp/make_variants.py
bash Scripts/hw/clock_exp/eq_norm.sh                       # the tree count gives the same normaliser output
HDL=$PWD/.smoke/hw/clock_exp/addr TAG=de10_addr bash Scripts/hw/fmax.sh sapho_all 5CSEBA6U23I7
```

`mux_x_cost.py` measures in Yosys what an unused `ula_mux` input tied to `x`
still costs: `python3 Scripts/hw/clock_exp/mux_x_cost.py` (2026-10-15: 235
against 229 LUT4 for an integer set, 306 against 253 for a float set).

`tb_norm.v` drives `ula_norm` with random inputs biased to many leading
zeros and to zero mantissas and prints every output; `eq_norm.sh` runs it
against `SAPHO/ula.v` and against the `lzc` copy in nine formats and
rounding levels and compares the prints.

What the timed variants do NOT have yet, so a program run on them is wrong:
the compilers still write each division as three words (`stg4` needs four,
`addr` five for `DIV`/`MOD`), and nothing stops a `JIZ`, `LDI` or `ILI` right
after an operation that goes through the normaliser (none of the C± goldens
has one; the assembler would have to refuse or pad it).
