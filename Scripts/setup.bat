:: ****************************************************************************
:: YANC setup -- one-time prep for the single_proc.bat / multi_proc.bat runners.
::
:: It removes the need for the per-machine path block that used to sit at the
:: top of every runner. Run it once after cloning the repo (or after
:: extracting a release) and it will:
::
::   1. Locate the build toolchain (x86_64-w64-mingw32-gcc / bison / flex) and,
::      if MSYS2 is installed but the packages are missing, offer to install
::      them with pacman.
::   2. Produce the YANC binaries in <repo>\bin :
::        * if they are already there (a release that ships the .exe), keep them
::        * else if the build toolchain is present, COMPILE them from source
::        * else DOWNLOAD the prebuilt binaries from the latest GitHub release
::   3. Locate the simulation tools (Icarus Verilog, Verilator) and GTKWave,
::      and offer to install the missing ones from MSYS2's UCRT64 with pacman.
::   4. Cache every resolved path in Scripts\tools.local.bat, which the
::      runners load through Scripts\env.bat. No path is hardcoded anywhere.
::
:: Flags:
::   setup.bat              auto: keep / build / download as described above
::   setup.bat --rebuild    force a fresh compile from source
::   setup.bat --download   force fetching the prebuilt binaries
:: ****************************************************************************

@echo off
setlocal enableextensions enabledelayedexpansion
cls

set "SCRIPTS_DIR=%~dp0"
pushd "%SCRIPTS_DIR%.." & set "ROOT_DIR=%CD%" & popd
set "YANC_BIN=%ROOT_DIR%\bin"
set "CACHE=%SCRIPTS_DIR%tools.local.bat"

set "FORCE="
if /i "%~1"=="--rebuild"  set "FORCE=build"
if /i "%~1"=="--build"    set "FORCE=build"
if /i "%~1"=="--download" set "FORCE=download"

echo ============================================================
echo  YANC setup
echo  repo : %ROOT_DIR%
echo ============================================================

:: ---------------------------------------------------------------------------
:: 1) Build toolchain (only needed to compile the binaries from source) -------
:: ---------------------------------------------------------------------------
set "GCC="
set "BISON="
set "FLEX="
set "MSYS2_ROOT="

for %%E in (x86_64-w64-mingw32-gcc.exe) do if not "%%~$PATH:E"=="" set "GCC=%%~$PATH:E"
if not defined GCC for %%E in (gcc.exe) do if not "%%~$PATH:E"=="" set "GCC=%%~$PATH:E"
for %%E in (bison.exe) do if not "%%~$PATH:E"=="" set "BISON=%%~$PATH:E"
for %%E in (flex.exe)  do if not "%%~$PATH:E"=="" set "FLEX=%%~$PATH:E"

for %%R in ("C:\msys64" "C:\packs\msys64" "%SystemDrive%\msys64" "C:\tools\msys64") do (
    if not defined MSYS2_ROOT if exist "%%~R\usr\bin\pacman.exe" set "MSYS2_ROOT=%%~R"
)

:: MSYS2's ucrt64\bin: the UCRT64 environment, the only one that still ships
:: Icarus and Verilator (MINGW64 no longer does). It holds the gcc, the
:: simulators and the DLLs they link against, so the runners put it on PATH
:: when the tools come from MSYS2. The name MINGW_BIN is kept so that a
:: tools.local.bat written by an earlier setup.bat still loads.
set "MINGW_BIN="
set "PKG=mingw-w64-ucrt-x86_64"
if defined MSYS2_ROOT set "MINGW_BIN=%MSYS2_ROOT%\ucrt64\bin"
if defined MSYS2_ROOT if not defined GCC   if exist "%MSYS2_ROOT%\ucrt64\bin\x86_64-w64-mingw32-gcc.exe" set "GCC=%MSYS2_ROOT%\ucrt64\bin\x86_64-w64-mingw32-gcc.exe"
if defined MSYS2_ROOT if not defined BISON if exist "%MSYS2_ROOT%\usr\bin\bison.exe" set "BISON=%MSYS2_ROOT%\usr\bin\bison.exe"
if defined MSYS2_ROOT if not defined FLEX  if exist "%MSYS2_ROOT%\usr\bin\flex.exe"  set "FLEX=%MSYS2_ROOT%\usr\bin\flex.exe"

set "BUILD_OK="
if defined GCC if defined BISON if defined FLEX set "BUILD_OK=1"

