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

@echo off
cls

:: %~dp0 = "<repo>\Scripts\" (trailing backslash). Up one -> repo root,
:: up two -> parent that holds Aurora\.
set SRC_DIR=%~dp0..
set BLD_DIR=%~dp0..\..\Aurora\components

:: ----------------------------------------------------------------------------
:: Toolchain (MSYS2 mingw64) --------------------------------------------------
:: ----------------------------------------------------------------------------
::
:: This script calls bison/flex from MSYS2's usr\bin and the MinGW cross
:: compiler x86_64-w64-mingw32-gcc.exe from MSYS2's mingw64\bin. The cross
:: tuple is intentional: it produces stand-alone .exes with no MSYS2 DLL
:: dependency, so the binaries copied into Aurora\components\bin\ run on
:: any Windows machine without needing MSYS2 installed.
::
:: If MSYS2 is already on your PATH, leave the next block commented.
:: Otherwise uncomment + adjust to match your MSYS2 install root:
::
:: set PATH=C:\msys64\mingw64\bin;C:\msys64\usr\bin;%PATH%
:: set PATH=C:\packs\msys64\mingw64\bin;C:\packs\msys64\usr\bin;%PATH%

:: Fail early with a clear message if the tools are not on PATH ---------------
where x86_64-w64-mingw32-gcc.exe >nul 2>nul || (
    echo ERROR: x86_64-w64-mingw32-gcc.exe not on PATH.
    echo Install MSYS2 ^(https://www.msys2.org/^), then either add
    echo     ^<msys64^>\mingw64\bin and ^<msys64^>\usr\bin
    echo to your system PATH, or uncomment one of the "set PATH=..." lines
    echo near the top of this script.
    exit /b 1
)
where bison.exe >nul 2>nul || (
    echo ERROR: bison.exe not on PATH. Install MSYS2's bison package
    echo and ensure ^<msys64^>\usr\bin is on PATH.
    exit /b 1
)
where flex.exe >nul 2>nul || (
    echo ERROR: flex.exe not on PATH. Install MSYS2's flex package
    echo and ensure ^<msys64^>\usr\bin is on PATH.
    exit /b 1
)

set GCC=x86_64-w64-mingw32-gcc.exe

:: ----------------------------------------------------------------------------
:: Clean the destination tree -------------------------------------------------
:: ----------------------------------------------------------------------------
::
:: Nuke + re-create each yanc-managed folder so the subsequent move / xcopy
:: steps always see a clean, empty destination directory. This survives any
:: corrupted state from earlier interrupted runs (e.g. a stray "bin" FILE
:: where the bin\ folder should be, which happens if `move foo.exe bin`
:: runs while bin\ does not yet exist as a directory).
::
:: Each target is hit with rmdir (handles the directory case) AND del
:: (handles the stray-file case); both are silenced so a fresh checkout
:: where these folders don't exist yet doesn't print "file not found".

for %%D in (bin HDL Macros Header) do (
    rmdir /s /q %BLD_DIR%\%%D 2>nul
    del   /q    %BLD_DIR%\%%D 2>nul
    mkdir       %BLD_DIR%\%%D
)

:: Scripts/ -- yanc no longer ships anything here, but previously-deployed
:: yanc artifacts (.tcl / .ys / fix.vcd / .bat / .sh / .c) may linger from
:: older releases. Wipe them by name, not by nuking the folder: Aurora's
:: own files (copy-components.js, download-*.js, empty.gtkw, ...) sit in
:: the same folder and must be preserved.
if exist %BLD_DIR%\Scripts (
    del /q %BLD_DIR%\Scripts\*.tcl       2>nul
    del /q %BLD_DIR%\Scripts\*.ys        2>nul
    del /q %BLD_DIR%\Scripts\fix.vcd     2>nul
    del /q %BLD_DIR%\Scripts\aurora.bat  2>nul
    del /q %BLD_DIR%\Scripts\regress.sh  2>nul
    del /q %BLD_DIR%\Scripts\build.bat   2>nul
    del /q %BLD_DIR%\Scripts\comp2gtkw.c 2>nul
)

:: ----------------------------------------------------------------------------
:: Populate the \bin folder with the executables ------------------------------
:: ----------------------------------------------------------------------------

:: Build the CMM compiler -----------------------------------------------------

cd %SRC_DIR%\Compilers\CMMComp\Sources

bison -y -d CMMComp.y
flex        CMMComp.l
%GCC%    -o cmmcomp.exe ast.c data_assign.c data_declar.c data_use.c itr.c diretivas.c funcoes.c labels.c lex.yy.c oper.c saltos.c stdlib.c t2t.c variaveis.c array_index.c global.c macros.c messages.c args.c y.tab.c

move /Y cmmcomp.exe %BLD_DIR%\bin\
del  lex.yy.c
del  y.tab.c
del  y.tab.h

:: Build the Assembler pre-processor ------------------------------------------

cd %SRC_DIR%\Compilers\APPComp\Sources

flex  -o app.c app.l
%GCC% -o appcomp.exe app.c eval.c variaveis.c messages.c args.c

move /Y appcomp.exe %BLD_DIR%\bin\
del  app.c

cd %SRC_DIR%

:: Build the Assembler compiler -----------------------------------------------

cd %SRC_DIR%\Compilers\ASMComp\Sources

flex  -o ASMComp.c ASMComp.l
%GCC% -o asmcomp.exe ASMComp.c eval.c labels.c opcodes.c variaveis.c t2t.c hdl.c simulacao.c array.c messages.c args.c

move /Y asmcomp.exe %BLD_DIR%\bin\
del  ASMComp.c

:: Build the CPP preprocessor ------------------------------------------------

cd %SRC_DIR%\Compilers\CPPComp\Sources

%GCC% -O2 -Wall -o cpppp.exe cpppp.c

move /Y cpppp.exe %BLD_DIR%\bin\

:: Build the CPP compiler ----------------------------------------------------

cd %SRC_DIR%\Compilers\CPPComp\Sources

bison -y -d CPPComp.y
flex        CPPComp.l
%GCC% -O2 -Wall -Wno-unused-but-set-variable -Wno-unused-variable -Wno-unused-function -o cppcomp.exe main.c messages.c types.c symtab.c ast.c codegen.c lex.yy.c y.tab.c

move /Y cppcomp.exe %BLD_DIR%\bin\
del  lex.yy.c
del  y.tab.c
del  y.tab.h

:: Build translators for GTKWave ----------------------------------------------

cd %SRC_DIR%\Scripts

%GCC% -mwindows -o comp2gtkw.exe comp2gtkw.c
%GCC%           -o gen_gtkw.exe  gen_gtkw.c

move /Y comp2gtkw.exe %BLD_DIR%\bin\
move /Y gen_gtkw.exe  %BLD_DIR%\bin\

:: ----------------------------------------------------------------------------
:: Copy HDL, Macros and Scripts folders ---------------------------------------
:: ----------------------------------------------------------------------------

cd %BLD_DIR%

:: /I = treat destination as directory (suppress F/D prompt)
:: /Q = quiet
:: /Y = overwrite without prompting
xcopy %SRC_DIR%\HDL                        HDL    /I /Q /Y
xcopy %SRC_DIR%\Compilers\CMMComp\Includes Macros /I /Q /Y
xcopy %SRC_DIR%\Compilers\CPPComp\Includes Header /I /Q /Y

:: Scripts/ is intentionally NOT copied: Aurora manages its own scripts
:: (copy-components.js, download-*.js, proc2rtl.ys, ...). The yanc-side
:: Scripts/ here holds dev tooling (aurora.bat, regress.sh, comp2gtkw.c)
:: that doesn't belong in the deploy.

cd %SRC_DIR%
