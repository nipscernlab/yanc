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
::   VERILATOR   verilator_bin.exe   VERILATOR_ROOT / VL_MINGW_BIN  (Verilator)
::   GTKWAVE     gtkwave.exe         FST2VCD    fst2vcd.exe
:: ****************************************************************************

@echo off

:: Repo root = parent of this Scripts\ folder (canonicalised, no trailing slash)
pushd "%~dp0.." & set "ROOT_DIR=%CD%" & popd
set "YANC_BIN=%ROOT_DIR%\bin"

:: Resolved tool paths cached by Scripts\setup.bat (machine-specific, gitignored)
if exist "%~dp0tools.local.bat" call "%~dp0tools.local.bat"

:: PATH fallbacks for anything the cache did not provide -----------------------
if not defined IVERILOG  for %%E in (iverilog.exe)       do if not "%%~$PATH:E"=="" set "IVERILOG=%%~$PATH:E"
if not defined VVP       for %%E in (vvp.exe)             do if not "%%~$PATH:E"=="" set "VVP=%%~$PATH:E"
if not defined VERILATOR for %%E in (verilator_bin.exe)  do if not "%%~$PATH:E"=="" set "VERILATOR=%%~$PATH:E"
if not defined VERILATOR for %%E in (verilator.exe)      do if not "%%~$PATH:E"=="" set "VERILATOR=%%~$PATH:E"
if not defined GTKWAVE   for %%E in (gtkwave.exe)         do if not "%%~$PATH:E"=="" set "GTKWAVE=%%~$PATH:E"
if not defined FST2VCD   for %%E in (fst2vcd.exe)         do if not "%%~$PATH:E"=="" set "FST2VCD=%%~$PATH:E"