set "BIN_PRESENT="
if exist "%YANC_BIN%\cmmcomp.exe" if exist "%YANC_BIN%\asmcomp.exe" if exist "%YANC_BIN%\cppcomp.exe" if exist "%YANC_BIN%\gen_gtkw.exe" set "BIN_PRESENT=1"

:: C++ toolchain sanity for the Verilator flow (MSYS2's gcc 16 lacks a
:: libstdc++ symbol -- see :check_cxx). Non-fatal; the Icarus flow is unaffected.
call :check_cxx

:: Offer to install the build tools via pacman if we are going to need them
:: (binaries are not present yet and the user did not force a download).
if not defined BIN_PRESENT if not "%FORCE%"=="download" if not defined BUILD_OK (
    echo.
    echo [tools] Build toolchain incomplete:
    if not defined GCC   echo          - missing: x86_64-w64-mingw32-gcc / gcc
    if not defined BISON echo          - missing: bison
    if not defined FLEX  echo          - missing: flex
    if defined MSYS2_ROOT (
        echo        MSYS2 detected at "!MSYS2_ROOT!".
        set "ans="
        set /p "ans=Install the build tools now via pacman? [y/N] "
        if /i "!ans!"=="y" (
            "!MSYS2_ROOT!\usr\bin\pacman.exe" -S --needed --noconfirm make bison flex !PKG!-gcc
            if exist "!MINGW_BIN!\x86_64-w64-mingw32-gcc.exe" set "GCC=!MINGW_BIN!\x86_64-w64-mingw32-gcc.exe"
            if exist "!MSYS2_ROOT!\usr\bin\bison.exe" set "BISON=!MSYS2_ROOT!\usr\bin\bison.exe"
            if exist "!MSYS2_ROOT!\usr\bin\flex.exe"  set "FLEX=!MSYS2_ROOT!\usr\bin\flex.exe"
            if defined GCC if defined BISON if defined FLEX set "BUILD_OK=1"
        )
    ) else (
        echo        Install MSYS2 from https://www.msys2.org/ then run:
        echo            pacman -S make bison flex mingw-w64-ucrt-x86_64-gcc
        echo        ^(or just let setup download the prebuilt binaries below^)
    )
)

:: ---------------------------------------------------------------------------
:: 2) Produce the binaries in <repo>\bin --------------------------------------
:: ---------------------------------------------------------------------------
echo.
echo --- YANC binaries -------------------------------------------------------
if "%FORCE%"=="build"    goto :do_build
if "%FORCE%"=="download" goto :do_download
if defined BIN_PRESENT (
    echo Prebuilt binaries already present in "%YANC_BIN%" - keeping them.
    echo ^(setup.bat --rebuild recompiles them, --download refetches them^)
    goto :bins_done
)
if defined BUILD_OK goto :do_build
echo No binaries and no build toolchain - falling back to the prebuilt release.
goto :do_download

:do_build
if not defined BUILD_OK (
    echo ERROR: cannot compile - gcc/bison/flex not found ^(see above^).
    echo        Re-run as "setup.bat --download" to fetch prebuilt binaries.
    exit /b 1
)
call :build_bins
if errorlevel 1 ( echo ERROR: build failed. & exit /b 1 )
goto :bins_done

:do_download
call :download_bins
if errorlevel 1 ( echo ERROR: could not obtain the binaries. & exit /b 1 )
goto :bins_done

:bins_done

:: ---------------------------------------------------------------------------
:: 3) Simulation tools + GTKWave ----------------------------------------------
:: ---------------------------------------------------------------------------
echo.
echo --- Simulation tools ----------------------------------------------------

set "IVERILOG="
set "VVP="
set "VERILATOR="
set "VERILATOR_ROOT="
set "GTKWAVE="
set "FST2VCD="
:: (MINGW_BIN is resolved earlier, from MSYS2_ROOT - do not clear it here)

