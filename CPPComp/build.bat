@echo off
REM ---------------------------------------------------------------------------
REM CPPComp build script - produces cpppp.exe (preprocessor) and cppcomp.exe --
REM ---------------------------------------------------------------------------
REM Fixed target: 32-bit word / IEEE-754 single float / sizeof()==1 /
REM short & long fold to 32-bit / 4K-word data memory with a dynamic heap.
REM A source can still override target params with #pragma yanc <key> <val>.
REM ---------------------------------------------------------------------------

setlocal

if not defined CFG_NUBITS set CFG_NUBITS=32
if not defined CFG_NBMANT set CFG_NBMANT=23
if not defined CFG_NBEXPO set CFG_NBEXPO=8
if not defined CFG_NUGAIN set CFG_NUGAIN=128
if not defined CFG_NDSTAC set CFG_NDSTAC=128
if not defined CFG_SDEPTH set CFG_SDEPTH=128
if not defined CFG_NUIOIN set CFG_NUIOIN=1
if not defined CFG_NUIOOU set CFG_NUIOOU=1
if not defined CFG_FFTSIZ set CFG_FFTSIZ=3
if not defined CFG_HEAPSZ set CFG_HEAPSZ=2048

if not defined GCC   set GCC=gcc
if not defined BISON set BISON=bison
if not defined FLEX  set FLEX=flex

set DEFS=-DCFG_NUBITS=%CFG_NUBITS% -DCFG_NBMANT=%CFG_NBMANT% -DCFG_NBEXPO=%CFG_NBEXPO% -DCFG_NUGAIN=%CFG_NUGAIN% -DCFG_NDSTAC=%CFG_NDSTAC% -DCFG_SDEPTH=%CFG_SDEPTH% -DCFG_NUIOIN=%CFG_NUIOIN% -DCFG_NUIOOU=%CFG_NUIOOU% -DCFG_FFTSIZ=%CFG_FFTSIZ% -DCFG_HEAPSZ=%CFG_HEAPSZ%

echo CPPComp: defs = %DEFS%

if not exist .bin mkdir .bin
pushd Sources

echo CPPComp: building cpppp.exe (preprocessor)
"%GCC%" -O2 -Wall -o ..\.bin\cpppp.exe cpppp.c
if errorlevel 1 ( echo CPPComp: cpppp build FAILED & popd & exit /b 1 )

echo CPPComp: running bison + flex
"%BISON%" -y -d CPPComp.y
if errorlevel 1 ( echo CPPComp: bison FAILED & popd & exit /b 1 )
"%FLEX%" CPPComp.l
if errorlevel 1 ( echo CPPComp: flex FAILED & popd & exit /b 1 )

echo CPPComp: building cppcomp.exe (compiler)
"%GCC%" -O2 -Wall -Wno-unused-but-set-variable -Wno-unused-variable -Wno-unused-function %DEFS% -o ..\.bin\cppcomp.exe main.c messages.c types.c symtab.c ast.c codegen.c lex.yy.c y.tab.c
if errorlevel 1 ( echo CPPComp: cppcomp build FAILED & del /q lex.yy.c y.tab.c y.tab.h 2>nul & popd & exit /b 1 )

del /q lex.yy.c y.tab.c y.tab.h 2>nul
popd
echo CPPComp: done.
