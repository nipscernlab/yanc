// ----------------------------------------------------------------------------
// CPPComp — compile-time defaults for the YANC target --------------------------
// ----------------------------------------------------------------------------
// These defaults ARE the fixed YANC hardware target: a 32-bit word. The float
// format follows from the width (23 mantissa and 8 exponent bits at 32, the
// sizes of IEEE-754 single but not its encoding; derive_ieee in codegen.c), and
// asmcomp sizes the stacks from the program. C++ sources carry no target
// directive, so the build default is the target. Override via -D only when
// building cppcomp.exe for a DIFFERENT (non-32-bit) target; a source can also
// override per program with `#pragma yanc <name> <value>` at file scope.
// ----------------------------------------------------------------------------

#ifndef CPPCOMP_CONFIG_H
#define CPPCOMP_CONFIG_H

#ifndef CFG_NUBITS
// This is the project's ONE default parameter set (audit 1.7): asmcomp,
// cmmcomp and the HDL now carry the same numbers. It lives here too because
// every C++ test omits the #pragma yanc lines and runs on these.
#define CFG_NUBITS 32
#endif
#ifndef CFG_NUGAIN
#define CFG_NUGAIN 128
#endif
#ifndef CFG_NUIOIN
#define CFG_NUIOIN 1
#endif
#ifndef CFG_NUIOOU
#define CFG_NUIOOU 1
#endif
#include "../../yanc_version.h"   // YANC_VERSION, shared with the driver and the other compilers

#endif
