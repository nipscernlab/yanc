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

See the **Build from source** section of the [README](README.md) — you'll
need MSYS2 with `mingw-w64-x86_64-gcc`, `bison`, and `flex`. On a clean
checkout, `Scripts/aurora.bat` produces all six binaries.

## Submitting changes

1. **Branch off `main`.**
2. **Keep commits focused.** One logical change per commit is easier to
   review and easier to revert if something goes wrong.
3. **Make sure CI passes.** Every push to `main` (and every PR) builds all
   four compilers with `-O2 -Wall` and smoke-tests the full pipeline
   (cmmcomp → appcomp → asmcomp) against every example in `Exemplos/`. If
   your change breaks any of these, the PR will be blocked.
4. **Open the PR against `main`** with a description that explains *why*
   the change is needed, not just what it does.

## Code conventions

### Comments

Source comments are in **English**. Mixing languages makes the codebase
harder to read for newcomers and makes diffs noisier. If you're translating
or adding comments, English only.

The shell-comments-only-here rule:

- **No apostrophes inside C-style `//` comments in `.l` / `.y` files.**
  Flex/Bison feed the comment text through `m4`, which interprets the
  apostrophe as a quote delimiter and errors out with
  `end of file in string`. Avoid contractions like `don't`, `won't`,
  `it's`, etc., or use full forms instead.

### User-facing strings

YANC supports bilingual diagnostics (PT default, EN via the `-en` CLI
flag). User-facing strings live in `<Component>/Headers/messages.h` and
use the `M(pt, en)` macro plus a named `MSG_*` identifier:

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

The symbol tables (`v_name`, `v_type`, etc. in `variaveis.c` and `l_name`
in `labels.c`) are grow-on-demand: they start small and `realloc` on
overflow, with `memset` to keep the BSS-zero semantics that callers rely
on. If you add a new parallel array, extend the `var_grow` / `lab_grow`
helper so it grows in lockstep with the others.

## Commit message style

The recent history follows the [Conventional Commits] form
(`feat: …`, `fix: …`, `docs: …`, `chore: …`, `ci: …`, `perf: …`,
`refactor: …`). Keep doing that.

Title: short imperative summary, lowercase after the prefix, no trailing
period. Body (optional but recommended for non-trivial changes):
explain the *why*. Wrap body lines at ~72 columns.

[Conventional Commits]: https://www.conventionalcommits.org/
