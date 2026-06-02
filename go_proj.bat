:: ****************************************************************************
:: Script to emulate SAPHO when compiling a project with multiple procs
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
    echo [go_proj] YANC binaries missing in "%YANC_BIN%".
    echo            Run  Scripts\setup.bat  once to build or download them.
    exit /b 1
)
if not defined IVERILOG (
    echo [go_proj] Icarus Verilog not found - run Scripts\setup.bat, or install
    echo            it from https://bleyer.org/icarus/ and re-run.
    exit /b 1
)
if not defined GTKWAVE (
    echo [go_proj] GTKWave not found - run Scripts\setup.bat to fetch the
    echo            nipscernlab GTKWave build, then re-run.
    exit /b 1
)

:: When iverilog comes from MSYS2, vvp needs mingw64\bin on PATH for its DLLs.
if defined MINGW_BIN set "PATH=%MINGW_BIN%;%PATH%"

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
:: whatever TMP a previous bat left in the cmd session (e.g. go_proc_vl sets it
:: to its own Temp\<proc>, which go_proj's rmdir then deletes) -- leaving gcc /
:: iverilog with no temp dir ("Cannot create temporary file ... check TMP").
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

:: Build the testbench with Icarus --------------------------------------------

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

%IVERILOG% -s %TB% -o %TMP_DIR%\%PROJET%.vvp %HDL_V% %PRO_V% %TOP_V%

for %%a in (%PROC_LIST%) do copy %TMP_DIR%\%%a\%%a_tb.v %USER_DIR%\%%a\Simulation>%TMP_DIR%\xcopy.txt

:: Run the testbench with vvp -------------------------------------------------

dir %TOPL_DIR%\*.txt /b > f_list.txt
for /f "delims=" %%a in (%TMP_DIR%\f_list.txt) do copy %TOPL_DIR%\%%a .\>%TMP_DIR%\xcopy.txt
for %%a in (%PROC_LIST%) do copy %USER_DIR%\%%a\Hardware\%%a_inst.mif .\>%TMP_DIR%\xcopy.txt
for %%a in (%PROC_LIST%) do copy %USER_DIR%\%%a\Hardware\%%a_data.mif .\>%TMP_DIR%\xcopy.txt
for %%a in (%PROC_LIST%) do copy %TMP_DIR%\%%a\pc_%%a_mem.txt .\>%TMP_DIR%\xcopy.txt

endlocal

del f_list.txt
del xcopy.txt

:: header-only pass (no -fst -> tiny text VCD): gives gen_gtkw the full signal
:: list in ~100 ms instead of converting the multi-GB body. top_level_tb gates
:: this on +HEADER_ONLY (#1; $dumpflush; $finish).
%VVP% %PROJET%.vvp +HEADER_ONLY
copy %TB%.vcd %TB%_hdr.vcd>%TMP_DIR%\xcopy.txt

:: +WAVE enables the tb's $dumpvars (gated off by default so the regression's
:: heavy multi-proc sim doesn't crash the FST writer); -fst is the dump format.
%VVP% %PROJET%.vvp -fst +WAVE

:: Run GtkWave ----------------------------------------------------------------

:: gen_gtkw reads the header VCD and writes the multi-proc .gtkw (one section per
:: instance owning valr2+linetabs); GTKWave opens the FST waveform with -a.
:: --zoom-fit fits the whole wave; the nipscern fork hides the SST pane (no rcvar).
if exist %TOPL_DIR%\%GTKW% (
    %GTKWAVE% --dark --zoom-fit --left-justify %TMP_DIR%\%TB%.vcd -a %TOPL_DIR%\%GTKW%
) else (
    %BIN_DIR%\gen_gtkw.exe %TMP_DIR%\%TB%_hdr.vcd %TMP_DIR%\%TB%.gtkw %TMP_DIR% %BIN_DIR%\comp2gtkw.exe
    %GTKWAVE% --dark --zoom-fit --left-justify %TMP_DIR%\%TB%.vcd -a %TMP_DIR%\%TB%.gtkw
)

cd %ROOT_DIR%
