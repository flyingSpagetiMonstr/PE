# Todos:

my test payload: 
function("printf")("Hallo, world!")
(use fopen to do file r/w) 

VS代码生成时关闭GS选项 命令行格式：/GS
use i_nt_headers.FileHeader.SizeOfOptionalHeader?

- read pyaload.exe[.payload] to target.exe[.virus]

- sizeofHeaders: The combined size of an MS-DOS stub, PE header, and section headers rounded up to a multiple of FileAlignment.
- SizeOfCode
- EntryPoint
- BaseOfCode
- size of the image: The size (in bytes) of the image, including all headers, as the image is loaded in memory. It must be a multiple of SectionAlignment.


# GDB manual
(gdb) run
(gdb) si
(gdb) ni
(gdb) si 5
(gdb) c
(gdb) info registers 
(gdb) info registers rax
(gdb) x/nfu addr (x/10cb addr)
- n specifies the number of units to display.
- f specifies the format. Common formats include x for hexadecimal, d for decimal, u for unsigned decimal, and c for character.
- u specifies the size of each unit (e.g., b for bytes, h for halfwords, and w for words).

(gdb) x/4xw &my_variable
(gdb) print my_variable
(gdb) layout asm
(gdb) display/i $pc
(gdb) x/5i $pc  

gcc -g(include debugging information)

# Qustions 
- Will the compiler optimize this code so that i_nt_headers.FileHeader.NumberOfSections gets loaded into register instead of reading memory every time?
  ```c
  for (int i = 0; i < i_nt_headers.FileHeader.NumberOfSections; i++)
  ```
- payload.c:16:1: warning: 'dllimport' attribute ignored [-Wattributes]
   16 | typedef  WINBASEAPI WINBOOL WINAPI (*tWriteConsoleA) (HANDLE hConsoleOutput,CONST VOID *lpBuffer,DWORD nNumberOfCharsToWrite,LPDWORD lpNumberOfCharsWritten,LPVOID lpReserved);
  This warning disappeared after:
  ```c
  // typedef  WINBASEAPI HANDLE WINAPI (*tGetStdHandle) (DWORD nStdHandle);
  // ->
  typedef  HANDLE  (*tGetStdHandle) (DWORD nStdHandle);
  ```

# Stalled
whether call_puts/syscall can be "PIC"
learn about syscall (better from sth like books)

# note
- when using GetStdHandle to print, freopen doesn't work (the message will be lost).
- When defining ptr to function types:
  ```c
  typedef  WINBASEAPI HANDLE WINAPI (*tGetStdHandle) (DWORD nStdHandle); // correct
  typedef  WINBASEAPI HANDLE WINAPI *tGetStdHandle (DWORD nStdHandle); // wrong
  ```
