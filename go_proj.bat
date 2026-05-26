:: ****************************************************************************
:: Script to emulate SAPHO when compiling a project with multiple procs
:: ****************************************************************************

:: Set up the terminal --------------------------------------------------------

cls
echo off
chcp 65001>%TMP_PRO%\log.txt

:: Set up the environment -----------------------------------------------------

:: current directory
set ROOT_DIR=%cd%

:: required tools
set BISON=C:\packs\msys64\usr\bin\bison.exe
set FLEX=C:\packs\msys64\usr\bin\flex.exe
set GCC=C:\packs\msys64\mingw64\bin\x86_64-w64-mingw32-gcc.exe
set IVERILOG=C:\nipscern\Aurora\components\Packages\iverilog\bin\iverilog.exe
set VVP=C:\nipscern\Aurora\components\Packages\iverilog\bin\vvp.exe
set GTKWAVE=C:\nipscern\Aurora\components\Packages\iverilog\gtkwave\bin\gtkwave.exe

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

:: Copy files into the test directories ---------------------------------------

xcopy Exemplos %USER_DIR% /e /i /q>%TMP_DIR%\xcopy.txt
xcopy HDL %HDL_DIR% /q /y>%TMP_DIR%\xcopy.txt
xcopy Macros %MAC_DIR% /q /y>%TMP_DIR%\xcopy.txt
xcopy Scripts %SCR_DIR% /q /y>%TMP_DIR%\xcopy.txt

:: Build the CMM compiler -----------------------------------------------------

cd %ROOT_DIR%\CMMComp\Sources

%BISON% -y -d CMMComp.y
%FLEX%        CMMComp.l
%GCC%      -o CMMComp.exe ast.c data_assign.c data_declar.c macros.c itr.c data_use.c diretivas.c funcoes.c labels.c lex.yy.c oper.c saltos.c stdlib.c t2t.c variaveis.c array_index.c global.c messages.c args.c y.tab.c

move CMMComp.exe %BIN_DIR%>%TMP_DIR%\xcopy.txt
del lex.yy.c
del  y.tab.c
del  y.tab.h

:: Build the Assembler pre-processor ------------------------------------------

cd %ROOT_DIR%\APPComp\Sources

%FLEX% -o app.c app.l
%GCC%  -o APP.exe app.c eval.c variaveis.c messages.c args.c

move APP.exe %BIN_DIR%>%TMP_DIR%\xcopy.txt
del app.c

:: Build the Assembler compiler -----------------------------------------------

cd %ROOT_DIR%\ASMComp\Sources

%FLEX% -o ASMComp.c ASMComp.l
%GCC%  -o ASM.exe ASMComp.c eval.c labels.c opcodes.c variaveis.c t2t.c hdl.c simulacao.c array.c messages.c args.c

move ASM.exe %BIN_DIR%>%TMP_DIR%\xcopy.txt
del ASMComp.c

:: Build data translators -----------------------------------------------------

cd %SCR_DIR%

%GCC% -o comp2gtkw.exe comp2gtkw.c

move comp2gtkw.exe  %BIN_DIR%>%TMP_DIR%\xcopy.txt

:: Run the CMM compiler -------------------------------------------------------

cd  %BIN_DIR%

(for %%i in (%PROC_LIST%) do (
    CMMComp.exe -i %%i.cmm -n %%i -p %PROJ_DIR%\%%i -m %MAC_DIR% -t %TMP_DIR%\%%i --project
))

:: Run the Assembler pre-processor --------------------------------------------

(for %%i in (%PROC_LIST%) do (
    APP.exe -i %PROJ_DIR%\%%i\Software\%%i.asm -t %TMP_DIR%\%%i
))

:: Run the Assembler compiler -------------------------------------------------

(for %%i in (%PROC_LIST%) do (
    ASM.exe -i %PROJ_DIR%\%%i\Software\%%i.asm -p %PROJ_DIR%\%%i -d %HDL_DIR% -m %MAC_DIR% -t %TMP_DIR%\%%i -f 0 -c 0 --project
    cp %PROJ_DIR%\%%i\Hardware\%%i.v %TMP_DIR%\%%i
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

for %%a in (%PROC_LIST%) do copy %TMP_DIR%\%%a\%%a_tb.v %PROJ_DIR%\%%a\Simulation>%TMP_DIR%\xcopy.txt

:: Run the testbench with vvp -------------------------------------------------

dir %TOPL_DIR%\*.txt /b > f_list.txt
for /f "delims=" %%a in (%TMP_DIR%\f_list.txt) do copy %TOPL_DIR%\%%a .\>%TMP_DIR%\xcopy.txt
for %%a in (%PROC_LIST%) do copy %PROJ_DIR%\%%a\Hardware\%%a_inst.mif .\>%TMP_DIR%\xcopy.txt
for %%a in (%PROC_LIST%) do copy %PROJ_DIR%\%%a\Hardware\%%a_data.mif .\>%TMP_DIR%\xcopy.txt
for %%a in (%PROC_LIST%) do copy %TMP_DIR%\%%a\pc_%%a_mem.txt .\>%TMP_DIR%\xcopy.txt

endlocal

del f_list.txt
del xcopy.txt

::start /b cmd /c %VVP% %PROJET%.vvp
%VVP% %PROJET%.vvp -fst

:: Run GtkWave ----------------------------------------------------------------

echo %INST_LIST%> tcl_infos.txt
echo %PROC_TYPE%>>tcl_infos.txt
echo %TMP_DIR%>>  tcl_infos.txt
echo %BIN_DIR%>>  tcl_infos.txt
echo %SCR_DIR%>>  tcl_infos.txt

copy %SCR_DIR%\fix.vcd %TMP_DIR%>%TMP_DIR%\xcopy.txt

if exist %TOPL_DIR%\%GTKW% (
    ::start /b cmd /c %GTKWAVE% --rcvar "hide_sst on" --dark %TOPL_DIR%\%GTKW% --script=%SCR_DIR%\pos_gtkw.tcl
    %GTKWAVE% --rcvar "hide_sst on" --dark %TOPL_DIR%\%GTKW% --script=%SCR_DIR%\pos_gtkw.tcl
) else (
    ::start /b cmd /c %GTKWAVE% --rcvar "hide_sst on" --dark %TMP_DIR%\%TB%.vcd --script=%SCR_DIR%\gtk_proj_init.tcl
    %GTKWAVE% --rcvar "hide_sst on" --dark %TMP_DIR%\%TB%.vcd --script=%SCR_DIR%\gtk_proj_init.tcl
)

cd %ROOT_DIR%
