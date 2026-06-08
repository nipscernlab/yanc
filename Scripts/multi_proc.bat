:: ****************************************************************************
:: multi_proc - end-to-end example of the YANC multi-processor pipeline.
::
:: It shows the pipeline of a top-level circuit that uses two SAPHO processors
:: internally, taking each processor's C+- (.cmm) program through the compile
:: pipeline to its synthesizable Verilog (.v):
::
::   * ZeroCross - FIR filters that smooth the 60 Hz mains (power-grid)
::                 waveform and compute its zero crossings.
::   * ProcDTW   - a DTW transform (2-D array) synchronized to the zero
::                 crossings produced by ZeroCross, doing novelty detection to
::                 spot disturbances on the power grid.
::
:: The circuit is several Verilog files -- the top level plus one generated
:: <proc>.v per processor -- and this project has been tested on an FPGA. The
:: GTKWave view shows the top-level signals alongside the software execution of
:: both processors in lockstep.
::
:: Because the processors run together under a top level, the testbench here is
:: the user-written TopLevel\top_level_tb.v. The asm-generated testbench can
:: only drive one processor in isolation -- that is the single_proc flow.
::
::   multi_proc.bat                  -> simulate with Icarus (default)
::   multi_proc.bat --sim verilator  -> simulate with Verilator (+define+YANC_TRACE)
::
:: Reads the HDL, macros and binaries straight from the repo; the only files it
:: creates live under Teste\ (gitignored). Run Scripts\setup.bat once first.
:: ****************************************************************************

:: Set up the terminal --------------------------------------------------------

cls
echo off
chcp 65001 >nul

:: Set up the environment -----------------------------------------------------

call "%~dp0env.bat"
cd /d "%ROOT_DIR%"

:: Pick the simulator (--sim iverilog|verilator, default iverilog) ------------
set "SIM=iverilog"
if /i "%~1"=="--sim" set "SIM=%~2"
if /i "%~1"=="-h" goto :usage
if /i "%~1"=="--help" goto :usage
if /i "%SIM%"=="vl" set "SIM=verilator"
if /i "%SIM%"=="icarus" set "SIM=iverilog"
if /i not "%SIM%"=="iverilog" if /i not "%SIM%"=="verilator" goto :badsim

:: Tools: binaries + GTKWave always, plus the chosen simulator ---------------
if not exist "%YANC_BIN%\cmmcomp.exe" (
    echo [multi_proc] YANC binaries missing in "%YANC_BIN%".
    echo            Run  Scripts\setup.bat  once to build or download them.
    exit /b 1
)
if not defined GTKWAVE (
    echo [multi_proc] GTKWave not found - run Scripts\setup.bat to fetch the
    echo            nipscernlab GTKWave build, then re-run.
    exit /b 1
)
if /i "%SIM%"=="verilator" (
    if not defined VERILATOR (
        echo [multi_proc] Verilator not found - run Scripts\setup.bat, or install it
        echo            with "pacman -S mingw-w64-x86_64-verilator" and re-run.
        exit /b 1
    )
    if not defined FST2VCD (
        echo [multi_proc] fst2vcd not found - it ships with the nipscernlab GTKWave
        echo            bundle. Re-run Scripts\setup.bat to fetch it.
        exit /b 1
    )
) else (
    if not defined IVERILOG (
        echo [multi_proc] Icarus Verilog not found - run Scripts\setup.bat, or install
        echo            it from https://bleyer.org/icarus/ and re-run.
        exit /b 1
    )
)

:: When iverilog/verilator come from MSYS2, their tools need mingw64\bin on PATH.
if defined MINGW_BIN set "PATH=%MINGW_BIN%;%PATH%"

