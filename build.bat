@echo off
set CYGWIN_PATH=c:\cygwin
set DEVKITPPC=/C/devkitPro/devkitPPC
set DEVKITPRO=/C/devkitPro
make print
make %1

pause