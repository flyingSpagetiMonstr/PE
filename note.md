0x957c8
<!-- 48 C7 C0 E0 14 00 00 -->
<!-- 48 C7 C0 C8 57 09 00 -->
<!-- 48C7C0C8570900 -->
<!-- 000957C8 -->

# Always check string literals
- 09.41
- 10.28


# Todos:
- VS代码生成时关闭GS选项 命令行格式：/GS
- A
  "mov $0x14E0, %%rax\n\t"
  "add %[base_addr], %%rax\n\t"
  "jmp *%%rax"
- reference
  <!-- - sizeofHeaders: The combined size of an MS-DOS stub, PE header, and section headers rounded up to a multiple of FileAlignment. -->
  - SizeOfCode 
  <!-- - EntryPoint -->
  <!-- - BaseOfCode -->
  <!-- - size of the image: The size (in bytes) of the image, including all headers, as the image is loaded in memory. It must be a multiple of SectionAlignment. -->
- check all headers to define whether its infected or not

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
- diff between fopen and CreateFile(mechanism, not only usage)
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
- calling CopyFileA will cause the original program can't exit(?) normally. (The program get stuck after running the host codes).

# Stalled
- use i_nt_headers.FileHeader.SizeOfOptionalHeader(?)

- whether call_puts/syscall can be "PIC"
- learn about syscall (better from sth like books)

# note
- when using GetStdHandle to print, freopen doesn't work (the message will be lost).
- When defining ptr to function types:
  ```c
  typedef  WINBASEAPI HANDLE WINAPI (*tGetStdHandle) (DWORD nStdHandle); // correct
  typedef  WINBASEAPI HANDLE WINAPI *tGetStdHandle (DWORD nStdHandle); // wrong
  ```
- Assertion failed: i_dos_header.e_lfanew == sizeof(i_dos_header), file PE.c, line 81
- can't read from gs:0(thread_env_block)
- For files open for update (those which include a "+" sign), on which both input and output operations are allowed, the stream shall be flushed (fflush) or repositioned (fseek, fsetpos, rewind) before a reading operation that follows a writing operation. The stream shall be repositioned (fseek, fsetpos, rewind) before a writing operation that follows a reading operation (whenever that operation did not reach the end-of-file).
- a label can only be part of a statement and a declaration is not a statement
- Put similar codes together, tightly.
- content of char *target(findFileData.cFileName) dies(become all zero) after fseek().
- 

