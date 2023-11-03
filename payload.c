// https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#export-directory-table

#include <stdio.h>
#include <string.h>
#include <Windows.h>
#include <winternl.h>

#define BUFFER_SIZE 1024

// #define KERNEL32_DLL 3
// #define CURRENT_EXE 1

// #define IMAGE_BASE  ((void *)0x00007FF8827B0000) // Changes when PC is restarted (?)

typedef HANDLE (*tGetStdHandle) (DWORD);
typedef WINBOOL (*tWriteConsoleA) (HANDLE, CONST VOID *, DWORD, LPDWORD, LPVOID);
typedef HMODULE (*tLoadLibraryA)(LPCSTR);
typedef FARPROC (*tGetProcAddress)(HMODULE, LPCSTR);

typedef int (*t_puts)(const char *);

void payload(void) 
__attribute__((section(".inject"))) __attribute__((used));

void *get_image_base(wchar_t *module_name, LIST_ENTRY *list) 
__attribute__((section(".inject")));

IMAGE_EXPORT_DIRECTORY *get_IMAGE_EXPORT_DIRECTORY(void *image_base) 
__attribute__((section(".inject")));
void *get_kernel_32_func(char *function_name, IMAGE_EXPORT_DIRECTORY* i_ex_dir, void *image_base) 
__attribute__((section(".inject")));

int compare(const char* a, const char* b) 
__attribute__((section(".inject")));
size_t length(const char* str) 
__attribute__((section(".inject")));
int wcompare(const wchar_t *a, const wchar_t *b) 
__attribute__((section(".inject")));

#define FUNCTION(name) ((t_##name)pGetProcAddress(msvcrt_hModule, s_##name))

#define LOAD(name) t##name p##name = (t##name)get_kernel_32_func(s##name, i_ex_dir, image_base)

int main()
{
    payload();
    return 0;
}

void payload(void)
{
    char sGetStdHandle[] = "GetStdHandle";
    char sWriteConsoleA[] = "WriteConsoleA";
    char sLoadLibraryA[] = "LoadLibraryA";
    char sGetProcAddress[] = "GetProcAddress";

    char s_puts[] = "puts";

    const char message[] = "This is the payload!!!!!!!!!!!!!!!!!!!!!!!!\n"; // !: 0x21 
    wchar_t kernel_32_name[] = L"KERNEL32.DLL";
    
    // get_InMemoryOrderModuleList
    LIST_ENTRY *list = 0;
    {
        PEB *peb = 0; 
        asm volatile ("movq %%gs:0x60, %0":"=r" (peb));
        list = &(peb->Ldr->InMemoryOrderModuleList);
    }
    
    void *image_base = get_image_base(kernel_32_name, list);
    // ############################################## function("printf")("Hallo, world!")

    IMAGE_EXPORT_DIRECTORY *i_ex_dir = get_IMAGE_EXPORT_DIRECTORY(image_base);

    LOAD(GetStdHandle);
    LOAD(WriteConsoleA);
    LOAD(LoadLibraryA);
    LOAD(GetProcAddress);

    // prt
    HANDLE hConsole = pGetStdHandle(STD_OUTPUT_HANDLE);
    DWORD charsWritten;
    pWriteConsoleA(hConsole, message, length(message), &charsWritten, NULL);

    // load msvcrt
    char s_msvcrt[] = "msvcrt.dll";
    HMODULE msvcrt_hModule = pLoadLibraryA(s_msvcrt);  // Load the C runtime library (msvcrt.dll)

    char msg[] = "puts succeed";
    char another[] = "another";

    FUNCTION(puts)(msg);
    FUNCTION(puts)(another);

    // go back to host code
    list = list->Flink;
    void *current_image_base = *((void**)((void*)list + 0x20));

    asm volatile (
        "mov $0x14E0, %%rax\n\t"
        "add %[base_addr], %%rax\n\t"
        "jmp *%%rax"
        : 
        : [base_addr] "r" (current_image_base) 
        : "rax"
    );

}

IMAGE_EXPORT_DIRECTORY *get_IMAGE_EXPORT_DIRECTORY(void *image_base)
{
    IMAGE_DOS_HEADER *i_dos_header = (IMAGE_DOS_HEADER *)image_base;

    IMAGE_NT_HEADERS *i_nt_headers = (IMAGE_NT_HEADERS *)(image_base + i_dos_header->e_lfanew);
    
    DWORD EX_RVA = i_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    IMAGE_EXPORT_DIRECTORY* EX_VA = (IMAGE_EXPORT_DIRECTORY *)(image_base + EX_RVA);
    return EX_VA;
}

void *get_kernel_32_func(char *function_name, IMAGE_EXPORT_DIRECTORY* i_ex_dir, void *image_base)
{
    DWORD *name_table = (DWORD*)(i_ex_dir->AddressOfNames + image_base);

    int serial = 0;
    for (; serial < i_ex_dir->NumberOfFunctions; serial++)
    {
        // puts((char*)(name_table[serial]+image_base));
        if(compare((char*)(name_table[serial]+image_base), function_name) == 0)
        {
            break;
        }
    }

    WORD *ordinal = (WORD*)(i_ex_dir->AddressOfNameOrdinals + image_base);
    DWORD *func_rvas = (DWORD*)(i_ex_dir->AddressOfFunctions + image_base);

    return (void*)(func_rvas[ordinal[serial]] + image_base);
}

void *get_image_base(wchar_t *module_name, LIST_ENTRY *list)
{
    for (int i = 0; i < 10; i++)
    {
        LDR_DATA_TABLE_ENTRY *entry = (LDR_DATA_TABLE_ENTRY *)((void*)list - 0x10);

        wchar_t *buffer = ((UNICODE_STRING*)(entry->Reserved4))->Buffer;  

        if (buffer != NULL && wcompare(module_name, buffer) == 0)
        {
            // printf("%d: ,DllBase: %X\n", i, entry->DllBase);
            // wprintf(L"%ls\n", ((UNICODE_STRING*)(entry->Reserved4))->Buffer);
            return entry->DllBase;
        }
        list = list->Flink;
    }
}

#include "string.c"