:: Icarus Verilog (iverilog + vvp) -- ships in MSYS2's UCRT64, so offer pacman
:: (mingw-w64-ucrt-x86_64-iverilog). A standalone install is still accepted.
for %%E in (iverilog.exe) do if not "%%~$PATH:E"=="" set "IVERILOG=%%~$PATH:E"
for %%E in (vvp.exe)      do if not "%%~$PATH:E"=="" set "VVP=%%~$PATH:E"
if defined MSYS2_ROOT (
    if not defined IVERILOG if exist "%MINGW_BIN%\iverilog.exe" set "IVERILOG=%MINGW_BIN%\iverilog.exe"
    if not defined VVP      if exist "%MINGW_BIN%\vvp.exe"      set "VVP=%MINGW_BIN%\vvp.exe"
)
for %%D in ("C:\iverilog\bin" "C:\nipscern\Aurora\components\Packages\iverilog\bin") do (
    if not defined IVERILOG if exist "%%~D\iverilog.exe" set "IVERILOG=%%~D\iverilog.exe"
    if not defined VVP      if exist "%%~D\vvp.exe"      set "VVP=%%~D\vvp.exe"
)
if not defined IVERILOG if defined MSYS2_ROOT (
    set "ans="
    set /p "ans=[icarus] not found. Install it via pacman now? [y/N] "
    if /i "!ans!"=="y" (
        "!MSYS2_ROOT!\usr\bin\pacman.exe" -S --needed --noconfirm !PKG!-iverilog
        if exist "!MINGW_BIN!\iverilog.exe" set "IVERILOG=!MINGW_BIN!\iverilog.exe"
        if exist "!MINGW_BIN!\vvp.exe"      set "VVP=!MINGW_BIN!\vvp.exe"
    )
)
if defined IVERILOG (
    echo [icarus]    %IVERILOG%
) else (
    echo [icarus]    NOT found - install MSYS2 + "pacman -S mingw-w64-ucrt-x86_64-iverilog"
    echo             ^(or a standalone build; needed for single_proc.bat / multi_proc.bat^)
)

:: Verilator -- ships in MSYS2's UCRT64, so offer pacman if MSYS2 is present --
for %%E in (verilator_bin.exe) do if not "%%~$PATH:E"=="" set "VERILATOR=%%~$PATH:E"
if defined MSYS2_ROOT if not defined VERILATOR if exist "%MINGW_BIN%\verilator_bin.exe" set "VERILATOR=%MINGW_BIN%\verilator_bin.exe"
if not defined VERILATOR if defined MSYS2_ROOT (
    set "ans="
    set /p "ans=[verilator] not found. Install it via pacman now? [y/N] "
    if /i "!ans!"=="y" (
        "!MSYS2_ROOT!\usr\bin\pacman.exe" -S --needed --noconfirm !PKG!-verilator !PKG!-lz4 make !PKG!-gcc !PKG!-python
        if exist "!MINGW_BIN!\verilator_bin.exe" set "VERILATOR=!MINGW_BIN!\verilator_bin.exe"
    )
)
:: MSYS2's Verilator does not depend on lz4, but its FST writer (--trace-fst,
:: multi_proc.bat --sim verilator) includes lz4.h: offer it when it is missing.
set "LZ4_MISSING="
if defined VERILATOR if defined MSYS2_ROOT if not exist "%MSYS2_ROOT%\ucrt64\include\lz4.h" set "LZ4_MISSING=1"
if defined LZ4_MISSING (
    set "ans="
    set /p "ans=[verilator] lz4.h not found, needed by multi_proc.bat --sim verilator. Install !PKG!-lz4 via pacman now? [y/N] "
    if /i "!ans!"=="y" "!MSYS2_ROOT!\usr\bin\pacman.exe" -S --needed --noconfirm !PKG!-lz4
)

:: VERILATOR_ROOT = ...\share\verilator. MINGW_BIN, the ucrt64\bin holding
:: Verilator's g++/python3 chain, is usually already set from MSYS2_ROOT; if
:: Verilator was found elsewhere on PATH, derive it from the exe location.
:: (These lines stay outside the block: cmd misparses a :: comment inside
:: parentheses.)
if defined VERILATOR (
    if not defined MINGW_BIN (
        for %%F in ("%VERILATOR%") do set "MINGW_BIN=%%~dpF"
        if "!MINGW_BIN:~-1!"=="\" set "MINGW_BIN=!MINGW_BIN:~0,-1!"
    )
    if not defined VERILATOR_ROOT for %%F in ("!MINGW_BIN!\..") do if exist "%%~fF\share\verilator" set "VERILATOR_ROOT=%%~fF\share\verilator"
    echo [verilator] !VERILATOR!
) else (
    echo [verilator] NOT found - install MSYS2 + "pacman -S mingw-w64-ucrt-x86_64-verilator"
    echo             ^(needed only for the Verilator flow: single_proc.bat --sim verilator^)
)

