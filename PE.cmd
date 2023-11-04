set name=hallo_welt

cd /d %~dp0

@REM copy AES-bakup.exe AES.exe
copy %name%.exe %name%-target.exe

gcc PE.c -o PE

gcc payload.c -o payload.exe

PE %name%-target.exe

@REM PAUSE

@REM %name%-target.exe

copy %name%-target.exe test\
copy AES.exe test\
copy void.exe test\
copy test.exe test\

cd test\
rmdir v_folder /s /q
%name%-target.exe

void.exe
test.exe
