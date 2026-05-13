# Changelog

All notable changes to YANC are documented here.

The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and the project adheres to a loose semantic-versioning scheme on the `v*`
tags consumed by Aurora.

## [Unreleased]

### Added
- MIT `LICENSE` file (was previously "all rights reserved" by default).
- `CONTRIBUTING.md` documenting code, commit, and bilingual-message
  conventions.
- Per-push CI workflow (`.github/workflows/ci.yml`) that builds all four
  binaries (`cmmcomp`, `appcomp`, `asmcomp`, `comp2gtkw`) with
  `-O2 -Wall` on Windows + MSYS2, then smoke-tests the full
  `cmmcomp → appcomp → asmcomp` pipeline against every example in
  `Exemplos/`.
- `MSG_ERR_OUT_OF_MEMORY` bilingual diagnostic (replaces
  `MSG_ERR_TOO_MANY_VARS` / `MSG_ERR_TOO_MANY_LABELS`).

### Changed
- Symbol and label tables in all three compilers are now grow-on-demand
  via `realloc` (starting at 256 entries, 128 for the label-nesting
  stack), instead of fixed `NVARMAX=999999` / `NLABMAX=99999` BSS
  arrays. Per-process resident memory drops from ~500 MB of zeroed BSS
  to ~128 KB initial.
- All in-source comments translated from Portuguese to English across
  `APP/`, `ASM/`, `CMMComp/`, `HDL/`, `Scripts/`, `Macros/`, the build
  `.bat` files, and the runnable examples in `Exemplos/`.
- README expanded with pipeline diagram, component table, build
  instructions, CLI usage, a runnable CMM example, and project layout.
- `release.yml` inline comments translated to English.
- `.gitignore` comments translated to English; added `/.smoke/` and
  `.vscode/`.

### Removed
- `.vscode/c_cpp_properties.json` is no longer tracked (IDE-specific
  config with hard-coded MSYS2 paths from the original machine).

## [v2] – 2026-05-13

### Added
- Release workflow now bundles `HDL/` and `Macros/` into the release zip
  alongside `bin/`, so Aurora can extract `components/{bin,HDL,Macros}/`
  in one shot.

## [v1] – 2026-05-13

### Added
- Initial release artifact: tag-driven GitHub Actions workflow that
  builds `cmmcomp`, `appcomp`, `asmcomp`, and `comp2gtkw` with MSYS2 +
  MinGW-w64, packages them in `yanc-bin-vN.zip`, and publishes the zip
  as a release asset.

[Unreleased]: https://github.com/nipscernlab/yanc/compare/v2...HEAD
[v2]: https://github.com/nipscernlab/yanc/releases/tag/v2
[v1]: https://github.com/nipscernlab/yanc/releases/tag/v1
