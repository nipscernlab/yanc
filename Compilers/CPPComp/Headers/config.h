// ----------------------------------------------------------------------------
// CPPComp — compile-time defaults for the YANC target --------------------------
// ----------------------------------------------------------------------------
// Override via -D when building cppcomp.exe. These become the default header
// directives emitted in every .asm. A user .c source can still override any
// of them per-program with `#pragma yanc <name> <value>` at file scope.
// ----------------------------------------------------------------------------

#ifndef CPPCOMP_CONFIG_H
#define CPPCOMP_CONFIG_H

#ifndef CFG_NUBITS
#define CFG_NUBITS 16
#endif
#ifndef CFG_NBMANT
#define CFG_NBMANT 10
#endif
#ifndef CFG_NBEXPO
#define CFG_NBEXPO 5
#endif
#ifndef CFG_NUGAIN
#define CFG_NUGAIN 128
#endif
#ifndef CFG_NDSTAC
#define CFG_NDSTAC 8
#endif
#ifndef CFG_SDEPTH
#define CFG_SDEPTH 8
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