:: Verilator warning suppressions: the design isn't lint-clean and the project
:: mixes timescale'd top-level modules with non-timescale'd HDL; none of these
:: affect the simulation, so silence them (Verilator escalates them otherwise).
set VL_WARN=-Wno-lint -Wno-MULTIDRIVEN -Wno-BLKANDNBLK -Wno-WIDTH -Wno-CASEINCOMPLETE -Wno-IMPLICIT -Wno-COMBDLY -Wno-STMTDLY -Wno-INFINITELOOP -Wno-UNOPTFLAT -Wno-PINMISSING -Wno-SELRANGE -Wno-TIMESCALEMOD -Wno-INITIALDLY

:: What to simulate (a project under Compilers\CMMComp\Tests) -----------------

:: project folder (holds TopLevel\ with the tb)
set PROJET=DTW
:: processor types in the project (each a Tests\ subfolder we compile)
set PROC_LIST=ProcDTW ZeroCross
:: top testbench name (without .v) to simulate, in the TopLevel folder
set TB=top_level_tb

:: Repo sources, read in place (never written to) -----------------------------
set HDL_DIR=%ROOT_DIR%\HDL
set MAC_DIR=%ROOT_DIR%\Compilers\CMMComp\Includes
set BIN_DIR=%YANC_BIN%
set TESTS_DIR=%ROOT_DIR%\Compilers\CMMComp\Tests

:: Work area: the only files this script creates (Teste\ is gitignored) -------
set WORK=%ROOT_DIR%\Teste
set USER_DIR=%WORK%\Projetos
set TMP_DIR=%WORK%\Temp
set PROJ_DIR=%USER_DIR%\%PROJET%
set TOPL_DIR=%PROJ_DIR%\TopLevel
set VL_DIR=%TMP_DIR%\vl

:: Only the inputs are copied: the project's TopLevel\ (user testbench, top-level
:: Verilog and stimulus) and each processor's .cmm. The compilers create the
:: Software/Hardware outputs in the writable copies.
rmdir %WORK% /s /q
mkdir %TMP_DIR%
mkdir %TOPL_DIR%
xcopy "%TESTS_DIR%\%PROJET%\TopLevel" "%TOPL_DIR%\" /e /i /q /y>%TMP_DIR%\xcopy.txt
(for %%i in (%PROC_LIST%) do (
    mkdir %USER_DIR%\%%i\Software
    mkdir %TMP_DIR%\%%i
    copy "%TESTS_DIR%\%%i\Software\%%i.cmm" "%USER_DIR%\%%i\Software\">%TMP_DIR%\xcopy.txt
))

:: Verilator's g++ wants a writable TMP for its intermediate files
set TMP=%TMP_DIR%
set TEMP=%TMP_DIR%

:: Compile each processor: C+- -> asm -> Verilog -----------------------------

(for %%i in (%PROC_LIST%) do (
    "%BIN_DIR%\cmmcomp.exe" -i %%i.cmm -n %%i -p %USER_DIR%\%%i -m %MAC_DIR% -t %TMP_DIR%\%%i --array
))
(for %%i in (%PROC_LIST%) do (
    "%BIN_DIR%\appcomp.exe" -i %USER_DIR%\%%i\Software\%%i.asm -t %TMP_DIR%\%%i
))
(for %%i in (%PROC_LIST%) do (
    "%BIN_DIR%\asmcomp.exe" -i %USER_DIR%\%%i\Software\%%i.asm -p %USER_DIR%\%%i -d %HDL_DIR% -m %MAC_DIR% -t %TMP_DIR%\%%i -f 0 -c 0
    cp %USER_DIR%\%%i\Hardware\%%i.v %TMP_DIR%\%%i
))

:: Build the simulation (gather the same .v set for both simulators) ----------

cd %TMP_DIR%

setlocal enabledelayedexpansion

:: list HDL folder files
dir %HDL_DIR%\*.v /b > f_list.txt
for /f "delims=" %%a in (%TMP_DIR%\f_list.txt) do set "HDL_V=!HDL_V!%HDL_DIR%\%%a "

:: list TopLevel folder files
dir %TOPL_DIR%\*.v /b > f_list.txt
for /f "delims=" %%a in (%TMP_DIR%\f_list.txt) do set "TOP_V=!TOP_V!%TOPL_DIR%\%%a "

