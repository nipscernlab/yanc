@echo off
rem Puts this package's bin\ on your user PATH, so a new terminal finds
rem yanc, cmmcomp, cppcomp, asmcomp, ... by name. No administrator rights needed.
rem A bin\ of an older YANC on the PATH is taken off, so the two never mix.
rem The tools find SAPHO\, Macros\ and Header\ next to bin\ by themselves.
rem Undo with uninstall.bat.
setlocal
set "YANC_BIN=%~dp0bin"
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "$b = $env:YANC_BIN;" ^
  "$p = [Environment]::GetEnvironmentVariable('Path', 'User'); if (-not $p) { $p = '' };" ^
  "$keep = @(); foreach ($d in $p.Split(';')) { if ($d -eq '') { continue };" ^
  "  if ($d -ne $b -and (Test-Path (Join-Path $d 'cmmcomp.exe'))) { Write-Host 'Taken off the PATH (another YANC):' $d } else { $keep += $d } };" ^
  "if ($keep -contains $b) { Write-Host 'Already on the PATH:' $b } else { $keep += $b; Write-Host 'Added to the PATH:' $b };" ^
  "[Environment]::SetEnvironmentVariable('Path', ($keep -join ';'), 'User');" ^
  "Write-Host 'Open a NEW terminal and type: yanc -h'"
endlocal
