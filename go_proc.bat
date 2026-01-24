:: ****************************************************************************
:: Script para emular o SAPHO na compilacao de um unico processador ***********
:: ****************************************************************************

:: Configura o terminal -------------------------------------------------------

cls
echo off
chcp 65001>%TMP_PRO%\log.txt

:: Configura o ambiente -------------------------------------------------------

:: diretorio atual
set ROOT_DIR=%cd%

:: programas necessarios
set BISON=C:\packs\msys64\usr\bin\bison.exe
set FLEX=C:\packs\msys64\usr\bin\flex.exe
set GCC=C:\packs\msys64\mingw64\bin\x86_64-w64-mingw32-gcc.exe
set IVERILOG=C:\nipscern\Aurora\components\Packages\iverilog\bin\iverilog.exe
set VVP=C:\nipscern\Aurora\components\Packages\iverilog\bin\vvp.exe
set GTKWAVE=C:\nipscern\Aurora\components\Packages\iverilog\gtkwave\bin\gtkwave.exe

set    TESTE_DIR=%ROOT_DIR%\Teste
rmdir %TESTE_DIR% /s /q

:: Parametros definidos pelo usuario do SAPHO para compilacao -----------------

:: nome da pasta do projeto
::set PROJET=Math
::set PROJET=RLS
set PROJET=FFT
:: nome do tipo de processador a ser simulado (uma sub-pasta do projeto)
::set PROC=ArcTan
::set PROC=Seno
::set PROC=Sqrt
::set PROC=proc_rls
set PROC=proc_fft
:: nome do arquivo cmm em que o processador esta definido
::set FNAM=ArcTan.cmm
::set FNAM=Seno.cmm
::set FNAM=Sqrt.cmm
::set FNAM=proc_rls.cmm
set FNAM=proc_fft.cmm
:: test_bench (sem .v) a ser simulado (tem que estar na pasta Simulation)
:: se nao achar, usa simulacao padrao
set TB=errado
:: nome do arquivo de visualizacao do gtkwave (se nao achar, usa o script padrao)
set GTKW=teste.gtkw
:: frequencia de operacao do processador em MHz
set FRE_CLK=100
:: numero de clocks a ser simulado
set NUM_CLK=1000000

:: Parametros que o SAPHO tem que saber ---------------------------------------

:: Arvore de pastas apos a instalacao
set INST_DIR=%TESTE_DIR%\saphoComponents
set BIN_DIR=%INST_DIR%\bin
set HDL_DIR=%INST_DIR%\HDL
set MAC_DIR=%INST_DIR%\Macros
set SCR_DIR=%INST_DIR%\Scripts
set TMP_DIR=%INST_DIR%\Temp

:: Arvore de pastas do projeto sendo executado
set USER_DIR=%TESTE_DIR%\Projetos
set PROC_DIR=%USER_DIR%\%PROJET%\%PROC%
set SOFT_DIR=%PROC_DIR%\Software
set HARD_DIR=%PROC_DIR%\Hardware
set SIMU_DIR=%PROC_DIR%\Simulation
set TMP_PRO=%TMP_DIR%\%PROC%

:: Gera diretorios pra teste --------------------------------------------------

mkdir %TESTE_DIR%
    mkdir %INST_DIR%
        mkdir %BIN_DIR%
        mkdir %HDL_DIR%
        mkdir %MAC_DIR%
        mkdir %SCR_DIR%
        mkdir %TMP_DIR%
            mkdir %TMP_PRO%
    mkdir %USER_DIR%

:: Copia os arquivos para os diretorios de teste ------------------------------

xcopy Exemplos %USER_DIR% /e /i /q>%TMP_PRO%\log.txt
xcopy HDL      %HDL_DIR%  /q    /y>%TMP_PRO%\log.txt
xcopy Macros   %MAC_DIR%  /q    /y>%TMP_PRO%\log.txt
xcopy Scripts  %SCR_DIR%  /q    /y>%TMP_PRO%\log.txt

:: Gera o compilador CMM ------------------------------------------------------

cd %ROOT_DIR%\CMMComp\Sources

