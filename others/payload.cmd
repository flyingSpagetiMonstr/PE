@REM ignore hallo_welt-target.exe

cd /d %~dp0

rmdir v_folder /s /q

gcc .\..\payload.c -o payload.exe

gcc .\test.c -o test.exe
payload.exe

gcc .\void.c -o void.exe
test.exe

void.exe

del payload.exe
del void.exe
del test.exe
