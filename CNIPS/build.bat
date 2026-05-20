@echo off
REM ---------------------------------------------------------------------------
REM CNIPS build script — produces cnipspp.exe (preprocessor) and cnips.exe -----
REM ---------------------------------------------------------------------------
REM Override YANC target defaults by setting these env vars before running:
REM   set CFG_NUBITS=32
REM   set CFG_NBMANT=23
REM   set CFG_NBEXPO=8
REM   ...
REM A user .c source can still override any of them with #pragma yanc <key> <val>
REM ---------------------------------------------------------------------------

setlocal

REM default target: 32-bit / IEEE-754 single (float format derived from NUBITS)
if not defined CFG_NUBITS set CFG_NUBITS=32
if not defined CFG_NBMANT set CFG_NBMANT=23
if not defined CFG_NBEXPO set CFG_NBEXPO=8
if not defined CFG_NUGAIN set CFG_NUGAIN=128
if not defined CFG_NDSTAC set CFG_NDSTAC=8
if not defined CFG_SDEPTH set CFG_SDEPTH=8
if not defined CFG_NUIOIN set CFG_NUIOIN=1
if not defined CFG_NUIOOU set CFG_NUIOOU=1
if not defined CFG_FFTSIZ set CFG_FFTSIZ=3

if not defined GCC   set GCC=gcc
if not defined BISON set BISON=bison
if not defined FLEX  set FLEX=flex

set DEFS=-DCFG_NUBITS=%CFG_NUBITS% -DCFG_NBMANT=%CFG_NBMANT% -DCFG_NBEXPO=%CFG_NBEXPO% -DCFG_NUGAIN=%CFG_NUGAIN% -DCFG_NDSTAC=%CFG_NDSTAC% -DCFG_SDEPTH=%CFG_SDEPTH% -DCFG_NUIOIN=%CFG_NUIOIN% -DCFG_NUIOOU=%CFG_NUIOOU% -DCFG_FFTSIZ=%CFG_FFTSIZ%

echo CNIPS: defs = %DEFS%

pushd Sources

echo CNIPS: building cnipspp.exe (preprocessor)
"%GCC%" -O2 -Wall -o ..\cnipspp.exe cnipspp.c
if errorlevel 1 ( echo CNIPS: cnipspp build FAILED & popd & exit /b 1 )

echo CNIPS: running bison + flex
"%BISON%" -y -d CNIPSComp.y
if errorlevel 1 ( echo CNIPS: bison FAILED & popd & exit /b 1 )
"%FLEX%" CNIPSComp.l
if errorlevel 1 ( echo CNIPS: flex FAILED & popd & exit /b 1 )

echo CNIPS: building cnips.exe (compiler)
"%GCC%" -O2 -Wall -Wno-unused-but-set-variable -Wno-unused-variable -Wno-unused-function %DEFS% -o ..\cnips.exe main.c messages.c types.c symtab.c ast.c codegen.c lex.yy.c y.tab.c
if errorlevel 1 ( echo CNIPS: cnips build FAILED & del /q lex.yy.c y.tab.c y.tab.h 2>nul & popd & exit /b 1 )

del /q lex.yy.c y.tab.c y.tab.h 2>nul
popd

echo CNIPS: build OK
endlocal
