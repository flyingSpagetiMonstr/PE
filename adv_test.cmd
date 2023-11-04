set name=PE-bear
@REM -bakup.exe

cd /d %~dp0

copy %name%-bakup.exe test\%name%.exe

gcc PE.c -o PE

gcc payload.c -o payload.exe

PE test\%name%.exe

@REM PAUSE

@REM %name%-target.exe

test\%name%.exe

