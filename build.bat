:: ****************************************************************************
:: Script para criar a pasta saphoCompmonents *********************************
:: ****************************************************************************

:: ----------------------------------------------------------------------------
:: Configura o ambiente -------------------------------------------------------
:: ----------------------------------------------------------------------------

cls
echo off

set SRC_DIR=%cd%
set BLD_DIR=C:\nipscern\Aurora\components

set GCC=C:\packs\msys64\mingw64\bin\x86_64-w64-mingw32-gcc.exe

:: ----------------------------------------------------------------------------
:: Limpa a arvore de pastas ---------------------------------------------------
:: ----------------------------------------------------------------------------

:: limpa \bin
del %BLD_DIR%\bin\appcomp.exe
del %BLD_DIR%\bin\asmcomp.exe
del %BLD_DIR%\bin\cmmcomp.exe
del %BLD_DIR%\bin\comp2gtkw.exe

:: limpa \HDL
del %BLD_DIR%\HDL\*.* /q

:: limpa \Macros
del %BLD_DIR%\Macros\*.* /q

:: limpa \Scripts
del %BLD_DIR%\Scripts\*.c
del %BLD_DIR%\Scripts\*.tcl
del %BLD_DIR%\Scripts\*.ys

:: ----------------------------------------------------------------------------
:: Preenche a pasta \bin com os executaveis -----------------------------------
:: ----------------------------------------------------------------------------

:: Gera o compilador CMM ------------------------------------------------------

cd %SRC_DIR%\CMMComp\Sources

bison -y -d CMMComp.y
flex        CMMComp.l
%GCC%    -o cmmcomp.exe data_assign.c data_declar.c data_use.c itr.c diretivas.c funcoes.c labels.c lex.yy.c oper.c saltos.c stdlib.c t2t.c variaveis.c array_index.c global.c macros.c messages.c y.tab.c

move cmmcomp.exe %BLD_DIR%\bin
del  lex.yy.c
del  y.tab.c
del  y.tab.h

:: Gera o Assembler pre-processor ---------------------------------------------

cd %SRC_DIR%\APP\Sources

flex  -o app.c app.l
%GCC% -o appcomp.exe app.c eval.c variaveis.c messages.c

move appcomp.exe %BLD_DIR%\bin
del  app.c

cd %SRC_DIR%

:: Gera o compilador Assembler ------------------------------------------------

cd %SRC_DIR%\ASM\Sources

flex  -o ASMComp.c ASMComp.l
%GCC% -o asmcomp.exe ASMComp.c eval.c labels.c opcodes.c variaveis.c t2t.c hdl.c simulacao.c array.c messages.c

move asmcomp.exe %BLD_DIR%\bin
del  ASMComp.c

:: Gera tradutores para o GTKWave ---------------------------------------------

cd %SRC_DIR%\Scripts

%GCC% -o comp2gtkw.exe comp2gtkw.c

move comp2gtkw.exe  %BLD_DIR%\bin

:: ----------------------------------------------------------------------------
:: Copia as pastas HDL, Macros e Scipts ---------------------------------------
:: ----------------------------------------------------------------------------

cd %BLD_DIR%

xcopy %SRC_DIR%\HDL HDL /q /y
xcopy %SRC_DIR%\Macros Macros /q /y
xcopy %SRC_DIR%\Scripts\*.* Scripts /q /y

cd %SRC_DIR%