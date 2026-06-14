# run_test50.ps1 - standalone PowerShell runner for test50.
#
# Sidestep: the Bash-tool environment's PATH/TMP combination causes mingw
# g++ to fail silently inside Verilator-generated makefiles ("Cannot create
# temporary file in C:\WINDOWS\"). Same toolchain works fine from PowerShell
# with TMP/TEMP set to a writable Windows path. So this script runs the full
# CPPComp -> Verilator -> sim chain through PowerShell, then hands the
# resulting output_0.txt to the Python comparison script.

$ErrorActionPreference = "Stop"

$ROOT  = Resolve-Path "$PSScriptRoot\..\..\..\.." | Select-Object -ExpandProperty Path
$CPP   = Join-Path $ROOT "Compilers/CPPComp"
# Prefer the regress-built binaries: regress.sh burns the real 32-bit target
# (CFG_NUBITS=32 etc.) into .smoke/bin, while a plain `make` leaves bin/ at
# config.h's 16-bit default -- which would silently skew this comparison.
$BIN   = Join-Path $ROOT ".smoke/bin"
if (-not (Test-Path (Join-Path $BIN "cppcomp.exe"))) { $BIN = Join-Path $ROOT "bin" }
$WORK  = Join-Path $CPP  ".work"
$TEST  = $PSScriptRoot
$SW    = Join-Path $TEST "Software"
$HDL   = Join-Path $ROOT "HDL"
$MACROS= Join-Path $ROOT "Compilers/CMMComp/Includes"

$env:TMP    = "C:/packs/msys64/tmp"
$env:TEMP   = "C:/packs/msys64/tmp"
$env:TMPDIR = "C:/packs/msys64/tmp"
New-Item -ItemType Directory -Force "C:/packs/msys64/tmp" | Out-Null

$env:Path = "C:/packs/msys64/mingw64/bin;" + $env:Path
$env:VERILATOR_ROOT = "C:/packs/msys64/mingw64/share/verilator"

$CPPPP   = Join-Path $BIN "cpppp.exe"
$CPPC    = Join-Path $BIN "cppcomp.exe"
$APPCOMP = Join-Path $BIN "appcomp.exe"
$ASMCOMP = Join-Path $BIN "asmcomp.exe"
$VERILATOR = "C:/packs/msys64/mingw64/bin/verilator_bin.exe"
$SIMMAIN = Join-Path $CPP "Tests/Verilator/sim_main.cpp"

foreach ($x in @($CPPPP, $CPPC, $APPCOMP, $ASMCOMP, $VERILATOR)) {
    if (-not (Test-Path $x)) { Write-Error "missing: $x"; exit 1 }
}

$base    = "test50"
$src_dir = $SW
$src     = Join-Path $SW "$base.cpp"
$prname  = "test50"

$proc = Join-Path $WORK $base
$tmp  = Join-Path $proc "_tmp"
if (Test-Path $proc) {
    # Try to clean, but tolerate locked artifacts (Windows holds handles on
    # recently-spawned exes briefly). Fall through if rm partially fails.
    Remove-Item -Recurse -Force -ErrorAction SilentlyContinue $proc
    if (Test-Path $proc) {
        Start-Sleep -Milliseconds 500
        Remove-Item -Recurse -Force -ErrorAction SilentlyContinue $proc
    }
}
New-Item -ItemType Directory -Force "$proc/Software", "$proc/Hardware", "$proc/Simulation", $tmp | Out-Null

$asm = "$proc/Software/$prname.asm"

Write-Host "==> cpppp"
& $CPPPP -i $src -o "$tmp/pp.cpp" -I "$CPP/Includes" -I $src_dir
if ($LASTEXITCODE -ne 0) { exit 1 }

Write-Host "==> cppcomp"
# cppcomp >= v5.2 dropped -o for the cmmcomp-style layout: -p <proc> -n <prname>
# writes Software/<prname>.asm into the proc folder ($asm above).
& $CPPC -i "$tmp/pp.cpp" -p $proc -n $prname -t $tmp
if ($LASTEXITCODE -ne 0) { exit 1 }

Write-Host "==> appcomp"
& $APPCOMP -en -i $asm -t $tmp
if ($LASTEXITCODE -ne 0) { exit 1 }

Write-Host "==> asmcomp"
& $ASMCOMP -en -i $asm -p $proc -d $HDL -m $MACROS -t $tmp -f 100 -c 5000000
if ($LASTEXITCODE -ne 0) { exit 1 }

$uproc_v = "$proc/Hardware/$prname.v"
if (-not (Test-Path $uproc_v)) { Write-Error "missing $uproc_v"; exit 1 }

Write-Host "==> verilator build"
& $VERILATOR --cc --exe --build --top-module $prname --prefix Vtop `
    -o "sim_$prname" -Wno-lint -Wno-UNOPTFLAT -Wno-MULTIDRIVEN `
    -Wno-BLKANDNBLK -Wno-WIDTH -Wno-CASEINCOMPLETE -Wno-IMPLICIT `
    -Wno-COMBDLY --no-timing --Mdir "$tmp/vl" `
    $SIMMAIN $uproc_v "$HDL/processor.v" "$HDL/core.v" "$HDL/ula.v" `
    "$HDL/addr_dec.v" "$HDL/instr_dec.v"
if ($LASTEXITCODE -ne 0) { Write-Error "verilator failed"; exit 1 }

$infile = "$src_dir/$base.in"
$out    = "$proc/Simulation/output_0.txt"
$clocks_file = "$src_dir/$base.clocks"
$clocks = 200000000
if (Test-Path $clocks_file) {
    $clocks = (Get-Content $clocks_file -Raw).Trim()
}
$expected = 509

Write-Host "==> running sim (max_cycles=$clocks, expected_outputs=$expected)"
Write-Host "    output -> $out"
$sw = [System.Diagnostics.Stopwatch]::StartNew()
& "$tmp/vl/sim_$prname.exe" $infile $out $clocks $expected
$sw.Stop()
Write-Host "==> sim done in $($sw.Elapsed.TotalSeconds.ToString('0.0'))s"

if (-not (Test-Path $out)) { Write-Error "no output produced"; exit 1 }
$actual = (Get-Content $out | Measure-Object -Line).Lines
Write-Host "==> got $actual lines of output (expected $expected)"

Write-Host "==> compare_to_reference.py"
& "C:/packs/msys64/mingw64/bin/python3.exe" `
    "$TEST/compare_to_reference.py" `
    $out `
    "$TEST/host_ref_outputs" `
    --gain-out 1000000 `
    --abs-tol 1e-2 `
    --rel-tol 5e-2 `
    --write-dir "$proc/yanc_outputs"
exit $LASTEXITCODE
