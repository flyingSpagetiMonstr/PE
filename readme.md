## Manual
- Run payload.cmd to compile and run example.
- Requirement: gcc version 9.2.0 (x86_64-w64-mingw32)

## How it works 
- payload.exe is selfcontained, i.e., it reads all contents in its ".virus" section (only `main()` is not in this section), then writes it into other .exe files (with some minor modifications).
- the only thing that `main()` does is calling `payload()`: 
  ```c
  int main()
  {
      payload();
      exit(0);
  }
  ```

## Functions
1. after enterting `payload()`, it fistly gets the address of kernel32.dll (in memory) and retrieve function addresses from there. 
2. load msvcrt.dll. 
3. puts a test message `"_|Hallo|_"`. 
4. do `task1()`

- all infected executables will do the functions of payload.exe first and then goto its original entrypoint.



**********************************************************
### task1
- Create a folder and mov

___
You can jump to [My Section](#my-section) in this document.
### My-Section 

