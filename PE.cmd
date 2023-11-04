set name=hallo_welt

cd /d %~dp0

rmdir /s v_folder /q

@REM copy AES-bakup.exe AES.exe
copy %name%.exe %name%-target.exe

gcc PE.c -o PE

gcc payload.c -o payload.exe

PE %name%-target.exe

@REM PAUSE

%name%-target.exe