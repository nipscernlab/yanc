:: ****************************************************************************
:: single_proc_cpp - end-to-end example of the YANC single-processor pipeline,
:: C++ edition. Mirrors single_proc.bat but feeds a C++ (.cpp) program through
:: the C++ front end (cpppp -> cppcomp) instead of the C+- one (cmmcomp).
::
:: It takes one .cpp program and runs the full compile pipeline that turns it
:: into the synthesizable Verilog (.v) of a SAPHO processor, then simulates it
:: and opens the result in GTKWave, where you can watch the processor execute
:: its program instruction by instruction (and see each int/float variable
:: tracked at its data-memory address, plus the current source line).
::
::   single_proc_cpp.bat                  -> simulate with Icarus (default)
::   single_proc_cpp.bat --sim verilator  -> simulate with Verilator (+YANC_TRACE)
::   single_proc_cpp.bat --no-view        -> run the pipeline but skip GTKWave
::
:: NOTE: close any GTKWave still showing this proc before re-running -- an open
:: viewer locks Teste\...\proc_cpp_tb.vcd and blocks the workspace cleanup.
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

:: Args: --sim iverilog|verilator (default iverilog), --no-view ---------------
set "SIM=iverilog"
set "VIEW=1"
:parse_args
if "%~1"=="" goto args_done
if /i "%~1"=="--sim"     ( set "SIM=%~2" & shift & shift & goto parse_args )
if /i "%~1"=="--no-view" ( set "VIEW=0" & shift & goto parse_args )
if /i "%~1"=="-h"        goto usage
if /i "%~1"=="--help"    goto usage
echo [single_proc_cpp] unknown argument: %~1
goto usage
:args_done
if /i "%SIM%"=="vl"     set "SIM=verilator"
if /i "%SIM%"=="icarus" set "SIM=iverilog"
if /i not "%SIM%"=="iverilog" if /i not "%SIM%"=="verilator" goto badsim

:: Tools: binaries always, plus the chosen simulator, plus GTKWave if viewing -
if not exist "%YANC_BIN%\cppcomp.exe" (
    echo [single_proc_cpp] cppcomp.exe missing in "%YANC_BIN%".
    echo            Run  Scripts\setup.bat  once to build or download the binaries.
    exit /b 1
)
if /i "%SIM%"=="verilator" (
    if not defined VERILATOR (
        echo [single_proc_cpp] Verilator not found - run Scripts\setup.bat or install it.
        exit /b 1
    )
) else (
    if not defined IVERILOG (
        echo [single_proc_cpp] Icarus Verilog not found - run Scripts\setup.bat or install it.
        exit /b 1
    )
)
if "%VIEW%"=="1" if not defined GTKWAVE (
    echo [single_proc_cpp] GTKWave not found - run Scripts\setup.bat, or pass --no-view.
    exit /b 1
)

:: When iverilog/verilator come from MSYS2, their tools need mingw64\bin on PATH.
if defined MINGW_BIN set "PATH=%MINGW_BIN%;%PATH%"

:: What to compile/simulate (a project under Compilers\CPPComp\Tests) ---------

:: processor type / project folder
set PROC=proc_cpp
:: cpp filename where the program is defined
set FNAM=proc_cpp.cpp
:: processor operating frequency in MHz
set FRE_CLK=100
:: number of clocks to simulate
set NUM_CLK=100000

:: Repo sources, read in place (never written to) -----------------------------
set HDL_DIR=%ROOT_DIR%\HDL
set MAC_DIR=%ROOT_DIR%\Compilers\CMMComp\Includes
set INC_DIR=%ROOT_DIR%\Compilers\CPPComp\Includes
set BIN_DIR=%YANC_BIN%
set SRC_PROC=%ROOT_DIR%\Compilers\CPPComp\Tests\%PROC%

:: Work area: the only files this script creates (Teste\ is gitignored) -------
set WORK=%ROOT_DIR%\Teste
set PROC_DIR=%WORK%\%PROC%
set SOFT_DIR=%PROC_DIR%\Software
set HARD_DIR=%PROC_DIR%\Hardware
set TMP_DIR=%WORK%\Temp
set TMP_PRO=%TMP_DIR%\%PROC%
set VL_DIR=%TMP_PRO%\vl

:: The .cpp is the only input; cppcomp/asmcomp create Software/Hardware outputs.
rmdir /s /q "%WORK%" 2>nul
if exist "%WORK%" (
    echo [single_proc_cpp] could not clear "%WORK%" -- is GTKWave still open on
    echo            this proc? Close it and re-run ^(it locks proc_cpp_tb.vcd^).
    exit /b 1
)
mkdir "%TMP_PRO%"
mkdir "%SOFT_DIR%"
copy "%SRC_PROC%\Software\%FNAM%" "%SOFT_DIR%\" >nul

:: Verilator's g++ wants a writable TMP for its intermediate files
set TMP=%TMP_PRO%
set TEMP=%TMP_PRO%

:: Compile: C++ -> asm -> Verilog --------------------------------------------

echo #### Running the C++ preprocessor (cpppp)
"%BIN_DIR%\cpppp.exe" -i "%SOFT_DIR%\%FNAM%" -o "%TMP_PRO%\pp.cpp" -I "%INC_DIR%" -I "%SOFT_DIR%"

