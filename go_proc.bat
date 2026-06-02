:: ****************************************************************************
:: Emulate SAPHO when compiling a single processor and simulating it.
::   go_proc.bat                  -> simulate with Icarus (default)
::   go_proc.bat --sim verilator  -> simulate with Verilator (+define+YANC_TRACE)
::
:: --sim verilator feeds the SAME generated <proc>_tb.v to Verilator 5 in
:: --binary --timing mode; +define+YANC_TRACE turns on the visibility harness
:: inside <proc>.v so the waveform shows all the user variables / arrays / PC.
:: Run Scripts\setup.bat once first.
:: ****************************************************************************

:: Set up the terminal --------------------------------------------------------

cls
echo off
chcp 65001 >nul

:: Set up the environment -----------------------------------------------------

:: Resolve ROOT_DIR + the prebuilt binaries (YANC_BIN) and the tool locations
:: (IVERILOG, VVP, VERILATOR, GTKWAVE) with no hardcoded paths. Scripts\setup.bat
:: builds / downloads everything once and caches the paths; env.bat loads them.
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
    echo [go_proc] YANC binaries missing in "%YANC_BIN%".
    echo            Run  Scripts\setup.bat  once to build or download them.
    exit /b 1
)
if not defined GTKWAVE (
    echo [go_proc] GTKWave not found - run Scripts\setup.bat to fetch the
    echo            nipscernlab GTKWave build, then re-run.
    exit /b 1
)
if /i "%SIM%"=="verilator" (
    if not defined VERILATOR (
        echo [go_proc] Verilator not found - run Scripts\setup.bat, or install it
        echo            with "pacman -S mingw-w64-x86_64-verilator" and re-run.
        exit /b 1
    )
) else (
    if not defined IVERILOG (
        echo [go_proc] Icarus Verilog not found - run Scripts\setup.bat, or install
        echo            it from https://bleyer.org/icarus/ and re-run.
        exit /b 1
    )
)

:: When iverilog/verilator come from MSYS2, their tools need mingw64\bin on PATH.
if defined MINGW_BIN set "PATH=%MINGW_BIN%;%PATH%"

set    TESTE_DIR=%ROOT_DIR%\Teste
rmdir %TESTE_DIR% /s /q

:: Parameters defined by the SAPHO user for compilation -----------------------

:: project folder name
set PROJET=FFT
:: processor type name to simulate (a subfolder of the project)
set PROC=proc_fft
:: cmm filename where the processor is defined
set FNAM=proc_fft.cmm
:: test_bench (without .v) to simulate, in the Simulation folder (Icarus only);
:: if not found, the generated <proc>_tb is used
set TB=errado
:: gtkwave layout filename (if not found, uses the default script)
set GTKW=teste.gtkw
:: processor operating frequency in MHz
set FRE_CLK=100
:: number of clocks to simulate
set NUM_CLK=1000000

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
set PROC_DIR=%USER_DIR%\%PROC%
set SOFT_DIR=%PROC_DIR%\Software
set HARD_DIR=%PROC_DIR%\Hardware
set SIMU_DIR=%PROC_DIR%\Simulation
set TMP_PRO=%TMP_DIR%\%PROC%

:: Verilator obj_dir (generated C++ model + the V<proc>_tb sim exe)
set VL_DIR=%TMP_PRO%\vl

:: Create test directories ----------------------------------------------------

mkdir %TESTE_DIR%
    mkdir %INST_DIR%
        mkdir %BIN_DIR%
        mkdir %HDL_DIR%
        mkdir %MAC_DIR%
        mkdir %SCR_DIR%
        mkdir %TMP_DIR%
            mkdir %TMP_PRO%
    mkdir %USER_DIR%

:: Verilator's g++ wants a writable TMP for its intermediate files
set TMP=%TMP_PRO%
set TEMP=%TMP_PRO%

:: Copy files into the test directories ---------------------------------------

xcopy Compilers\CMMComp\Tests %USER_DIR% /e /i /q>%TMP_PRO%\log.txt
xcopy HDL      %HDL_DIR%  /q    /y>%TMP_PRO%\log.txt
xcopy Compilers\CMMComp\Includes %MAC_DIR% /q /y>%TMP_PRO%\log.txt
xcopy Scripts  %SCR_DIR%  /q    /y>%TMP_PRO%\log.txt

:: Stage the prebuilt YANC binaries -------------------------------------------
:: No bison/flex/gcc here: cmmcomp/appcomp/asmcomp and the GTKWave helpers
:: (comp2gtkw, gen_gtkw) were already built or downloaded into %YANC_BIN% by
:: Scripts\setup.bat. Copy them into the sandbox bin the rest of the flow uses.

copy %YANC_BIN%\*.exe %BIN_DIR%>%TMP_PRO%\log.txt

:: Run the CMM compiler -------------------------------------------------------

