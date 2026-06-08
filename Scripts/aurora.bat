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
where make.exe >nul 2>nul || (
    echo ERROR: make.exe not on PATH. Install MSYS2's make package
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
:: Build the binaries via the top-level Makefile (single source of truth) ------
:: ----------------------------------------------------------------------------
::
:: make / sh / bison / flex come from MSYS2's usr\bin and the cross gcc from
:: mingw64\bin -- both already required on PATH by the checks above. The cross
:: tuple gives stand-alone .exe with no MSYS2 DLL dependency. Build into the
:: repo-local bin\ (the Makefile's cwd-relative default, so no backslash path is
:: passed into make/sh), then copy the .exe into the Aurora deploy bin\.
::
:: BISON=bison FLEX=flex are forced on the make command line so that a
:: pre-existing BISON / FLEX environment variable holding a backslash Windows
:: path (e.g. C:\packs\msys64\usr\bin\bison.exe -- which setup.bat may have
:: exported) cannot leak in. The Makefile's "BISON ?= bison" is a *conditional*
:: assignment and would NOT override such an inherited value; the backslashes
:: would then be eaten by MSYS2's /bin/sh ("C:packs...bison.exe: not found").
:: A make command-line assignment overrides the environment, and bison/flex are
:: already on PATH (checked above), so sh resolves the bare names cleanly.

pushd %SRC_DIR%
make CC=x86_64-w64-mingw32-gcc BISON=bison FLEX=flex clean all
popd

:: Copy only the binaries Aurora actually uses: the compile pipeline plus
:: comp2gtkw (the complex-number -> GTKWave converter). gen_gtkw is a
:: yanc-runner-only tool -- Aurora builds the GTKWave layout itself -- so it is
:: deliberately NOT deployed.
for %%E in (cmmcomp cppcomp cpppp appcomp asmcomp comp2gtkw) do (
    copy /Y "%SRC_DIR%\bin\%%E.exe" "%BLD_DIR%\bin\" >nul
)

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
:: Scripts/ here holds dev tooling (aurora.bat, regress.sh, comp2gtkw.c) and
:: the standalone runner scripts (single_proc, multi_proc, single_proc_cpp;
:: .bat + .sh) -- none of which belong in the deploy: Aurora drives the
:: compile/simulate pipeline from its own UI, not from these runners.

cd %SRC_DIR%