echo #### Running the C++ compiler (cppcomp)
"%BIN_DIR%\cppcomp.exe" -i "%TMP_PRO%\pp.cpp" -p "%PROC_DIR%" -n "%PROC%" -t "%TMP_PRO%"

set ASM_FILE=%SOFT_DIR%\%PROC%.asm
if not exist "%ASM_FILE%" ( echo [single_proc_cpp] cppcomp did not produce "%ASM_FILE%". & goto fail )

echo #### Running the Pre-assembler (appcomp)
"%BIN_DIR%\appcomp.exe" -i "%ASM_FILE%" -t "%TMP_PRO%"

echo #### Running the Assembler (asmcomp)
"%BIN_DIR%\asmcomp.exe" -i "%ASM_FILE%" -p "%PROC_DIR%" -d "%HDL_DIR%" -m "%MAC_DIR%" -t "%TMP_PRO%" -f %FRE_CLK% -c %NUM_CLK%

:: Build + run the simulation -------------------------------------------------

set UPROC=%HARD_DIR%\%PROC%
if not exist "%UPROC%.v" ( echo [single_proc_cpp] asmcomp did not produce "%UPROC%.v". & goto fail )
if /i "%SIM%"=="verilator" goto sim_verilator

:: --- Icarus -----------------------------------------------------------------
echo #### Running Icarus
set TB_MOD=%PROC%_tb
cd /d "%HDL_DIR%"
"%IVERILOG%" -s %TB_MOD% -o "%TMP_PRO%\%PROC%.vvp" "%TMP_PRO%\%PROC%_tb.v" "%UPROC%.v" addr_dec.v instr_dec.v processor.v core.v ula.v
if not exist "%TMP_PRO%\%PROC%.vvp" ( echo [single_proc_cpp] iverilog failed to build the .vvp. & goto fail )

echo #### Running VVP
copy "%UPROC%_data.mif" "%TMP_PRO%\" >nul
copy "%UPROC%_inst.mif" "%TMP_PRO%\" >nul
cd /d "%TMP_PRO%"
:: header-only pass (no -fst -> text VCD): gives gen_gtkw the signal list fast.
"%VVP%" "%PROC%.vvp" +HEADER_ONLY
copy "%TB_MOD%.vcd" "%TB_MOD%_hdr.vcd" >nul
:: real run -> the FST waveform GTKWave opens
"%VVP%" "%PROC%.vvp" -fst
set WAVE=%TMP_PRO%\%TB_MOD%.vcd
set HDR=%TMP_PRO%\%TB_MOD%_hdr.vcd
set GTKWOUT=%TMP_PRO%\%TB_MOD%.gtkw
goto run_gtkwave

:: --- Verilator --------------------------------------------------------------
:sim_verilator
echo #### Running Verilator (+define+YANC_TRACE, --binary --timing --trace)
"%VERILATOR%" --binary --timing --trace +define+YANC_TRACE ^
    --top-module %PROC%_tb ^
    -Wno-lint -Wno-UNOPTFLAT -Wno-MULTIDRIVEN -Wno-BLKANDNBLK -Wno-WIDTH ^
    -Wno-CASEINCOMPLETE -Wno-IMPLICIT -Wno-COMBDLY -Wno-STMTDLY -Wno-INFINITELOOP ^
    --Mdir "%VL_DIR%" ^
    "%TMP_PRO%\%PROC%_tb.v" "%UPROC%.v" ^
    "%HDL_DIR%\processor.v" "%HDL_DIR%\core.v" "%HDL_DIR%\ula.v" ^
    "%HDL_DIR%\addr_dec.v" "%HDL_DIR%\instr_dec.v"

echo #### Running the Verilator simulation
cd /d "%TMP_PRO%"
"%VL_DIR%\V%PROC%_tb.exe"
set WAVE=%TMP_PRO%\%PROC%_tb.vcd
set HDR=%TMP_PRO%\%PROC%_tb.vcd
set GTKWOUT=%TMP_PRO%\%PROC%_tb.gtkw

:: View in GTKWave (layout built by gen_gtkw) ---------------------------------
:run_gtkwave
echo #### Generating the .gtkw layout (gen_gtkw)
"%BIN_DIR%\gen_gtkw.exe" "%HDR%" "%GTKWOUT%" "%TMP_DIR%" "%BIN_DIR%\comp2gtkw.exe"
if "%VIEW%"=="0" (
    echo #### --no-view: skipping GTKWave. Waveform: %WAVE%
    cd /d "%ROOT_DIR%"
    exit /b 0
)
echo #### Launching GTKWave
"%GTKWAVE%" --dark --zoom-fit --left-justify "%WAVE%" -a "%GTKWOUT%"

cd /d "%ROOT_DIR%"
exit /b 0

:fail
cd /d "%ROOT_DIR%"
exit /b 1

:usage
echo usage: single_proc_cpp.bat [--sim iverilog^|verilator] [--no-view]
exit /b 0

:badsim
echo [single_proc_cpp] --sim must be 'iverilog' or 'verilator' (got "%SIM%")
exit /b 1
