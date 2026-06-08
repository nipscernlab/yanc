:: ****************************************************************************
:: single_proc - end-to-end example of the YANC single-processor pipeline.
::
:: It takes one C+- (.cmm) program and runs the full compile pipeline that
:: turns it into the synthesizable Verilog (.v) of a SAPHO processor -- the
:: core you would synthesize onto an FPGA to actually run this code in
:: hardware. It then runs the simulate-and-view routines: the simulator
:: compiles the asm-generated testbench (<proc>_tb.v) together with that
:: synthesizable <proc>.v, and the result is opened in GTKWave, where you can
:: watch the processor execute its program instruction by instruction.
::
:: The bundled example (proc_fft) computes an order-8 FFT, using the C+-
:: language's bit-reversed ("inverted") addressing for the FFT data.
::
::   single_proc.bat                  -> simulate with Icarus (default)
::   single_proc.bat --sim verilator  -> simulate with Verilator (+define+YANC_TRACE)
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
    echo [single_proc] YANC binaries missing in "%YANC_BIN%".
    echo            Run  Scripts\setup.bat  once to build or download them.
    exit /b 1
)
if not defined GTKWAVE (
    echo [single_proc] GTKWave not found - run Scripts\setup.bat to fetch the
    echo            nipscernlab GTKWave build, then re-run.
    exit /b 1
)
if /i "%SIM%"=="verilator" (
    if not defined VERILATOR (
        echo [single_proc] Verilator not found - run Scripts\setup.bat, or install it
        echo            with "pacman -S mingw-w64-x86_64-verilator" and re-run.
        exit /b 1
    )
) else (
    if not defined IVERILOG (
        echo [single_proc] Icarus Verilog not found - run Scripts\setup.bat, or install
        echo            it from https://bleyer.org/icarus/ and re-run.
        exit /b 1
    )
)

:: When iverilog/verilator come from MSYS2, their tools need mingw64\bin on PATH.
if defined MINGW_BIN set "PATH=%MINGW_BIN%;%PATH%"

:: What to simulate (a project under Compilers\CMMComp\Tests) -----------------

:: processor type / project folder to simulate
set PROC=proc_fft
:: cmm filename where the processor is defined
set FNAM=proc_fft.cmm
:: processor operating frequency in MHz
set FRE_CLK=100
:: number of clocks to simulate
set NUM_CLK=1000000

:: Repo sources, read in place (never written to) -----------------------------
set HDL_DIR=%ROOT_DIR%\HDL
set MAC_DIR=%ROOT_DIR%\Compilers\CMMComp\Includes
set BIN_DIR=%YANC_BIN%
set SRC_PROC=%ROOT_DIR%\Compilers\CMMComp\Tests\%PROC%

:: Work area: the only files this script creates (Teste\ is gitignored) -------
set WORK=%ROOT_DIR%\Teste
set PROC_DIR=%WORK%\%PROC%
set SOFT_DIR=%PROC_DIR%\Software
set HARD_DIR=%PROC_DIR%\Hardware
set TMP_DIR=%WORK%\Temp
set TMP_PRO=%TMP_DIR%\%PROC%
set VL_DIR=%TMP_PRO%\vl

:: The .cmm is the only input; cmmcomp/asmcomp create Software/Hardware outputs here.
rmdir %WORK% /s /q
mkdir %TMP_PRO%
mkdir %SOFT_DIR%
copy "%SRC_PROC%\Software\%FNAM%" "%SOFT_DIR%\">%TMP_PRO%\log.txt

:: Verilator's g++ wants a writable TMP for its intermediate files
set TMP=%TMP_PRO%
set TEMP=%TMP_PRO%

:: Compile: C+- -> asm -> Verilog --------------------------------------------

echo #### Running the CMM compiler
"%BIN_DIR%\cmmcomp.exe" -i %FNAM% -n %PROC% -p %PROC_DIR% -m %MAC_DIR% -t %TMP_PRO%

echo #### Running the Pre-assembler
set ASM_FILE=%SOFT_DIR%\%PROC%.asm
"%BIN_DIR%\appcomp.exe" -i %ASM_FILE% -t %TMP_PRO%

echo #### Running the Assembler
"%BIN_DIR%\asmcomp.exe" -i %ASM_FILE% -p %PROC_DIR% -d %HDL_DIR% -m %MAC_DIR% -t %TMP_PRO% -f %FRE_CLK% -c %NUM_CLK%

:: Build + run the simulation -------------------------------------------------

set UPROC=%HARD_DIR%\%PROC%
if /i "%SIM%"=="verilator" goto :sim_verilator

:: --- Icarus -----------------------------------------------------------------
echo #### Running Icarus
:: The testbench asmcomp generated (<proc>_tb.v in Temp).
set TB_MOD=%PROC%_tb
cd %HDL_DIR%
%IVERILOG% -s %TB_MOD% -o %TMP_PRO%\%PROC%.vvp %TMP_PRO%\%PROC%_tb.v %UPROC%.v addr_dec.v instr_dec.v processor.v core.v ula.v

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

:: View in GTKWave (layout built by gen_gtkw) ---------------------------------
:run_gtkwave
echo #### Generating the .gtkw layout (gen_gtkw) and launching GTKWave
%BIN_DIR%\gen_gtkw.exe %HDR% %GTKWOUT% %TMP_DIR% %BIN_DIR%\comp2gtkw.exe
%GTKWAVE% --dark --zoom-fit --left-justify %WAVE% -a %GTKWOUT%

cd %ROOT_DIR%
exit /b 0

:usage
echo usage: single_proc.bat [--sim iverilog^|verilator]
exit /b 0

:badsim
echo [single_proc] --sim must be 'iverilog' or 'verilator' (got "%SIM%")
exit /b 1
