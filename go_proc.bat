:: ****************************************************************************
:: Script to emulate SAPHO when compiling a single processor ******************
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
set PROC_DIR=%USER_DIR%\%PROJET%\%PROC%
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

xcopy Exemplos %USER_DIR% /e /i /q>%TMP_PRO%\log.txt
xcopy HDL      %HDL_DIR%  /q    /y>%TMP_PRO%\log.txt
xcopy Macros   %MAC_DIR%  /q    /y>%TMP_PRO%\log.txt
xcopy Scripts  %SCR_DIR%  /q    /y>%TMP_PRO%\log.txt

:: Build the CMM compiler -----------------------------------------------------

cd %ROOT_DIR%\CMMComp\Sources

%BISON% -y -d CMMComp.y
%FLEX%        CMMComp.l
%GCC%      -o cmmcomp.exe ast.c data_assign.c data_declar.c data_use.c itr.c diretivas.c funcoes.c labels.c lex.yy.c oper.c saltos.c stdlib.c t2t.c variaveis.c array_index.c global.c macros.c messages.c args.c y.tab.c

move cmmcomp.exe %BIN_DIR%>%TMP_PRO%\log.txt
del  lex.yy.c
del  y.tab.c
del  y.tab.h

:: Build the Assembler pre-processor ------------------------------------------

cd %ROOT_DIR%\APPComp\Sources

%FLEX% -o app.c app.l
%GCC%  -o appcomp.exe app.c eval.c variaveis.c messages.c args.c

move appcomp.exe %BIN_DIR%>%TMP_PRO%\log.txt
del  app.c

:: Build the Assembler compiler -----------------------------------------------

cd %ROOT_DIR%\ASMComp\Sources

%FLEX% -o ASMComp.c ASMComp.l
%GCC%  -o asmcomp.exe ASMComp.c eval.c labels.c opcodes.c variaveis.c t2t.c hdl.c simulacao.c array.c messages.c args.c

move asmcomp.exe %BIN_DIR%>%TMP_PRO%\log.txt
del  ASMComp.c

:: Build translators for GTKWave ----------------------------------------------

cd %SCR_DIR%

%GCC% -o comp2gtkw.exe comp2gtkw.c

move comp2gtkw.exe  %BIN_DIR%>%TMP_PRO%\log.txt

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

:: asmcomp expects these output dirs to exist (does not create them)
mkdir %PROC_DIR%\Hardware 2>nul
mkdir %PROC_DIR%\Simulation 2>nul

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

%VVP% %PROC%.vvp -fst

:: Run GtkWave ----------------------------------------------------------------

echo #### Running GTKWave

echo %TMP_PRO%>tcl_infos.txt
echo %BIN_DIR%>>tcl_infos.txt

copy %SCR_DIR%\fix.vcd %TMP_PRO%>%TMP_PRO%\log.txt

if exist %SIMU_DIR%\%GTKW% (
    %GTKWAVE% --rcvar "hide_sst on" --dark %SIMU_DIR%\%GTKW%      --script=%SCR_DIR%\pos_gtkw.tcl
) else (
    %GTKWAVE% --rcvar "hide_sst on" --dark %TMP_PRO%\%TB_MOD%.vcd --script=%SCR_DIR%\gtk_proc_init.tcl
)

cd %ROOT_DIR%
