@REM ignore hallo_welt-target.exe

cd /d %~dp0

gcc payload.c -o payload.exe

copy payload.exe test\payload.exe
del payload.exe

copy AES.exe test\
copy void.exe test\
copy test.exe test\

cd test\

payload.exe
test.exe
