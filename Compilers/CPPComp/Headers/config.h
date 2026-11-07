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
// a C++ program without `#pragma yanc` lines runs on these, as almost every
// C++ test does.
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
// the longest name a program may write (issue #66, the same rule as C±'s
// CMM_NAME_MAX). It is a rule of the language, not the size of a buffer:
// every name lives in a block of its own. The longest name cppcomp builds
// from it joins a function and a local (<function>_<name>), about twice
// this, and iverilog takes identifiers of up to 16382 characters.
#define CPP_NAME_MAX 1000

// the most parameters a template may have (issue #129): the instance tables
// (ctinst, the codegen's type arguments) hold this many
#define CPP_TPARAM_MAX 8

// the most dimensions an array may have (#129): a declarator's list holds this many
#define CPP_DIM_MAX 8

#include "../../yanc_version.h"   // YANC_VERSION, shared with the driver and the other compilers

#endif
