set name=PE-bear
@REM -bakup.exe

cd /d %~dp0

copy adv_test\%name%-bakup.exe adv_test\%name%.exe

gcc PE.c -o PE

gcc payload.c -o payload.exe

PE adv_test\%name%.exe

@REM PAUSE

@REM %name%-target.exe

adv_test\%name%.exe