:: GTKWave -- MSYS2's UCRT64 package, offered through pacman like Icarus. A
:: GTKWave already on PATH (the nipscernlab build, say) is kept; the runners
:: give each GTKWave only the options it knows (Scripts\env.bat, GTKWAVE_OPTS).
for %%E in (gtkwave.exe) do if not "%%~$PATH:E"=="" set "GTKWAVE=%%~$PATH:E"
if defined MSYS2_ROOT if not defined GTKWAVE if exist "%MINGW_BIN%\gtkwave.exe" set "GTKWAVE=%MINGW_BIN%\gtkwave.exe"
if not defined GTKWAVE if defined MSYS2_ROOT (
    set "ans="
    set /p "ans=[gtkwave] not found. Install it via pacman now? [y/N] "
    if /i "!ans!"=="y" (
        "!MSYS2_ROOT!\usr\bin\pacman.exe" -S --needed --noconfirm !PKG!-gtkwave
        if exist "!MINGW_BIN!\gtkwave.exe" set "GTKWAVE=!MINGW_BIN!\gtkwave.exe"
    )
)
:: fst2vcd comes with GTKWave, in the same folder
if defined GTKWAVE for %%F in ("%GTKWAVE%") do if exist "%%~dpFfst2vcd.exe" set "FST2VCD=%%~dpFfst2vcd.exe"
if not defined FST2VCD for %%E in (fst2vcd.exe) do if not "%%~$PATH:E"=="" set "FST2VCD=%%~$PATH:E"
if defined GTKWAVE (
    echo [gtkwave]   %GTKWAVE%
) else (
    echo [gtkwave]   NOT found - install MSYS2 + "pacman -S mingw-w64-ucrt-x86_64-gtkwave"
    echo             ^(the runners still simulate and write the waveform without it^)
)

:: ---------------------------------------------------------------------------
:: 4) Cache the resolved paths for env.bat ------------------------------------
:: ---------------------------------------------------------------------------
> "%CACHE%" echo :: Generated by Scripts\setup.bat - machine-specific tool paths.
>>"%CACHE%" echo :: Do not commit. Re-run setup.bat to regenerate.
>>"%CACHE%" echo @echo off
if defined IVERILOG       >>"%CACHE%" echo set "IVERILOG=%IVERILOG%"
if defined VVP            >>"%CACHE%" echo set "VVP=%VVP%"
if defined VERILATOR      >>"%CACHE%" echo set "VERILATOR=%VERILATOR%"
if defined VERILATOR_ROOT >>"%CACHE%" echo set "VERILATOR_ROOT=%VERILATOR_ROOT%"
if defined MINGW_BIN      >>"%CACHE%" echo set "MINGW_BIN=%MINGW_BIN%"
if defined GTKWAVE        >>"%CACHE%" echo set "GTKWAVE=%GTKWAVE%"
if defined FST2VCD        >>"%CACHE%" echo set "FST2VCD=%FST2VCD%"

echo.
echo ============================================================
echo  Setup complete. Paths cached in:
echo    %CACHE%
echo.
echo  You can now run the examples, e.g.:
echo    single_proc.bat                  ^(one processor,      Icarus^)
echo    multi_proc.bat                  ^(multi-proc project, Icarus^)
echo    single_proc.bat --sim verilator  ^(one processor,      Verilator^)
echo    multi_proc.bat --sim verilator  ^(multi-proc project, Verilator^)
echo ============================================================
endlocal
exit /b 0

:: ===========================================================================
:: Subroutines
:: ===========================================================================

