cd /d %~dp0


@REM copy AES-bakup.exe AES.exe
copy hallo_welt-bakup.exe hallo_welt.exe

gcc PE.c -o PE

gcc payload.c -o payload.exe

PE hallo_welt.exe

@REM PAUSE