echo #### Running the CMM compiler

cd %BIN_DIR%

cmmcomp.exe -i %FNAM% -n %PROC% -p %PROC_DIR% -m %MAC_DIR% -t %TMP_PRO%

:: Run the Assembler pre-processor --------------------------------------------

echo #### Running the Pre-assembler

set ASM_FILE=%SOFT_DIR%\%PROC%.asm

appcomp.exe -i %ASM_FILE% -t %TMP_PRO%

:: Run the Assembler compiler -------------------------------------------------

echo #### Running the Assembler

asmcomp.exe -i %ASM_FILE% -p %PROC_DIR% -d %HDL_DIR% -m %MAC_DIR% -t %TMP_PRO% -f %FRE_CLK% -c %NUM_CLK%

:: Build + run the simulation -------------------------------------------------

set UPROC=%HARD_DIR%\%PROC%
if /i "%SIM%"=="verilator" goto :sim_verilator

:: --- Icarus -----------------------------------------------------------------
echo #### Running Icarus
cd %HDL_DIR%
if exist %SIMU_DIR%\%TB%.v (
    set TB_MOD=%TB%
) else (
    copy %TMP_PRO%\%PROC%_tb.v %SIMU_DIR%>%TMP_PRO%\log.txt
    set TB_MOD=%PROC%_tb
)
%IVERILOG% -s %TB_MOD% -o %TMP_PRO%\%PROC%.vvp %SIMU_DIR%\%TB_MOD%.v %UPROC%.v addr_dec.v instr_dec.v processor.v core.v ula.v

echo #### Running VVP
copy %UPROC%_data.mif %TMP_PRO%>%TMP_PRO%\log.txt
copy %UPROC%_inst.mif %TMP_PRO%>%TMP_PRO%\log.txt
cd %TMP_PRO%
:: header-only pass (no -fst -> text VCD): gives gen_gtkw the signal list fast.
%VVP% %PROC%.vvp +HEADER_ONLY
copy %TB_MOD%.vcd %TB_MOD%_hdr.vcd>%TMP_PRO%\log.txt
:: real run -> the FST waveform GTKWave opens
%VVP% %PROC%.vvp -fst
set WAVE=%TMP_PRO%\%TB_MOD%.vcd
set HDR=%TMP_PRO%\%TB_MOD%_hdr.vcd
set GTKWOUT=%TMP_PRO%\%TB_MOD%.gtkw
goto :run_gtkwave

:: --- Verilator --------------------------------------------------------------
:sim_verilator
echo #### Running Verilator (+define+YANC_TRACE, --binary --timing --trace)
%VERILATOR% --binary --timing --trace +define+YANC_TRACE ^
    --top-module %PROC%_tb ^
    -Wno-lint -Wno-UNOPTFLAT -Wno-MULTIDRIVEN -Wno-BLKANDNBLK -Wno-WIDTH ^
    -Wno-CASEINCOMPLETE -Wno-IMPLICIT -Wno-COMBDLY -Wno-STMTDLY -Wno-INFINITELOOP ^
    --Mdir %VL_DIR% ^
    %TMP_PRO%\%PROC%_tb.v %UPROC%.v ^
    %HDL_DIR%\processor.v %HDL_DIR%\core.v %HDL_DIR%\ula.v ^
    %HDL_DIR%\addr_dec.v %HDL_DIR%\instr_dec.v

echo #### Running the Verilator simulation
cd %TMP_PRO%
%VL_DIR%\V%PROC%_tb.exe
set WAVE=%TMP_PRO%\%PROC%_tb.vcd
set HDR=%TMP_PRO%\%PROC%_tb.vcd
set GTKWOUT=%TMP_PRO%\%PROC%_tb.gtkw

:: Run GtkWave ----------------------------------------------------------------
:run_gtkwave
echo #### Generating the .gtkw layout and launching GTKWave
:: gen_gtkw reads the header VCD and writes the .gtkw layout; GTKWave then opens
:: the waveform with -a. --zoom-fit fits the whole wave; the nipscern fork hides
:: the SST pane, so no --rcvar.
if exist %SIMU_DIR%\%GTKW% (
    %GTKWAVE% --dark --zoom-fit --left-justify %WAVE% -a %SIMU_DIR%\%GTKW%
) else (
    %BIN_DIR%\gen_gtkw.exe %HDR% %GTKWOUT% %TMP_DIR% %BIN_DIR%\comp2gtkw.exe
    %GTKWAVE% --dark --zoom-fit --left-justify %WAVE% -a %GTKWOUT%
)

cd %ROOT_DIR%
exit /b 0

:usage
echo usage: go_proc.bat [--sim iverilog^|verilator]
exit /b 0

:badsim
echo [go_proc] --sim must be 'iverilog' or 'verilator' (got "%SIM%")
exit /b 1