:check_cxx
:: MSYS2's gcc 16 (16.1 and 16.2, MINGW64 and UCRT64) ships a libstdc++ whose
:: std::string move constructor is missing; it shows at -Os, which
:: Verilator's build uses. With -D_GLIBCXX_ASSERTIONS the compiler
:: instantiates the constructor itself, and the runners pass that to
:: Verilator (Scripts\env.bat, YANC_VL_CFLAGS; #117). Probe with the g++
:: Verilator uses, without the flag and then with it, as setup.sh does.
:: Non-fatal: the Icarus flow does not need g++.
set "GXX="
if defined MINGW_BIN if exist "%MINGW_BIN%\g++.exe" set "GXX=%MINGW_BIN%\g++.exe"
if not defined GXX for %%E in (g++.exe) do if not "%%~$PATH:E"=="" set "GXX=%%~$PATH:E"
if not defined GXX exit /b 0
> "%TEMP%\yanc_cxx_probe.cpp" echo #include ^<string^>
>>"%TEMP%\yanc_cxx_probe.cpp" echo int main(){std::string a="x";std::string b=std::move(a)+"y";return b.size()?0:1;}
set "CXX_OK="
set "CXX_FLAG="
call :cxx_probe
if not defined CXX_OK call :cxx_probe -D_GLIBCXX_ASSERTIONS
if not defined CXX_OK goto :cxx_broken
if not defined CXX_FLAG goto :cxx_done
echo [g++]       this g++ lacks the std::string move constructor at -Os; the
echo             runners pass -D_GLIBCXX_ASSERTIONS to Verilator, which works around it.
goto :cxx_done
:cxx_broken
echo.
echo **************************************************************************
echo  WARNING: this g++ cannot link a std::string move, even with
echo  -D_GLIBCXX_ASSERTIONS: the Verilator flow will fail. The Icarus flow is
echo  unaffected. See docs\install.md.
echo **************************************************************************
:cxx_done
del "%TEMP%\yanc_cxx_probe.cpp" "%TEMP%\yanc_cxx_probe.exe" >nul 2>&1
exit /b 0

:cxx_probe
:: %1 = an extra flag, or nothing. Sets CXX_OK (and CXX_FLAG) if the probe
:: links. g++ runs cc1plus, as and ld from its own folder, so that folder goes
:: first on PATH for the call.
setlocal
for %%F in ("%GXX%") do set "PATH=%%~dpF;%PATH%"
"%GXX%" -Os %1 -o "%TEMP%\yanc_cxx_probe.exe" "%TEMP%\yanc_cxx_probe.cpp" >nul 2>&1
if errorlevel 1 (endlocal & exit /b 0)
endlocal & set "CXX_OK=1" & set "CXX_FLAG=%~1"
exit /b 0

:build_bins
:: Delegate to the top-level Makefile (single source of truth for the compile
:: rules). Put MSYS2's usr\bin (make, sh, bison, flex) and ucrt64\bin (the
:: cross gcc) on PATH so the MSYS2 make and its sh-based recipes resolve the
:: toolchain, then build into the repo-local bin\. We cd into the repo and use
:: the Makefile's default cwd-relative BIN=bin, so no Windows path (with
:: backslashes) is ever passed into make / sh.
echo [build] Building YANC binaries via make into "%YANC_BIN%"
if defined MSYS2_ROOT set "PATH=%MSYS2_ROOT%\usr\bin;%MINGW_BIN%;%PATH%"

:: Prefer the cross tuple (standalone .exe, no MSYS2 DLLs); fall back to gcc.
set "CCNAME=gcc"
echo %GCC% | find /i "x86_64-w64-mingw32-gcc" >nul && set "CCNAME=x86_64-w64-mingw32-gcc"

:: Force BISON=bison FLEX=flex on the command line: this batch resolved BISON /
:: FLEX above to full backslash Windows paths (e.g. ...\usr\bin\bison.exe) and
:: exports them into make's environment. The Makefile's "BISON ?= bison" is a
:: conditional assignment and would NOT override an inherited value, so the
:: backslashes would be eaten by MSYS2's /bin/sh. The command-line assignment
:: overrides the env var, and bison/flex are now on PATH, so sh resolves the
:: bare names cleanly.
pushd "%ROOT_DIR%"
make CC=%CCNAME% BISON=bison FLEX=flex clean all || (popd & exit /b 1)
popd

echo [build] Done.
exit /b 0

:download_bins
:: Pull the latest yanc-bin-*.zip release asset and unpack its bin\ into <repo>\bin.
echo [download] Fetching prebuilt YANC binaries from the latest GitHub release...
if not exist "%YANC_BIN%" mkdir "%YANC_BIN%"
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "$ErrorActionPreference='Stop'; $h=@{'User-Agent'='yanc-setup'};" ^
  "$r=Invoke-RestMethod 'https://api.github.com/repos/nipscernlab/yanc/releases/latest' -Headers $h;" ^
  "$a=$r.assets ^| Where-Object { $_.name -like 'yanc-bin-*.zip' } ^| Select-Object -First 1;" ^
  "if(-not $a){throw 'no yanc-bin-*.zip asset in the latest release'};" ^
  "$zip=Join-Path $env:TEMP $a.name;" ^
  "Invoke-WebRequest $a.browser_download_url -OutFile $zip -Headers $h;" ^
  "$dst=Join-Path $env:TEMP 'yanc-bin-extract'; if(Test-Path $dst){Remove-Item $dst -Recurse -Force};" ^
  "Expand-Archive $zip $dst -Force;" ^
  "Copy-Item (Join-Path $dst 'bin\*') '%YANC_BIN%' -Recurse -Force;" ^
  "Write-Host ('[download] installed ' + $a.name + ' -> %YANC_BIN%')"
if errorlevel 1 (
    echo [download] FAILED. Download a release zip manually from
    echo            https://github.com/nipscernlab/yanc/releases  and extract its
    echo            bin\ folder into "%YANC_BIN%".
    exit /b 1
)
exit /b 0
