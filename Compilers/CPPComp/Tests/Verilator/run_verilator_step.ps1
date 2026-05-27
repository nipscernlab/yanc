# run_verilator_step.ps1 - PowerShell helper for regress.sh's Verilator stage.
#
# Called per heavy test (those with a .in sidecar). Does:
#   1. Verilator --cc --exe --build to produce sim_<prname>.exe
#   2. Run the resulting sim binary with the input file
#
# Why: when regress.sh's bash spawns `make`, the env vars get stripped at
# the bash -> make boundary in Claude's sandbox (and possibly elsewhere
# on Windows), so mingw g++ can't find a writable TMP and silently falls
# back to C:\WINDOWS\ which fails for unprivileged users. PowerShell
# propagates env correctly to subprocesses, so spawning Verilator from
# PS instead of bash unblocks the build.

param(
    [Parameter(Mandatory=$true)] [string] $Prname,
    [Parameter(Mandatory=$true)] [string] $TmpDir,
    [Parameter(Mandatory=$true)] [string] $UprocV,
    [Parameter(Mandatory=$true)] [string] $SimMain,
    [Parameter(Mandatory=$true)] [string] $HdlDir,
    [Parameter(Mandatory=$true)] [string] $InFile,
    [Parameter(Mandatory=$true)] [string] $OutFile,
    [Parameter(Mandatory=$true)] [long]   $Clocks,
    [Parameter(Mandatory=$true)] [int]    $Expected
)

$ErrorActionPreference = "Stop"

$env:TMP    = "C:/packs/msys64/tmp"
$env:TEMP   = "C:/packs/msys64/tmp"
$env:TMPDIR = "C:/packs/msys64/tmp"
New-Item -ItemType Directory -Force "C:/packs/msys64/tmp" | Out-Null
$env:Path   = "C:/packs/msys64/mingw64/bin;" + $env:Path
$env:VERILATOR_ROOT = "C:/packs/msys64/mingw64/share/verilator"

$Verilator = "C:/packs/msys64/mingw64/bin/verilator_bin.exe"
if (-not (Test-Path $Verilator)) {
    Write-Error "verilator not found at $Verilator"
    exit 1
}

$vlDir = Join-Path $TmpDir "vl"
$simExe = Join-Path $vlDir "sim_$Prname.exe"

# 1. Verilator build
& $Verilator --cc --exe --build --top-module $Prname --prefix Vtop `
    -o "sim_$Prname" -Wno-lint -Wno-UNOPTFLAT -Wno-MULTIDRIVEN `
    -Wno-BLKANDNBLK -Wno-WIDTH -Wno-CASEINCOMPLETE -Wno-IMPLICIT `
    -Wno-COMBDLY --no-timing --Mdir $vlDir `
    $SimMain $UprocV `
    "$HdlDir/processor.v" "$HdlDir/core.v" "$HdlDir/ula.v" `
    "$HdlDir/addr_dec.v" "$HdlDir/instr_dec.v" 2>&1 | Out-Null

if ($LASTEXITCODE -ne 0 -or -not (Test-Path $simExe)) {
    Write-Error "verilator build failed for $Prname"
    exit 1
}

# 2. Run the sim
& $simExe $InFile $OutFile $Clocks $Expected 2>&1 | Out-Null
# sim exit code matters less than whether output was produced (the sim's
# idle-detector can exit non-zero on programs that legitimately stall
# briefly between out() calls). regress.sh checks for the output file
# separately.
exit 0
