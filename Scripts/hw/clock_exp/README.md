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
| `param`, `pcase`, `pnorm` | the muxes generated from the opcode parameters (note 4.3); `pnorm` is the base of the normaliser variants below |
| `nshift`, `rpar`, `nsr` | normalizing shifter, rounding carry select, both |
| `dsp` | the normaliser's shift as a product by 2^k (a DSP block), 2^k = r & -r of the bit-reversed mantissa |
| `alm` | the leading-zero count as a radix-4 tree |
| `rshort` | the rounding increment only for a shift of 0 or 1 |
| `lza` | `F_ADD` normalised by leading-zero anticipation, then the normaliser's direct path |
| `twopath` | `F_ADD` in two paths (near: alignment by 0 or 1 and full normalising; far: full alignment and at most 2 bits), then the direct path |

| `sepadd` | the add as a separate circuit (own normaliser and rounding), a third input of the final mux |
| `sep2p` | `sepadd` with the near/far add inside |

The five from `dsp` to `twopath` are the methods of
[docs/design/normalisation-survey.md](../../../docs/design/normalisation-survey.md).

```sh
python3 Scripts/hw/clock_exp/make_variants.py
bash Scripts/hw/clock_exp/eq_norm.sh                       # the tree count gives the same normaliser output
HDL=$PWD/.smoke/hw/clock_exp/addr TAG=de10_addr bash Scripts/hw/fmax.sh sapho_all 5CSEBA6U23I7
```

`mux_x_cost.py` measures in Yosys what an unused `ula_mux` input tied to `x`
still costs: `python3 Scripts/hw/clock_exp/mux_x_cost.py` (2026-10-15: 235
against 229 LUT4 for an integer set, 306 against 253 for a float set).

`tb_fops.v` drives the whole ALU on the eight operations that go through the
normaliser, with normalised operands (`RAW=0`, as programs make them) and
with any bit pattern (`RAW=1`); `eq_fops.sh <variant> [N]` compares a
variant's prints with `pnorm`'s in five formats and rounding levels
(the reference print is cached in `eqf_ref/`). Only `RAW=0` has to be equal:
`rshort` and `twopath` assume normalised operands.

`tb_norm.v` drives `ula_norm` with random inputs biased to many leading
zeros and to zero mantissas and prints every output; `eq_norm.sh` runs it
against `SAPHO/ula.v` and against the `lzc` copy in nine formats and
rounding levels and compares the prints.

What the timed variants do NOT have yet, so a program run on them is wrong:
the compilers still write each division as three words (`stg4` needs four,
`addr` five for `DIV`/`MOD`), and nothing stops a `JIZ`, `LDI` or `ILI` right
after an operation that goes through the normaliser (none of the C± goldens
has one; the assembler would have to refuse or pad it).
