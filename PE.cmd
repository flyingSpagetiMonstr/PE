cd /d %~dp0


@REM copy AES-bakup.exe AES.exe
copy hallo_welt-bakup.exe hallo_welt.exe

del PE
gcc PE.c -o PE

@REM PE AES.exe
PE hallo_welt.exe

@REM PAUSE