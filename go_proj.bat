:: ****************************************************************************
:: Script para emular o SAPHO na compilacao de um projeto com varios procs
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
set PROJET=DTW
:: lista dos tipo de processadores que tem no projeto (nomes das sub-pastas do projeto)
set PROC_LIST=ProcDTW ZeroCross
:: lista das instancias que serao simuladas (um mesmo proc pode ter varias instancias)
set INST_LIST=ZeroCross_inst DTWv4_inst
:: lista do tipo de processador para cada instancia (tem que ser do mesmo tamanho de PROC_LIST)
set PROC_TYPE=ZeroCross ProcDTW
:: nome do test bench (sem .v) a ser simulado (tem que estar na pasta TopLevel)
set TB=top_level_tb
:: nome do arquivo de visualizacao do gtkwave (se nao achar, usa o script padrao)
set GTKW=dtw.gtkw

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
set PROJ_DIR=%USER_DIR%\%PROJET%
set TOPL_DIR=%PROJ_DIR%\TopLevel

:: Gera diretorios pra teste --------------------------------------------------

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

:: Copia os arquivos para os diretorios de teste ------------------------------

xcopy Exemplos %USER_DIR% /e /i /q>%TMP_DIR%\xcopy.txt
xcopy HDL %HDL_DIR% /q /y>%TMP_DIR%\xcopy.txt
xcopy Macros %MAC_DIR% /q /y>%TMP_DIR%\xcopy.txt
xcopy Scripts %SCR_DIR% /q /y>%TMP_DIR%\xcopy.txt

:: Gera o compilador CMM ------------------------------------------------------

cd %ROOT_DIR%\CMMComp\Sources

%BISON% -y -d CMMComp.y
%FLEX%        CMMComp.l
%GCC%      -o CMMComp.exe data_assign.c data_declar.c macros.c itr.c data_use.c diretivas.c funcoes.c labels.c lex.yy.c oper.c saltos.c stdlib.c t2t.c variaveis.c array_index.c global.c y.tab.c

move CMMComp.exe %BIN_DIR%>%TMP_DIR%\xcopy.txt
del lex.yy.c
del  y.tab.c
del  y.tab.h

:: Gera o Assembler pre-processor ---------------------------------------------

cd %ROOT_DIR%\APP\Sources

%FLEX% -o app.c app.l
%GCC%  -o APP.exe app.c eval.c variaveis.c

move APP.exe %BIN_DIR%>%TMP_DIR%\xcopy.txt
del app.c

:: Gera o compilador Assembler ------------------------------------------------

cd %ROOT_DIR%\ASM\Sources

%FLEX% -o ASMComp.c ASMComp.l
%GCC%  -o ASM.exe ASMComp.c eval.c labels.c opcodes.c variaveis.c t2t.c hdl.c simulacao.c array.c

move ASM.exe %BIN_DIR%>%TMP_DIR%\xcopy.txt
del ASMComp.c

:: Gera tradutores de dados ---------------------------------------------------

cd %SCR_DIR%

%GCC% -o comp2gtkw.exe comp2gtkw.c

move comp2gtkw.exe  %BIN_DIR%>%TMP_DIR%\xcopy.txt

:: Executa o compilador CMM ---------------------------------------------------

cd  %BIN_DIR%

(for %%i in (%PROC_LIST%) do (
    CMMComp.exe %%i.cmm %%i %PROJ_DIR%\%%i %MAC_DIR% %TMP_DIR%\%%i 1
))

:: Executa o Assembler pre-processor ------------------------------------------

(for %%i in (%PROC_LIST%) do (
    APP.exe %PROJ_DIR%\%%i\Software\%%i.asm %TMP_DIR%\%%i
))

:: Executa o compilador Assembler ---------------------------------------------

(for %%i in (%PROC_LIST%) do (
    ASM.exe %PROJ_DIR%\%%i\Software\%%i.asm %PROJ_DIR%\%%i %HDL_DIR% %MAC_DIR% %TMP_DIR%\%%i 0 0 1
    cp %PROJ_DIR%\%%i\Hardware\%%i.v %TMP_DIR%\%%i
))

:: Gera o testbench com o Icarus ----------------------------------------------

cd %TMP_DIR%

setlocal enabledelayedexpansion

:: lista arquivos da pasta HDL
dir %HDL_DIR%\*.v /b > f_list.txt
for /f "delims=" %%a in (%TMP_DIR%\f_list.txt) do set "HDL_V=!HDL_V!%HDL_DIR%\%%a "

:: lista arquivos da pasta TopLevel
dir %TOPL_DIR%\*.v /b > f_list.txt
for /f "delims=" %%a in (%TMP_DIR%\f_list.txt) do set "TOP_V=!TOP_V!%TOPL_DIR%\%%a "

:: lista arquivos dos processadores encontrados
for %%a in (%PROC_LIST%) do set "PRO_V=!PRO_V!%TMP_DIR%\%%a\%%a.v " 

%IVERILOG% -s %TB% -o %TMP_DIR%\%PROJET%.vvp %HDL_V% %PRO_V% %TOP_V%

for %%a in (%PROC_LIST%) do copy %TMP_DIR%\%%a\%%a_tb.v %PROJ_DIR%\%%a\Simulation>%TMP_DIR%\xcopy.txt

:: Roda o testbench com o vvp -------------------------------------------------

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

:: Roda o GtkWave -------------------------------------------------------------

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