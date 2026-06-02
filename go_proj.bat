:: ****************************************************************************
:: Emulate SAPHO when compiling a multi-processor project and simulating it.
::   go_proj.bat                  -> simulate with Icarus (default)
::   go_proj.bat --sim verilator  -> simulate with Verilator (+define+YANC_TRACE)
::
:: --sim verilator feeds the same .v set to Verilator 5 (--binary --timing
:: --trace-fst) with top_level_tb.v as the top; +define+YANC_TRACE keeps each
:: processor's user variables / arrays in the waveform. Run Scripts\setup.bat
:: once first.
:: ****************************************************************************

:: Set up the terminal --------------------------------------------------------

cls
echo off
chcp 65001 >nul

:: Set up the environment -----------------------------------------------------

:: Resolve ROOT_DIR + the prebuilt binaries (YANC_BIN) and the tool locations
:: (IVERILOG, VVP, VERILATOR, GTKWAVE, FST2VCD) with no hardcoded paths.
:: Scripts\setup.bat builds / downloads everything once and caches the paths;
:: env.bat loads them here.
call "%~dp0Scripts\env.bat"
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
    echo [go_proj] YANC binaries missing in "%YANC_BIN%".
    echo            Run  Scripts\setup.bat  once to build or download them.
    exit /b 1
)
if not defined GTKWAVE (
    echo [go_proj] GTKWave not found - run Scripts\setup.bat to fetch the
    echo            nipscernlab GTKWave build, then re-run.
    exit /b 1
)
if /i "%SIM%"=="verilator" (
    if not defined VERILATOR (
        echo [go_proj] Verilator not found - run Scripts\setup.bat, or install it
        echo            with "pacman -S mingw-w64-x86_64-verilator" and re-run.
        exit /b 1
    )
    if not defined FST2VCD (
        echo [go_proj] fst2vcd not found - it ships with the nipscernlab GTKWave
        echo            bundle. Re-run Scripts\setup.bat to fetch it.
        exit /b 1
    )
) else (
    if not defined IVERILOG (
        echo [go_proj] Icarus Verilog not found - run Scripts\setup.bat, or install
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

set    TESTE_DIR=%ROOT_DIR%\Teste
rmdir %TESTE_DIR% /s /q

:: Parameters defined by the SAPHO user for compilation -----------------------

:: project folder name
set PROJET=DTW
:: list of processor types in the project (names of the project subfolders)
set PROC_LIST=ProcDTW ZeroCross
:: list of instances to simulate (a single proc may have several instances)
set INST_LIST=ZeroCross_inst DTWv4_inst
:: list of processor types for each instance (must match the length of PROC_LIST)
set PROC_TYPE=ZeroCross ProcDTW
:: testbench name (without .v) to simulate (must be in the TopLevel folder)
set TB=top_level_tb
:: gtkwave layout filename (if not found, uses the default script)
set GTKW=dtw.gtkw

:: Parameters that SAPHO must know --------------------------------------------

:: folder tree after installation
set INST_DIR=%TESTE_DIR%\saphoComponents
set BIN_DIR=%INST_DIR%\bin
set HDL_DIR=%INST_DIR%\HDL
set MAC_DIR=%INST_DIR%\Macros
set SCR_DIR=%INST_DIR%\Scripts
set TMP_DIR=%INST_DIR%\Temp

:: project folder tree being executed
set USER_DIR=%TESTE_DIR%\Projetos
set PROJ_DIR=%USER_DIR%\%PROJET%
set TOPL_DIR=%PROJ_DIR%\TopLevel

:: Verilator obj_dir (generated C++ model + the V<tb> sim exe)
set VL_DIR=%TMP_DIR%\vl

:: Create test directories ----------------------------------------------------

mkdir %TESTE_DIR%
    mkdir %INST_DIR%
        mkdir %BIN_DIR%
        mkdir %HDL_DIR%
        mkdir %MAC_DIR%
        mkdir %SCR_DIR%
        mkdir %TMP_DIR%
    mkdir %USER_DIR%

(for %%i in (%PROC_LIST%) do (
    mkdir %TMP_DIR%\%%i
))

:: Point TMP/TEMP at this project's Temp dir. Without this, go_proj inherits
:: whatever TMP a previous run left in the cmd session, which its own rmdir then
:: deletes -- leaving gcc / iverilog with no temp dir.
set TMP=%TMP_DIR%
set TEMP=%TMP_DIR%

:: Copy files into the test directories ---------------------------------------

xcopy Compilers\CMMComp\Tests %USER_DIR% /e /i /q>%TMP_DIR%\xcopy.txt
xcopy HDL %HDL_DIR% /q /y>%TMP_DIR%\xcopy.txt
xcopy Compilers\CMMComp\Includes %MAC_DIR% /q /y>%TMP_DIR%\xcopy.txt
xcopy Scripts %SCR_DIR% /q /y>%TMP_DIR%\xcopy.txt

:: Stage the prebuilt YANC binaries -------------------------------------------
:: No bison/flex/gcc here: cmmcomp/appcomp/asmcomp and the GTKWave helpers
:: (comp2gtkw, gen_gtkw) were already built or downloaded into %YANC_BIN% by
:: Scripts\setup.bat. Copy them into the sandbox bin the rest of the flow uses.

copy %YANC_BIN%\*.exe %BIN_DIR%>%TMP_DIR%\xcopy.txt

:: Run the CMM compiler -------------------------------------------------------

cd  %BIN_DIR%

(for %%i in (%PROC_LIST%) do (
    cmmcomp.exe -i %%i.cmm -n %%i -p %USER_DIR%\%%i -m %MAC_DIR% -t %TMP_DIR%\%%i --array
))

:: Run the Assembler pre-processor --------------------------------------------

(for %%i in (%PROC_LIST%) do (
    appcomp.exe -i %USER_DIR%\%%i\Software\%%i.asm -t %TMP_DIR%\%%i
))

:: Run the Assembler compiler -------------------------------------------------

(for %%i in (%PROC_LIST%) do (
    asmcomp.exe -i %USER_DIR%\%%i\Software\%%i.asm -p %USER_DIR%\%%i -d %HDL_DIR% -m %MAC_DIR% -t %TMP_DIR%\%%i -f 0 -c 0
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
for %%a in (%PROC_LIST%) do copy %TMP_DIR%\%%a\%%a_tb.v %USER_DIR%\%%a\Simulation>%TMP_DIR%\xcopy.txt
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

:: Run GtkWave ----------------------------------------------------------------
:proj_gtkwave
echo #### Generating the .gtkw layout and launching GTKWave
:: gen_gtkw reads the header VCD and writes the multi-proc .gtkw (one section per
:: instance owning valr2+linetabs); GTKWave opens the waveform with -a.
if exist %TOPL_DIR%\%GTKW% (
    %GTKWAVE% --dark --zoom-fit --left-justify %TMP_DIR%\%TB%.vcd -a %TOPL_DIR%\%GTKW%
) else (
    %BIN_DIR%\gen_gtkw.exe %TMP_DIR%\%TB%_hdr.vcd %TMP_DIR%\%TB%.gtkw %TMP_DIR% %BIN_DIR%\comp2gtkw.exe
    %GTKWAVE% --dark --zoom-fit --left-justify %TMP_DIR%\%TB%.vcd -a %TMP_DIR%\%TB%.gtkw
)

cd %ROOT_DIR%
exit /b 0

:usage
echo usage: go_proj.bat [--sim iverilog^|verilator]
exit /b 0

:badsim
echo [go_proj] --sim must be 'iverilog' or 'verilator' (got "%SIM%")
exit /b 1
