@echo off
rem Takes this package's bin\ off your user PATH (undoes install.bat).
rem The files stay where they are: delete the folder to remove them.
setlocal
set "YANC_BIN=%~dp0bin"
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "$b = $env:YANC_BIN;" ^
  "$p = [Environment]::GetEnvironmentVariable('Path', 'User'); if (-not $p) { $p = '' };" ^
  "$keep = @($p.Split(';') | Where-Object { $_ -ne '' -and $_ -ne $b });" ^
  "[Environment]::SetEnvironmentVariable('Path', ($keep -join ';'), 'User');" ^
  "Write-Host 'Taken off the PATH:' $b"
endlocal