:: list files of the processors found
for %%a in (%PROC_LIST%) do set "PRO_V=!PRO_V!%TMP_DIR%\%%a\%%a.v "

if /i "%SIM%"=="verilator" goto :proj_build_vl

echo #### Running Icarus
%IVERILOG% -s %TB% -o %TMP_DIR%\%PROJET%.vvp !HDL_V! !PRO_V! !TOP_V!
endlocal
goto :proj_run_icarus

:proj_build_vl
echo #### Running Verilator (+define+YANC_TRACE, --binary --timing --trace-fst)
%VERILATOR% --binary --timing --trace-fst +define+YANC_TRACE --top-module %TB% %VL_WARN% --Mdir %VL_DIR% !HDL_V! !PRO_V! !TOP_V!
endlocal
goto :proj_run_vl

:: --- Icarus: stage the files vvp reads (relative to CWD), then run ----------
:proj_run_icarus
dir %TOPL_DIR%\*.txt /b > f_list.txt
for /f "delims=" %%a in (%TMP_DIR%\f_list.txt) do copy %TOPL_DIR%\%%a .\>%TMP_DIR%\xcopy.txt
for %%a in (%PROC_LIST%) do copy %USER_DIR%\%%a\Hardware\%%a_inst.mif .\>%TMP_DIR%\xcopy.txt
for %%a in (%PROC_LIST%) do copy %USER_DIR%\%%a\Hardware\%%a_data.mif .\>%TMP_DIR%\xcopy.txt
for %%a in (%PROC_LIST%) do copy %TMP_DIR%\%%a\pc_%%a_mem.txt .\>%TMP_DIR%\xcopy.txt
del f_list.txt
del xcopy.txt
echo #### Running VVP
:: header-only pass (tiny text VCD) then the real FST run (+WAVE arms $dumpvars)
%VVP% %PROJET%.vvp +HEADER_ONLY
copy %TB%.vcd %TB%_hdr.vcd>%TMP_DIR%\xcopy.txt
%VVP% %PROJET%.vvp -fst +WAVE
goto :proj_gtkwave

:: --- Verilator: stage the files the run reads, then run ---------------------
:proj_run_vl
dir %TOPL_DIR%\*.txt /b > f_list.txt
for /f "delims=" %%a in (%TMP_DIR%\f_list.txt) do copy %TOPL_DIR%\%%a .\>%TMP_DIR%\xcopy.txt
for %%a in (%PROC_LIST%) do copy %TMP_DIR%\%%a\pc_%%a_mem.txt .\>%TMP_DIR%\xcopy.txt
del f_list.txt
del xcopy.txt
echo #### Running the Verilator simulation
:: header-only pass -> tiny FST; fst2vcd extracts a text VCD for the formatter.
%VL_DIR%\V%TB%.exe +HEADER_ONLY
%FST2VCD% -f %TB%.vcd -o %TB%_hdr.vcd>%TMP_DIR%\xcopy.txt 2>&1
:: +WAVE arms the tb's $dumpvars (off by default so the heavy sim doesn't crash).
%VL_DIR%\V%TB%.exe +WAVE

:: View in GTKWave (layout built by gen_gtkw) ---------------------------------
:proj_gtkwave
echo #### Generating the .gtkw layout (gen_gtkw) and launching GTKWave
%BIN_DIR%\gen_gtkw.exe %TMP_DIR%\%TB%_hdr.vcd %TMP_DIR%\%TB%.gtkw %TMP_DIR% %BIN_DIR%\comp2gtkw.exe
%GTKWAVE% --dark --zoom-fit --left-justify %TMP_DIR%\%TB%.vcd -a %TMP_DIR%\%TB%.gtkw

cd %ROOT_DIR%
exit /b 0

:usage
echo usage: multi_proc.bat [--sim iverilog^|verilator]
exit /b 0

:badsim
echo [multi_proc] --sim must be 'iverilog' or 'verilator' (got "%SIM%")
exit /b 1
