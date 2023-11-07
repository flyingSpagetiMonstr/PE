## Intro
- This project is for an assignment of my software security course at 3rd year.
- A simple program that will inject itself into other PE+ files (executables) that are in the same directory with it. Executables being injected will do the same things as payload.exe first, and then do its original job. 
- Currently it works well with my executables that are generated by gcc from .c files, but for "normal" .exe files, such as a starter of a game, the target .exe file just, seems, to be corrupted after being injected. Also Windows seems to be no longer recognizing it as a valid executable. 
- However for "side" executables like KillPot64.exe of PotPlayer and UnityCrashHandler64.exe of many unity games, the result is successful.

## Manual
- requirements: gcc version 9.2.0 (x86_64-w64-mingw32)
- run payload.cmd to compile and run example. 
- toggle `#define DO_TASK1 0` to determine whether to perform task1 or not. 

## How it works 
- payload.exe is selfcontained, i.e., it reads all contents in its `".virus"` (macro `V_SECT_NAME`) section (only `main()` is not in this section), then writes it into other .exe files (with some minor modifications).
- the only thing that `main()` does is calling `payload()`: 
  ```c
  int main()
  {
      payload();
      exit(0);
  }
  ```

## Breakdown
1. after enterting `payload()`, it fistly gets the address of kernel32.dll (in memory) and retrieve function addresses from there. 
2. load msvcrt.dll. 
3. puts a test message `"_|Hallo|_"`. 
4. do [`task1()`](#task1)
5. do [`infect()`](#infect) (which is the 2nd task of the assignment)
6. jmp to the original entrypoint of infected program.


***********************************************************

### task1
1. search .txt file under current directory and randomly select one. 
2. create a folder and copy the selected .txt file into it (the name of new file is macro `NAME_OF_NEW_FILE`).

### infect
1. do `get_payload()`: read from the .exe file itself, store the payload and return it with its size.
2. calculate in-file offset of the final jmp instruction.
3. search .exe file under current directory and do `inject()` to inject payload into file.
