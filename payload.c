// https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#export-directory-table

#include <stdio.h>
#include <string.h>
#include <Windows.h>
#include <winternl.h>

// #define KERNEL32_DLL 3
// #define CURRENT_EXE 1

#define SET_SECT __attribute__((section(".inject")))

#define EXE_FILE_NAME "hallo_welt.exe"

// ==========
typedef HMODULE (*tLoadLibraryA)(LPCSTR);
typedef FARPROC (*tGetProcAddress)(HMODULE, LPCSTR);
typedef WINBOOL (*tFreeLibrary) (HMODULE);

// ==========
void payload(void) SET_SECT __attribute__((used));

void *get_image_base(wchar_t *module_name, LIST_ENTRY *list) SET_SECT;

IMAGE_EXPORT_DIRECTORY *get_IMAGE_EXPORT_DIRECTORY(void *image_base) SET_SECT;
void *get_kernel_32_func(char *function_name, IMAGE_EXPORT_DIRECTORY* i_ex_dir, void *image_base) SET_SECT;

// ==========
int compare(const char* a, const char* b) SET_SECT;
size_t length(const char* str) SET_SECT;
int wcompare(const wchar_t *a, const wchar_t *b) SET_SECT;

// ==========
#define FUNCTION(name) ((t_##name)pGetProcAddress(msvcrt_hModule, s_##name))
#define LOAD(name) t##name p##name = (t##name)get_kernel_32_func(s##name, i_ex_dir, image_base)

int main()
{
    payload();
    return 0;
}

void payload(void)
{
    // asm ("start_label:");

    char sLoadLibraryA[] = "LoadLibraryA";
    char sGetProcAddress[] = "GetProcAddress";
    char sFreeLibrary[] = "FreeLibrary";
    
    // FindFirstFileA
    // FindNextFileA
    // FindClose
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

    IMAGE_EXPORT_DIRECTORY *i_ex_dir = get_IMAGE_EXPORT_DIRECTORY(image_base);

    LOAD(LoadLibraryA);
    LOAD(GetProcAddress);
    LOAD(FreeLibrary);

    // load msvcrt
    char s_msvcrt[] = "msvcrt.dll";
    HMODULE msvcrt_hModule = pLoadLibraryA(s_msvcrt);  // Load the C runtime library (msvcrt.dll)


// do sth.
    // test
    typedef int (*t_puts)(const char *);
    char s_puts[] = "puts";
    typedef int (*t_printf) (const char *, ...);
    char s_printf[] = "printf";

    char msg[] = "_|Hallo|_";
    FUNCTION(puts)(msg);

    // ===========================================
    // File 

    // GetModuleFileNameW
    typedef FILE *(*t_fopen)(const char *,const char *);
    char s_fopen[] = "fopen";
    typedef size_t (*t_fread)(void *, size_t, size_t, FILE *);
    char s_fread[] = "fread";
    typedef int (*t_fseek)(FILE *, long, int);
    char s_fseek[] = "fseek";
    typedef int (*t_fclose)(FILE *);
    char s_fclose[] = "fclose";

    char host_name[] = EXE_FILE_NAME; 
    char mode[] = "rb";
    
    FILE *host = FUNCTION(fopen)(host_name, mode);

    
    // get nt_header
    IMAGE_DOS_HEADER i_dos_header = {0};
    FUNCTION(fread)(&i_dos_header, 1, sizeof(i_dos_header), host);
    IMAGE_NT_HEADERS i_nt_headers = {0};
    FUNCTION(fseek)(host, i_dos_header.e_lfanew, SEEK_SET);
    FUNCTION(fread)(&i_nt_headers, 1, sizeof(i_nt_headers), host);

    FUNCTION(fclose)(host);
    // End file
    // ===========================================

    //  calculating in-file offset of instruction mov /(rva)/, %%rax
    DWORD offset = 0;
    asm volatile(
        "leal payload, %%eax\n\t"
        "leal key_label, %%ebx\n\t"
        "subl %%eax, %%ebx\n\t"
        "movl %%ebx, %0\n\t"
        :"=r" (offset)
        :
        :"eax", "ebx"
    );

    offset += 3;
    char format[] = "offset: %X\n"; 
    FUNCTION(printf)(format, offset);

    pFreeLibrary(msvcrt_hModule);
// done

// go back to host code
    list = list->Flink;
    void *current_image_base = *((void**)((void*)list + 0x20));

    asm volatile (
        // "xor %%rax, %%rax\n\t"
        "key_label: \n\t"
        "mov $0x000014E0, %%rax\n\t"
        // "mov $0x1234ABCD, %%rax\n\t"
        "add %[base_addr], %%rax\n\t"
        "jmp *%%rax"
        :
        :[base_addr] "r" (current_image_base)
        //  [entry_rva] "r" (i_nt_headers.OptionalHeader.AddressOfEntryPoint) 
        :"rax"
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