%BISON% -y -d CMMComp.y
%FLEX%        CMMComp.l
%GCC%      -o CMMComp.exe data_assign.c data_declar.c data_use.c itr.c diretivas.c funcoes.c labels.c lex.yy.c oper.c saltos.c stdlib.c t2t.c variaveis.c array_index.c global.c macros.c y.tab.c

move CMMComp.exe %BIN_DIR%>%TMP_PRO%\log.txt
del  lex.yy.c
del  y.tab.c
del  y.tab.h

:: Gera o Assembler pre-processor ---------------------------------------------

cd %ROOT_DIR%\APP\Sources

%FLEX% -o app.c app.l
%GCC%  -o APP.exe app.c eval.c variaveis.c

move APP.exe %BIN_DIR%>%TMP_PRO%\log.txt
del  app.c

:: Gera o compilador Assembler ------------------------------------------------

cd %ROOT_DIR%\ASM\Sources

%FLEX% -o ASMComp.c ASMComp.l
%GCC%  -o ASM.exe ASMComp.c eval.c labels.c opcodes.c variaveis.c t2t.c hdl.c simulacao.c array.c

move ASM.exe %BIN_DIR%>%TMP_PRO%\log.txt
del  ASMComp.c

:: Gera tradutores para o GTKWave ---------------------------------------------

cd %SCR_DIR%

%GCC% -o comp2gtkw.exe comp2gtkw.c

move comp2gtkw.exe  %BIN_DIR%>%TMP_PRO%\log.txt

:: Executa o compilador CMM ---------------------------------------------------

echo #### Roda o compilador CMM

cd %BIN_DIR%

CMMComp.exe %FNAM% %PROC% %PROC_DIR% %MAC_DIR% %TMP_PRO% 0

:: Executa o Assembler pre-processor ------------------------------------------

echo #### Roda o Pre-assembler

set ASM_FILE=%SOFT_DIR%\%PROC%.asm

APP.exe %ASM_FILE% %TMP_PRO%

:: Executa o compilador Assembler ---------------------------------------------

echo #### Roda o Assembler

set ASM_FILE=%SOFT_DIR%\%PROC%.asm

ASM.exe %ASM_FILE% %PROC_DIR% %HDL_DIR% %MAC_DIR% %TMP_PRO% %FRE_CLK% %NUM_CLK% 0

:: Gera o testbench com o Icarus ----------------------------------------------

echo #### Roda o Icarus

set UPROC=%HARD_DIR%\%PROC%
cd  %HDL_DIR%

if exist %SIMU_DIR%\%TB%.v (
    set TB_MOD=%TB%
) else (
    copy %TMP_PRO%\%PROC%_tb.v %SIMU_DIR%>%TMP_PRO%\log.txt
    set TB_MOD=%PROC%_tb
)

%IVERILOG% -s %TB_MOD% -o %TMP_PRO%\%PROC%.vvp %SIMU_DIR%\%TB_MOD%.v %UPROC%.v addr_dec.v instr_dec.v processor.v core.v ula.v

:: Roda o testbench com o vvp -------------------------------------------------

echo #### Roda o VVP

copy %UPROC%_data.mif %TMP_PRO%>%TMP_PRO%\log.txt
copy %UPROC%_inst.mif %TMP_PRO%>%TMP_PRO%\log.txt

cd  %TMP_PRO%

%VVP% %PROC%.vvp -fst

:: Roda o GtkWave -------------------------------------------------------------

echo #### Roda o GTKWave

echo %TMP_PRO%>tcl_infos.txt
echo %BIN_DIR%>>tcl_infos.txt

copy %SCR_DIR%\fix.vcd %TMP_PRO%>%TMP_PRO%\log.txt

if exist %SIMU_DIR%\%GTKW% (
    %GTKWAVE% --rcvar "hide_sst on" --dark %SIMU_DIR%\%GTKW%      --script=%SCR_DIR%\pos_gtkw.tcl
) else (
    %GTKWAVE% --rcvar "hide_sst on" --dark %TMP_PRO%\%TB_MOD%.vcd --script=%SCR_DIR%\gtk_proc_init.tcl
)

cd %ROOT_DIR%