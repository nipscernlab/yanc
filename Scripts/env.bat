:: ****************************************************************************
:: Shared environment loader for the runner scripts (single_proc.bat,
:: multi_proc.bat).
::
:: This file lives in <repo>\Scripts\. It resolves the repository root and the
:: locations of the simulation tools (Icarus, Verilator, GTKWave) WITHOUT any
:: hardcoded per-machine paths:
::
::   1. It loads Scripts\tools.local.bat if present -- the cache that
::      Scripts\setup.bat writes after verifying / downloading the tools.
::   2. For anything not in that cache, it falls back to a PATH lookup, so the
::      runners still work if the tools happen to be on PATH already.
::
:: It is meant to be CALLed (not run standalone) and deliberately does NOT use
:: setlocal, so the variables it sets reach the caller:
::
::   ROOT_DIR    repo root           YANC_BIN   <repo>\bin (the prebuilt exes)
::   IVERILOG    iverilog.exe        VVP        vvp.exe
::   VERILATOR   verilator_bin.exe   VERILATOR_ROOT  (Verilator share dir)
::   MINGW_BIN   MSYS2 ucrt64\bin (on PATH for MSYS2-sourced iverilog/verilator)
::   MSYS_USR_BIN  MSYS2 usr\bin, when MINGW_BIN is MSYS2's (make, for Verilator)
::   GTKWAVE     gtkwave.exe         FST2VCD    fst2vcd.exe
::   GTKWAVE_OPTS  the options of the gtkwave call this GTKWave knows (see below)
::   YANC_VL_CFLAGS  what every Verilator call adds as -CFLAGS (see below)
::   YANC_VL_ARGS    the same, as Verilator arguments: -CFLAGS "<flags>"
:: ****************************************************************************

@echo off

:: Repo root = parent of this Scripts\ folder (canonicalised, no trailing slash).
:: Derive it from THIS file's location, not %CD% -- a `pushd .. & set=%CD%` line
:: expands %CD% at parse time (before the pushd), so it would capture the
:: caller's working directory instead. `%%~fI` canonicalises "<Scripts>\..".
for %%I in ("%~dp0..") do set "ROOT_DIR=%%~fI"
set "YANC_BIN=%ROOT_DIR%\bin"

:: Resolved tool paths cached by Scripts\setup.bat (machine-specific, gitignored)
if exist "%~dp0tools.local.bat" call "%~dp0tools.local.bat"

:: Drop stale tool paths inherited from the environment. An IDE/launcher (e.g.
:: Aurora, or the VS Code integrated terminal) may pre-set IVERILOG/VVP/... to
:: bundled tools that are not actually installed on this machine; if the path
:: does not exist, clear it so the PATH lookup below finds a working tool.
if defined IVERILOG  if not exist "%IVERILOG%"  set "IVERILOG="
if defined VVP       if not exist "%VVP%"       set "VVP="
if defined VERILATOR if not exist "%VERILATOR%" set "VERILATOR="
if defined GTKWAVE   if not exist "%GTKWAVE%"   set "GTKWAVE="
if defined FST2VCD   if not exist "%FST2VCD%"   set "FST2VCD="
if defined MINGW_BIN if not exist "%MINGW_BIN%" set "MINGW_BIN="

:: PATH fallbacks for anything the cache did not provide -----------------------
if not defined IVERILOG  for %%E in (iverilog.exe)       do if not "%%~$PATH:E"=="" set "IVERILOG=%%~$PATH:E"
if not defined VVP       for %%E in (vvp.exe)             do if not "%%~$PATH:E"=="" set "VVP=%%~$PATH:E"
if not defined VERILATOR for %%E in (verilator_bin.exe)  do if not "%%~$PATH:E"=="" set "VERILATOR=%%~$PATH:E"
if not defined VERILATOR for %%E in (verilator.exe)      do if not "%%~$PATH:E"=="" set "VERILATOR=%%~$PATH:E"
if not defined GTKWAVE   for %%E in (gtkwave.exe)         do if not "%%~$PATH:E"=="" set "GTKWAVE=%%~$PATH:E"
if not defined FST2VCD   for %%E in (fst2vcd.exe)         do if not "%%~$PATH:E"=="" set "FST2VCD=%%~$PATH:E"

