:: ****************************************************************************
:: Script to emulate SAPHO when compiling a single processor ******************
:: ****************************************************************************

:: Set up the terminal --------------------------------------------------------

cls
echo off
chcp 65001 >nul

:: Set up the environment -----------------------------------------------------

:: Resolve ROOT_DIR + the prebuilt binaries (YANC_BIN) and the tool locations
:: (IVERILOG, VVP, GTKWAVE) with no hardcoded paths. Scripts\setup.bat builds /
:: downloads everything once and caches the paths; env.bat loads them here.
call "%~dp0Scripts\env.bat"
cd /d "%ROOT_DIR%"

:: This Icarus flow needs the binaries plus iverilog/vvp and GTKWave ----------
if not exist "%YANC_BIN%\cmmcomp.exe" (
    echo [go_proc] YANC binaries missing in "%YANC_BIN%".
    echo            Run  Scripts\setup.bat  once to build or download them.
    exit /b 1
)
if not defined IVERILOG (
    echo [go_proc] Icarus Verilog not found - run Scripts\setup.bat, or install
    echo            it from https://bleyer.org/icarus/ and re-run.
    exit /b 1
)
if not defined GTKWAVE (
    echo [go_proc] GTKWave not found - run Scripts\setup.bat to fetch the
    echo            nipscernlab GTKWave build, then re-run.
    exit /b 1
)

set    TESTE_DIR=%ROOT_DIR%\Teste
rmdir %TESTE_DIR% /s /q

:: Parameters defined by the SAPHO user for compilation -----------------------

:: project folder name
::set PROJET=Math
::set PROJET=RLS
set PROJET=FFT
:: processor type name to simulate (a subfolder of the project)
::set PROC=ArcTan
::set PROC=Seno
::set PROC=Sqrt
::set PROC=proc_rls
set PROC=proc_fft
:: cmm filename where the processor is defined
::set FNAM=ArcTan.cmm
::set FNAM=Seno.cmm
::set FNAM=Sqrt.cmm
::set FNAM=proc_rls.cmm
set FNAM=proc_fft.cmm
:: test_bench (without .v) to simulate (must be in the Simulation folder)
:: if not found, uses default simulation
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

set ASM_FILE=%SOFT_DIR%\%PROC%.asm

asmcomp.exe -i %ASM_FILE% -p %PROC_DIR% -d %HDL_DIR% -m %MAC_DIR% -t %TMP_PRO% -f %FRE_CLK% -c %NUM_CLK%

:: Build the testbench with Icarus --------------------------------------------

echo #### Running Icarus

set UPROC=%HARD_DIR%\%PROC%
cd  %HDL_DIR%

if exist %SIMU_DIR%\%TB%.v (
    set TB_MOD=%TB%
) else (
    copy %TMP_PRO%\%PROC%_tb.v %SIMU_DIR%>%TMP_PRO%\log.txt
    set TB_MOD=%PROC%_tb
)

%IVERILOG% -s %TB_MOD% -o %TMP_PRO%\%PROC%.vvp %SIMU_DIR%\%TB_MOD%.v %UPROC%.v addr_dec.v instr_dec.v processor.v core.v ula.v

:: Run the testbench with vvp -------------------------------------------------

echo #### Running VVP

copy %UPROC%_data.mif %TMP_PRO%>%TMP_PRO%\log.txt
copy %UPROC%_inst.mif %TMP_PRO%>%TMP_PRO%\log.txt

cd  %TMP_PRO%

:: header-only pass (no -fst -> text VCD): gives gen_gtkw the signal list fast.
:: The generated <proc>_tb has the +HEADER_ONLY gate (#1; $dumpflush; $finish);
:: a user testbench without it just runs normally and still yields a VCD header.
%VVP% %PROC%.vvp +HEADER_ONLY
copy %TB_MOD%.vcd %TB_MOD%_hdr.vcd>%TMP_PRO%\log.txt

:: real run -> the FST waveform GTKWave opens
%VVP% %PROC%.vvp -fst

:: Run GtkWave ----------------------------------------------------------------

echo #### Generating the .gtkw layout and launching GTKWave

:: gen_gtkw reads the header VCD and writes the .gtkw layout; GTKWave then opens
:: the FST waveform with -a. --zoom-fit fits the whole wave (the Tcl flow did
:: Zoom_Best_Fit); the nipscern fork hides the SST pane, so no --rcvar.
if exist %SIMU_DIR%\%GTKW% (
    %GTKWAVE% --dark --zoom-fit --left-justify %TMP_PRO%\%TB_MOD%.vcd -a %SIMU_DIR%\%GTKW%
) else (
    %BIN_DIR%\gen_gtkw.exe %TMP_PRO%\%TB_MOD%_hdr.vcd %TMP_PRO%\%TB_MOD%.gtkw %TMP_DIR% %BIN_DIR%\comp2gtkw.exe
    %GTKWAVE% --dark --zoom-fit --left-justify %TMP_PRO%\%TB_MOD%.vcd -a %TMP_PRO%\%TB_MOD%.gtkw
)

cd %ROOT_DIR%
