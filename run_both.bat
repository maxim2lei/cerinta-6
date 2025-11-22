@echo off
cd cmake-build-debug

REM Pornește process1 în fereastră nouă
start "Process 1" cmd /k process1.exe

REM Așteaptă 2 secunde
timeout /t 2 /nobreak > nul

REM Pornește process2 în fereastră nouă
start "Process 2" cmd /k process2.exe

echo Both processes started!
