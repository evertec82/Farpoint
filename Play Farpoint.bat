@echo off
powershell.exe -NoProfile -STA -ExecutionPolicy Bypass -File "%~dp0pc-vr\launch-farpoint.ps1" %*
if errorlevel 1 pause