:: MINGW_BIN is the dir holding the MSYS2 tools; callers prepend it to PATH so
:: iverilog/verilator/gtkwave find the DLLs and sub-tools (ivlpp, ivl, ...) they
:: load at runtime. setup.bat caches it in tools.local.bat; if that is absent,
:: derive it from whichever tool resolved above (%%~dpI keeps the trailing \).
if not defined MINGW_BIN if defined IVERILOG  for %%I in ("%IVERILOG%")  do set "MINGW_BIN=%%~dpI"
if not defined MINGW_BIN if defined VVP       for %%I in ("%VVP%")       do set "MINGW_BIN=%%~dpI"
if not defined MINGW_BIN if defined VERILATOR for %%I in ("%VERILATOR%") do set "MINGW_BIN=%%~dpI"
if not defined MINGW_BIN if defined GTKWAVE   for %%I in ("%GTKWAVE%")   do set "MINGW_BIN=%%~dpI"

:: MSYS_USR_BIN: Verilator's --binary runs make, which MSYS2 keeps in usr\bin.
:: The runners put it on PATH after MINGW_BIN, the order of the UCRT64 terminal.
set "MSYS_USR_BIN="
if defined MINGW_BIN for %%I in ("%MINGW_BIN%\..\..\usr\bin") do if exist "%%~fI\make.exe" set "MSYS_USR_BIN=%%~fI"

:: VERILATOR_ROOT: MSYS2's verilator_bin.exe has /ucrt64/share/verilator built
:: in, a path only MSYS2 resolves, so run from cmd it cannot find its include
:: files (verilated_std.sv). setup.bat caches the Windows path; without the
:: cache, derive it from the tool's folder (<prefix>\bin -> <prefix>\share\verilator).
if defined VERILATOR_ROOT if not exist "%VERILATOR_ROOT%" set "VERILATOR_ROOT="
if not defined VERILATOR_ROOT if defined VERILATOR for %%I in ("%VERILATOR%") do if exist "%%~dpI..\share\verilator" for %%R in ("%%~dpI..\share\verilator") do set "VERILATOR_ROOT=%%~fR"

:: GTKWAVE_OPTS: the options of the runners' gtkwave call that this GTKWave
:: lists in its --help. The nipscernlab build has --zoom-fit and
:: --left-justify; MSYS2's and the distributions' GTKWave refuse an option they
:: do not know and open nothing, so each one goes only where it is listed.
set "GTKWAVE_OPTS="
set "YANC_GW_HELP=%TEMP%\yanc_gtkwave_help.txt"
if exist "%YANC_GW_HELP%" del "%YANC_GW_HELP%"
if defined GTKWAVE "%GTKWAVE%" --help > "%YANC_GW_HELP%" 2>&1
if exist "%YANC_GW_HELP%" findstr /c:"--dark" "%YANC_GW_HELP%" >nul && set "GTKWAVE_OPTS=%GTKWAVE_OPTS% --dark"
if exist "%YANC_GW_HELP%" findstr /c:"--zoom-fit" "%YANC_GW_HELP%" >nul && set "GTKWAVE_OPTS=%GTKWAVE_OPTS% --zoom-fit"
if exist "%YANC_GW_HELP%" findstr /c:"--left-justify" "%YANC_GW_HELP%" >nul && set "GTKWAVE_OPTS=%GTKWAVE_OPTS% --left-justify"
if exist "%YANC_GW_HELP%" del "%YANC_GW_HELP%"
set "YANC_GW_HELP="

:: YANC_VL_CFLAGS: MSYS2's gcc 16 (16.1 and 16.2) ships a libstdc++ without the
:: std::string move constructor at -Os, which Verilator's build uses. With
:: _GLIBCXX_ASSERTIONS libstdc++ drops its extern template for basic_string, so
:: the compiler instantiates the constructor itself (#117), as in Scripts\env.sh.
:: A value already set is kept; cmd cannot hold an empty variable, so
:: YANC_VL_CFLAGS=none passes nothing.
if not defined YANC_VL_CFLAGS set "YANC_VL_CFLAGS=-D_GLIBCXX_ASSERTIONS"
set "YANC_VL_ARGS="
if /i not "%YANC_VL_CFLAGS%"=="none" set YANC_VL_ARGS=-CFLAGS "%YANC_VL_CFLAGS%"
