:: ****************************************************************************
:: Shared environment loader for the go_*.bat scripts.
::
:: This file lives in <repo>\Scripts\. It resolves the repository root and the
:: locations of the simulation tools (Icarus, Verilator, GTKWave) WITHOUT any
:: hardcoded per-machine paths:
::
::   1. It loads Scripts\tools.local.bat if present -- the cache that
::      Scripts\setup.bat writes after verifying / downloading the tools.
::   2. For anything not in that cache, it falls back to a PATH lookup, so the
::      go_*.bat still work if the tools happen to be on PATH already.
::
:: It is meant to be CALLed (not run standalone) and deliberately does NOT use
:: setlocal, so the variables it sets reach the caller:
::
::   ROOT_DIR    repo root           YANC_BIN   <repo>\bin (the prebuilt exes)
::   IVERILOG    iverilog.exe        VVP        vvp.exe
::   VERILATOR   verilator_bin.exe   VERILATOR_ROOT  (Verilator share dir)
::   MINGW_BIN   MSYS2 mingw64\bin (on PATH for MSYS2-sourced iverilog/verilator)
::   GTKWAVE     gtkwave.exe         FST2VCD    fst2vcd.exe
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
