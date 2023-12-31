# Always check string literals

# Todos:
- reference
  <!-- - sizeofHeaders: The combined size of an MS-DOS stub, PE header, and section headers rounded up to a multiple of FileAlignment. -->
  <!-- - SizeOfCode  -->
  <!-- - EntryPoint -->
  <!-- - BaseOfCode -->
  <!-- - size of the image: The size (in bytes) of the image, including all headers, as the image is loaded in memory. It must be a multiple of SectionAlignment. -->
- freopen stdout > NUL (?)
- SizeOfHeaders
  The combined size of the following items, rounded to a multiple of the value specified in the FileAlignment member.
  e_lfanew member of IMAGE_DOS_HEADER
  4 byte signature
  size of IMAGE_FILE_HEADER
  size of optional header
  size of all section headers
- how win PE loaders check if PE is valid
- (file routes in payload.c depands on current cmd route.)

# GDB manual
- (gdb) run
- (gdb) si
- (gdb) ni
- (gdb) si 5
- (gdb) c /continue
- (gdb) info registers 
- (gdb) info registers rax
- (gdb) x/nfu addr (x/10cb addr)
- (gdb) x/4xw &my_variable
- (gdb) print my_variable
- (gdb) layout asm
- (gdb) display/i $pc
- (gdb) x/5i $pc  
- gcc -g(include debugging information)
- n specifies the number of units to display.
- f specifies the format. Common formats include x for hexadecimal, d for decimal, u for unsigned decimal, and c for character.
- u specifies the size of each unit (e.g., b for bytes, h for halfwords, and w for words).


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
- /d: The /d option is used with the cd command to change both the current drive and the current directory. Without it, cd would only change the current directory on the current drive.

  %~dp0: This is a special batch script variable that represents the path of the batch script itself.
  - %0: This represents the batch script's name, which is typically the first argument passed to the script. In this case, it's the name of the batch file.
  - %~d0: This extracts the drive letter from the path of the batch file.
  - %~p0: This extracts the directory (or path) of the batch file.
- https://sebastiandedeyne.com/setting-up-a-global-gitignore-file/
- git update-index --assume-unchanged <file-path>
- system() takes route of current cmd instead of the position of .exe file as "."