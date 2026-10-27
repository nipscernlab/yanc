# Contributing to YANC

Thanks for taking the time to look at YANC. This document covers the few
conventions that aren't obvious from the source.

## Reporting issues

Open a GitHub issue with:

- a short description of what you expected vs. what happened;
- the smallest input that reproduces it (a `.cmm` snippet is usually enough);
- the exact command line and the messages each compiler printed.

Crash reports without a reproducer are hard to act on — please include one
even if it's just a stripped-down example.

## Setting up

See [docs/develop/building.md](docs/develop/building.md) — you'll
need MSYS2 with `mingw-w64-x86_64-gcc`, `make`, `bison` and `flex`. On a
clean checkout, `make` builds the six tools into `bin/`.

## Submitting changes

1. **Branch off `main`.**
2. **Keep commits focused.** One logical change per commit is easier to
   review and easier to revert if something goes wrong.
3. **Make sure CI passes.** Every push to `main` (and every PR) builds the
   six tools with `-O2 -Wall -Werror` on Windows and on Linux, runs
   cmmcomp → asmcomp on every test in `Compilers/CMMComp/Tests/`, runs the
   README quick start and builds the manual with every warning an error.
   If your change breaks any of these, the PR will be blocked. CI does not
   compare goldens or simulate: run `Scripts/regress.sh` before you push
   ([AGENTS.md](AGENTS.md), "The one command").
4. **Open the PR against `main`** with a description that explains *why*
   the change is needed, not just what it does.

## Code conventions

### Comments

Source comments are in **English**. Mixing languages makes the codebase
harder to read for newcomers and makes diffs noisier. If you're translating
or adding comments, English only.

### User-facing strings

YANC prints its diagnostics in English by default, in Portuguese with
`--lang=pt` or `YANC_LANG=pt`. In cmmcomp and asmcomp, user-facing strings
live in `<Component>/Headers/messages.h` and use the `M(pt, en)` macro plus
a named `MSG_*` identifier (the `yanc` driver uses `M()` inline; cppcomp
and `gen_gtkw` are English only for now, issue #24):

```c
#define MSG_ERR_NO_MAIN \
    M("Erro: cadê a função main()?\n", \
      "Error: yo, where's the main() function?\n")
```

Call sites then reference `MSG_ERR_NO_MAIN`, not inline `M()` literals:

```c
fprintf(stderr, MSG_ERR_NO_MAIN);
```

When you add a new diagnostic:

- pick a descriptive `MSG_*` name;
- write both PT and EN versions (keep the playful tone of the existing
  messages — `cadê`, `viajou`, `Sweet:`, `Heads up`, etc.);
- if the same message recurs at multiple call sites, share the same
  identifier instead of duplicating.

### Memory and tables

The symbol table of cmmcomp (`v_table`, one `symbol` row per name, in
`variaveis.c`) is grow-on-demand: it starts small and `realloc`s on
overflow, with `memset` to keep the BSS-zero semantics that callers rely
on. A new attribute of a name is a new field of `symbol`, and it grows
with the table through `var_grow`.

## Commit message style

The recent history follows the [Conventional Commits] form
(`feat: …`, `fix: …`, `docs: …`, `chore: …`, `ci: …`, `perf: …`,
`refactor: …`). Keep doing that.

Title: short imperative summary, lowercase after the prefix, no trailing
period. Body (optional but recommended for non-trivial changes):
explain the *why*. Wrap body lines at ~72 columns.

[Conventional Commits]: https://www.conventionalcommits.org/
