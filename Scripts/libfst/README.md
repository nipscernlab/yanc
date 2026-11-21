# libfst

GTKWave's reader and writer of FST waveforms, which `gen_gtkw` uses to read the FST the simulations write (issue #217).

The files are copied unchanged from the copy Verilator 5.048 (2026-04-26) ships in `share/verilator/include/gtkwave/`:

- `fstapi.c`, `fstapi.h`, `fst_win_unistd.h`: Tony Bybell, MIT;
- `lz4.c`, `lz4.h`: Yann Collet, BSD 2-Clause;
- `fastlz.c`, `fastlz.h`: Ariya Hidayat, MIT.

Each file carries its license. The library needs zlib (`zlib.h`, `-lz`), which the Makefile links statically on Windows.

To update, copy the same files from a newer Verilator or from GTKWave's `lib/libfst/`, keep them unchanged, and say here which version they came from.
