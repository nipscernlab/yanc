:: ****************************************************************************
:: Build YANC and deploy into the sibling Aurora checkout.
::
:: Layout assumption (both repos side-by-side under a common parent):
::
::   <parent>\
::      yanc\        (this repo; this script is at yanc\Scripts\aurora.bat)
::      Aurora\
::         components\   <-- BLD_DIR (deploy target)
::
:: Both SRC_DIR and BLD_DIR are derived from %~dp0 (the script's own
:: directory), so it works no matter what the CWD was when invoked.
:: ****************************************************************************

:: ----------------------------------------------------------------------------
:: Set up the environment -----------------------------------------------------
:: ----------------------------------------------------------------------------

cls
echo off

:: %~dp0 = "<repo>\Scripts\" (trailing backslash). Up one -> repo root,
:: up two -> parent that holds Aurora\.
set SRC_DIR=%~dp0..
set BLD_DIR=%~dp0..\..\Aurora\components

set GCC=C:\packs\msys64\mingw64\bin\x86_64-w64-mingw32-gcc.exe

:: ----------------------------------------------------------------------------
:: Clean the folder tree ------------------------------------------------------
:: ----------------------------------------------------------------------------

:: clean \bin
del %BLD_DIR%\bin\appcomp.exe
del %BLD_DIR%\bin\asmcomp.exe
del %BLD_DIR%\bin\cmmcomp.exe
del %BLD_DIR%\bin\comp2gtkw.exe

:: clean \HDL
del %BLD_DIR%\HDL\*.* /q

:: clean \Macros
del %BLD_DIR%\Macros\*.* /q

:: clean \Header (CPPComp/Includes shims that .cpp programs include)
if not exist %BLD_DIR%\Header mkdir %BLD_DIR%\Header
del %BLD_DIR%\Header\*.* /q

:: clean \Scripts
del %BLD_DIR%\Scripts\*.c
del %BLD_DIR%\Scripts\*.tcl
del %BLD_DIR%\Scripts\*.ys

:: cppcomp + cpppp belong to bin/ too -- include them in the bin sweep
del %BLD_DIR%\bin\cpppp.exe
del %BLD_DIR%\bin\cppcomp.exe

:: ----------------------------------------------------------------------------
:: Populate the \bin folder with the executables ------------------------------
:: ----------------------------------------------------------------------------

:: Build the CMM compiler -----------------------------------------------------

cd %SRC_DIR%\Compilers\CMMComp\Sources

bison -y -d CMMComp.y
flex        CMMComp.l
%GCC%    -o cmmcomp.exe ast.c data_assign.c data_declar.c data_use.c itr.c diretivas.c funcoes.c labels.c lex.yy.c oper.c saltos.c stdlib.c t2t.c variaveis.c array_index.c global.c macros.c messages.c args.c y.tab.c

move cmmcomp.exe %BLD_DIR%\bin
del  lex.yy.c
del  y.tab.c
del  y.tab.h

:: Build the Assembler pre-processor ------------------------------------------

cd %SRC_DIR%\Compilers\APPComp\Sources

flex  -o app.c app.l
%GCC% -o appcomp.exe app.c eval.c variaveis.c messages.c args.c

move appcomp.exe %BLD_DIR%\bin
del  app.c

cd %SRC_DIR%

:: Build the Assembler compiler -----------------------------------------------

cd %SRC_DIR%\Compilers\ASMComp\Sources

flex  -o ASMComp.c ASMComp.l
%GCC% -o asmcomp.exe ASMComp.c eval.c labels.c opcodes.c variaveis.c t2t.c hdl.c simulacao.c array.c messages.c args.c

move asmcomp.exe %BLD_DIR%\bin
del  ASMComp.c

:: Build the CPP preprocessor ------------------------------------------------

cd %SRC_DIR%\Compilers\CPPComp\Sources

%GCC% -O2 -Wall -o cpppp.exe cpppp.c

move cpppp.exe %BLD_DIR%\bin

:: Build the CPP compiler ----------------------------------------------------

cd %SRC_DIR%\Compilers\CPPComp\Sources

bison -y -d CPPComp.y
flex        CPPComp.l
%GCC% -O2 -Wall -Wno-unused-but-set-variable -Wno-unused-variable -Wno-unused-function -o cppcomp.exe main.c messages.c types.c symtab.c ast.c codegen.c lex.yy.c y.tab.c

move cppcomp.exe %BLD_DIR%\bin
del  lex.yy.c
del  y.tab.c
del  y.tab.h

:: Build translators for GTKWave ----------------------------------------------

cd %SRC_DIR%\Scripts

%GCC% -mwindows -o comp2gtkw.exe comp2gtkw.c

move comp2gtkw.exe  %BLD_DIR%\bin

:: ----------------------------------------------------------------------------
:: Copy HDL, Macros and Scripts folders ---------------------------------------
:: ----------------------------------------------------------------------------

cd %BLD_DIR%

xcopy %SRC_DIR%\HDL HDL /q /y
xcopy %SRC_DIR%\Compilers\CMMComp\Includes Macros /q /y
xcopy %SRC_DIR%\Compilers\CPPComp\Includes Header /q /y
xcopy %SRC_DIR%\Scripts\*.* Scripts /q /y

cd %SRC_DIR%
