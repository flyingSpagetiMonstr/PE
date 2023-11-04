set name=void

cd /d %~dp0

@REM copy AES-bakup.exe AES.exe
copy %name%.exe test\

gcc PE.c -o PE

gcc payload.c -o payload.exe

PE test\%name%.exe

copy test.exe test\

cd test\
rmdir v_folder /s /q

%name%.exe

test.exe
