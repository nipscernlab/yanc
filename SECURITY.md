# Security Policy

YANC is a command-line compiler toolchain (`cmmcomp`, `cppcomp`, `asmcomp`,
`appcomp`, `cpppp`) that turns C±/C++ source into assembly and Verilog for the
SAPHO soft-processor. It runs locally on source you control; it is **not** a
network service. The security-relevant surface is therefore the parsers and code
generators that read source / assembly / array-initialization files: a malformed
or hostile input file could in principle crash a compiler or corrupt memory
instead of producing a clean diagnostic.

## Supported versions

Fixes land on `main` and ship in the next tagged release. Only the latest
release line is supported.

| Version       | Supported |
|---------------|-----------|
| v5.0 / `main` | ✅        |
| < v5.0        | ❌        |

## Reporting a vulnerability

Please report suspected vulnerabilities **privately**, not in a public issue:

- **Preferred:** GitHub private vulnerability reporting — open the repository's
  **Security** tab and click **Report a vulnerability**.
- **Alternatively:** email the maintainers at <luciano.ma.filho@gmail.com>.

Include the smallest input file and the exact command line that triggers the
problem (a stripped-down `.cmm` / `.cpp` / `.asm` is ideal), plus what you
observed (crash, hang, memory corruption, unexpected output).

We aim to acknowledge a report within a few days. YANC is a research / academic
toolchain maintained by a small team, so please allow reasonable time for a fix
before any public disclosure.

## Scope

**In scope:** memory-safety bugs (buffer overflows, NULL dereferences), infinite
loops or hangs on crafted input, and any path that lets an input file cause a
write outside its intended output directory.

**Out of scope:** the behaviour of code you deliberately run *through* the
generated processor, and bugs in the third-party tools YANC merely invokes
(gcc, bison/flex, Icarus Verilog, Verilator, GTKWave).
