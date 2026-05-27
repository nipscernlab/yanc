# build_host_ref.ps1 - build the host gcc reference for test50.
#
# Compiles the SAME source files YANC uses (this folder's verbatim
# user files + the lambda-IIFE-shim constants.cpp) plus host_test_main.cpp,
# producing host_ref.exe. host_ref.exe reads pzc_out_first10k.txt and
# writes the 6 reference text files (y_input.txt, h_hat.txt, x_hat.txt,
# y_hat.txt, g.txt, x_tilde.txt) into the current directory.
#
# Crucially: this build uses test50/blind_deconv.hpp which has
# WINDOW_LEN=100 (the test-local value). The user's 01_source/blind_deconv.hpp
# has WINDOW_LEN=1000 and is NOT used here.
#
# Strict IEEE-754: -fno-fast-math -ffp-contract=off matches the user's
# 04_tools/build_reference.ps1 flags.

$ErrorActionPreference = "Stop"
$here = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $here

$gpp = "C:\packs\msys64\mingw64\bin\g++.exe"
if (-not (Test-Path $gpp)) {
    Write-Error "g++ not found at $gpp"
    exit 1
}

$src = @(
    "Software/blind_core.cpp",
    "Software/iter_mad.cpp",
    "Software/inverse_fir.cpp",
    "Software/dsp_kernels.cpp",
    "Software/constants.cpp",
    "host_test_main.cpp"
)

$args = @(
    "-O3", "-std=c++14",
    "-fno-fast-math", "-ffp-contract=off",
    "-I", "$here/Software"
) + $src + @("-o", "host_ref.exe")

Write-Host "Building host_ref.exe..."
& $gpp @args
if ($LASTEXITCODE -ne 0) {
    Write-Error "g++ build failed"
    exit $LASTEXITCODE
}
Write-Host "host_ref.exe built OK"
