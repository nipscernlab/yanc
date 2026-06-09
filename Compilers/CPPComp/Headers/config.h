// ----------------------------------------------------------------------------
// CPPComp — compile-time defaults for the YANC target --------------------------
// ----------------------------------------------------------------------------
// These defaults ARE the fixed YANC hardware target (32-bit word, IEEE-style
// mant23/exp8, 128-deep data/return stacks) — C++ sources carry no target
// directive, so the build default is the target. Override via -D only when
// building cppcomp.exe for a DIFFERENT (non-32-bit) target; a .c source can also
// override per-program with `#pragma yanc <name> <value>` at file scope.
// ----------------------------------------------------------------------------

#ifndef CPPCOMP_CONFIG_H
#define CPPCOMP_CONFIG_H

#ifndef CFG_NUBITS
#define CFG_NUBITS 32
#endif
#ifndef CFG_NBMANT
#define CFG_NBMANT 23
#endif
#ifndef CFG_NBEXPO
#define CFG_NBEXPO 8
#endif
#ifndef CFG_NUGAIN
#define CFG_NUGAIN 128
#endif
#ifndef CFG_NDSTAC
#define CFG_NDSTAC 128
#endif
#ifndef CFG_SDEPTH
#define CFG_SDEPTH 128
#endif
#ifndef CFG_NUIOIN
#define CFG_NUIOIN 1
#endif
#ifndef CFG_NUIOOU
#define CFG_NUIOOU 1
#endif
#ifndef CFG_FFTSIZ
#define CFG_FFTSIZ 3
#endif

#include "../../yanc_version.h"   // YANC_VERSION shared across all four compilers

#endif
