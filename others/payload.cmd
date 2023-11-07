@ REM inject to void.exe and run void.exe 

@ cd /d %~dp0

@ rmdir v_folder /s /q

gcc .\..\payload.c -o payload.exe

@ gcc .\void.c -o void.exe

payload.exe

@ gcc .\void.c -o test.exe
void.exe

test.exe

@ del payload.exe
@ del void.exe
@ del test.exe
