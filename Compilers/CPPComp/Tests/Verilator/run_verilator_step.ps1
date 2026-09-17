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

# Stop on CMDLET errors (a missing file, a bad path): those are real.
$ErrorActionPreference = "Stop"

# ...but NOT on what a NATIVE tool writes to stderr. PowerShell 5.1 wraps every
# stderr line of an .exe into a NativeCommandError, and under "Stop" that is a
# TERMINATING error: the script dies before its own exit 0, and regress.sh
# reports "FAIL (<test>): verilator" for a build that actually succeeded. The
# trigger is any tool that merely WARNS -- e.g. g++ printing the `dllimport`
# warnings from Verilator's own svdpi.h whenever the model needs the DPI
# plumbing (a public signal in the HDL), which only happens on a FRESH build:
# an incremental one compiles nothing, prints nothing, and passed. That cost a
# whole debugging session, so the two native calls below run with
# $ErrorActionPreference = "Continue" (see native_quiet) and are judged the way
# they always should have been: by $LASTEXITCODE and by the artifact.

# Runs a native command with stderr demoted to normal output, and returns its
# exit code. Output is discarded here (regress.sh compares the output file);
# to see it while debugging, drop the redirect.
function Invoke-Native {
    param([string] $Exe, [string[]] $Arguments)
    $previous = $ErrorActionPreference
    $ErrorActionPreference = "Continue"
    try {
        & $Exe @Arguments 2>&1 | Out-Null
        return $LASTEXITCODE
    } finally {
        $ErrorActionPreference = $previous
    }
}

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
$buildCode = Invoke-Native $Verilator @(
    "--cc", "--exe", "--build", "--top-module", $Prname, "--prefix", "Vtop",
    "-o", "sim_$Prname", "-Wno-lint", "-Wno-UNOPTFLAT", "-Wno-MULTIDRIVEN",
    "-Wno-BLKANDNBLK", "-Wno-WIDTH", "-Wno-CASEINCOMPLETE", "-Wno-IMPLICIT",
    "-Wno-COMBDLY", "--no-timing", "--Mdir", $vlDir,
    $SimMain, $UprocV,
    "$HdlDir/processor.v", "$HdlDir/core.v", "$HdlDir/ula.v",
    "$HdlDir/addr_dec.v", "$HdlDir/instr_dec.v")

if ($buildCode -ne 0 -or -not (Test-Path $simExe)) {
    Write-Error "verilator build failed for $Prname (exit $buildCode)"
    exit 1
}

# 2. Run the sim
Invoke-Native $simExe @($InFile, $OutFile, "$Clocks", "$Expected") | Out-Null
# sim exit code matters less than whether output was produced (the sim's
# idle-detector can exit non-zero on programs that legitimately stall
# briefly between out() calls). regress.sh checks for the output file
# separately.
exit 0
