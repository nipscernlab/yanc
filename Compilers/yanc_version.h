// ----------------------------------------------------------------------------
// Single source of truth for the YANC toolchain version. All five binaries
// (compilers: cmmcomp, cppcomp, asmcomp -- preprocessors: cpppp, appcomp)
// include this header so that bumping the version in one place updates the
// --version output everywhere.
// ----------------------------------------------------------------------------

#ifndef YANC_VERSION_H
#define YANC_VERSION_H

#define YANC_VERSION "5.2"

#endif
