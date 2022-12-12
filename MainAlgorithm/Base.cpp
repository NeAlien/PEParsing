#include "Windows.h"
#include <iostream>
#include <fstream>
#include <string>

#define ALIGN_DOWN(x, align)            (x & ~(align-1)) //выравнивание вниз
#define ALIGN_UP(x, align)              ((x & (align-1))?ALIGN_DOWN(x,align)+align:x) //выравнивание вверх

// функция перевода из RVA в Offset
DWORD RVAtoOffset(DWORD Base, DWORD RVA)
{
    PIMAGE_NT_HEADERS BaseOfPE = (PIMAGE_NT_HEADERS)((long)Base + ((PIMAGE_DOS_HEADER)Base)->e_lfanew);
    short NumberOfSection = BaseOfPE->FileHeader.NumberOfSections;
    long SectionAlign = BaseOfPE->OptionalHeader.SectionAlignment;
    PIMAGE_SECTION_HEADER Section = (PIMAGE_SECTION_HEADER)(BaseOfPE->FileHeader.SizeOfOptionalHeader + (long)&(BaseOfPE->FileHeader) + sizeof(IMAGE_FILE_HEADER));
    long VirtualAddress, PointerToRawData;
    bool flag = false;
    for (int i = 0; i < NumberOfSection; i++)
    {
        if ((RVA >= (Section->VirtualAddress)) && (RVA < ALIGN_UP(Section->VirtualAddress + Section->Misc.VirtualSize, BaseOfPE->OptionalHeader.SectionAlignment)))
        {
            VirtualAddress = Section->VirtualAddress;
            PointerToRawData = Section->PointerToRawData;
            flag = true;
            break;
        }
        Section++;
    }
    if (flag) return RVA - VirtualAddress + PointerToRawData;
    else return RVA;
}


int main(int argc, char* argv[]) {

    // сохранение в файл для дальнейшей обработки
    freopen("OutputOfParsing.txt", "w", stdout);

    const int MAX_FILEPATH = 255;
    char fileName[MAX_FILEPATH] = { 0 };
    memcpy_s(&fileName, MAX_FILEPATH, argv[1], MAX_FILEPATH);
    HANDLE file = NULL;
    DWORD fileSize = NULL;
    DWORD bytesRead = NULL;
    LPVOID fileData = NULL;
    PIMAGE_DOS_HEADER dosHeader = {};
    PIMAGE_NT_HEADERS imageNTHeadersExport = {};
    PIMAGE_NT_HEADERS imageNTHeadersImport = {};
    PIMAGE_SECTION_HEADER sectionHeaderImport = {};
    PIMAGE_SECTION_HEADER exportSection = {};
    PIMAGE_SECTION_HEADER importSection = {};
    IMAGE_EXPORT_DIRECTORY* exportDirectory = {};
    IMAGE_IMPORT_DESCRIPTOR* importDescriptor = {};
    PIMAGE_FILE_HEADER fileHeaderImport = {};
    PIMAGE_OPTIONAL_HEADER optionalHeaderImport = {};
    WORD* AddressOfNameOrdinals = {};
    DWORD* AddressOfFunctions = {};
    DWORD* AddressOfNames = {};
    PIMAGE_THUNK_DATA thunkData = {};
    DWORD thunk = NULL;
    DWORD rawOffset = NULL;
    DWORD NameOfDll = NULL;
    DWORD OrdinalOfImportedFunctions = NULL;
    DWORD NameOfImportedFunctions = NULL;
    LPDWORD addressOfnames = NULL;

    // открываем файл
    file = CreateFileA(fileName, GENERIC_ALL, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) printf("Could not read file");

    // распределяем "heap" (кучу)
    fileSize = GetFileSize(file, NULL);
    fileData = HeapAlloc(GetProcessHeap(), 0, fileSize);

    // считываем байты файла в память 
    ReadFile(file, fileData, fileSize, &bytesRead, NULL);

    // IMAGE_DOS_HEADER
    dosHeader = (PIMAGE_DOS_HEADER)fileData;

    // IMAGE_NT_HEADERS
    imageNTHeadersExport = (PIMAGE_NT_HEADERS)((DWORD)fileData + dosHeader->e_lfanew);
    imageNTHeadersImport = (PIMAGE_NT_HEADERS)((DWORD)fileData + dosHeader->e_lfanew);

    // IMAGE_FILE_HEADER
    fileHeaderImport = (PIMAGE_FILE_HEADER)((PUCHAR)imageNTHeadersImport + 4);

    // IMAGE_OPTIONAL_HEADER
    optionalHeaderImport = (PIMAGE_OPTIONAL_HEADER)((PUCHAR)fileHeaderImport + IMAGE_SIZEOF_FILE_HEADER);

    // смещаемся к первому "section header"
    DWORD sectionLocationImport = (DWORD)imageNTHeadersImport + sizeof(DWORD) + (DWORD)(sizeof(IMAGE_FILE_HEADER)) + (DWORD)imageNTHeadersImport->FileHeader.SizeOfOptionalHeader;
    DWORD sectionSizeImport = (DWORD)sizeof(IMAGE_SECTION_HEADER);



    //*******************************************************************************************************************************************************************************************************************************************************


    // проверяем на наличие таблицы экспорта
    if (!imageNTHeadersExport->OptionalHeader.DataDirectory[0].VirtualAddress) {

        printf("EXPORTS OF FILE:\n\n");

        printf("File has no export functions\n");

    }
    else if (!imageNTHeadersExport->OptionalHeader.DataDirectory[0].Size) {

        printf("EXPORTS OF FILE:\n\n");

        printf("File has no export functions\n");
    }
    else {

        printf("EXPORTS OF FILE:\n\n");

        // получаем RVA адрес таблицы экспорта
        DWORD ExportDirectoryRVA = imageNTHeadersExport->OptionalHeader.DataDirectory[0].VirtualAddress;

        // переводим RVA адрес в Offset
        PIMAGE_EXPORT_DIRECTORY ExportDirectory = (PIMAGE_EXPORT_DIRECTORY)RVAtoOffset((long)fileData, ExportDirectoryRVA);
        ExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((long)ExportDirectory + (long)fileData);

        // считываем данные из AddressOfNameOrdinals
        AddressOfNameOrdinals = (unsigned short*)RVAtoOffset((long)fileData, ExportDirectory->AddressOfNameOrdinals);
        AddressOfNameOrdinals = (WORD*)((long)AddressOfNameOrdinals + (long)fileData);

        // считываем данные из AddressOfNames
        DWORD* AddressOfNames = (unsigned long*)RVAtoOffset((long)fileData, ExportDirectory->AddressOfNames);
        AddressOfNames = (DWORD*)((long)AddressOfNames + (long)fileData);

        // считываем данные из AddressOfFunctions
        AddressOfFunctions = (unsigned long*)RVAtoOffset((long)fileData, ExportDirectory->AddressOfFunctions);
        AddressOfFunctions = (DWORD*)((long)AddressOfFunctions + (long)fileData);

        WORD index;

        // т.к нужные нам названия экспортируемых функций зависят от количества функций в структуре IMAGE_EXPORT_DIRECTORY, то "бежим" по ним
        for (unsigned int i = 0; i < ExportDirectory->NumberOfFunctions - 1; i++) {

            index = 0xFFFF;
            for (unsigned int j = 0; j < ExportDirectory->NumberOfNames; j++) {

                if (AddressOfNameOrdinals[j] == (i + ExportDirectory->Base)) {
                    index = j;
                    continue;
                }
            }
            if ((AddressOfFunctions[i] >= imageNTHeadersExport->OptionalHeader.DataDirectory[0].VirtualAddress) && (AddressOfFunctions[i] <= imageNTHeadersExport->OptionalHeader.DataDirectory[0].VirtualAddress + imageNTHeadersExport->OptionalHeader.DataDirectory[0].Size)) {

                if (index != 0xFFFF) {
                    printf("%s\n", (long)fileData + RVAtoOffset((long)fileData, AddressOfNames[index]));
                }
                else {
                    printf("OrdinalOnly\n", (long)fileData + RVAtoOffset((long)fileData, AddressOfNames[index]));
                }
            }
            if (index != 0xFFFF) {
                printf("%s\n", (long)fileData + RVAtoOffset((long)fileData, AddressOfNames[index]));
            }
            else {
                printf("%Ordinal: %x\n", i + ExportDirectory->Base, AddressOfFunctions[i]);
            }
        }
    }



    //*******************************************************************************************************************************************************************************************************************************************************


    // проверяем на тип таблицы импорта
    // нужен не delayed import table
    if (!imageNTHeadersImport->OptionalHeader.DataDirectory[1].VirtualAddress) {

        printf("\nDLL IMPORTS:\n");

        printf("\nFile has delay import table\n");

    }
    else if (!imageNTHeadersImport->OptionalHeader.DataDirectory[1].Size) {

        printf("\nDLL IMPORTS:\n");

        printf("\nFile has delay import table\n");

    }
    else {
        // смещаемся к RVA таблицы импорта
        DWORD importDirectoryRVA = imageNTHeadersImport->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;

        // заполняем данные секции
        for (int i = 0; i < imageNTHeadersImport->FileHeader.NumberOfSections; i++) {
            sectionHeaderImport = (PIMAGE_SECTION_HEADER)sectionLocationImport;

            // сохраняем секцию, которая содержит "import directory table"
            if (importDirectoryRVA >= sectionHeaderImport->VirtualAddress && importDirectoryRVA < sectionHeaderImport->VirtualAddress + sectionHeaderImport->Misc.VirtualSize) {
                importSection = sectionHeaderImport;
            }
            sectionLocationImport += sectionSizeImport;
        }

        // устанавливаем "file offset" на таблицу импорта
        rawOffset = (DWORD)fileData + importSection->PointerToRawData;

        // устанавливаем "pointer" на "import descriptor's file offset" используя для вычисления "file offset" формулу: imageBaseAddress + pointerToRawDataOfTheSectionContainingRVAofInterest + (RVAofInterest - SectionContainingRVAofInterest.VirtualAddress)
        importDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)(rawOffset + (imageNTHeadersImport->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress - importSection->VirtualAddress));

        printf("\nDLL IMPORTS:\n");
        for (; importDescriptor->Name != 0; importDescriptor++) {

            // вывод названия dll библиотеки, которая импортирует функции
            NameOfDll = rawOffset + (importDescriptor->Name - importSection->VirtualAddress);
            printf("\n%s:\n", NameOfDll);

            thunk = importDescriptor->OriginalFirstThunk == 0 ? importDescriptor->FirstThunk : importDescriptor->OriginalFirstThunk;
            thunkData = (PIMAGE_THUNK_DATA)(rawOffset + (thunk - importSection->VirtualAddress));

            // вывод названия импортируемых функций или, если их нельзя считать, их ординала
            for (; thunkData->u1.AddressOfData != 0; thunkData++) {

                if (thunkData->u1.AddressOfData > 0x80000000) {

                    OrdinalOfImportedFunctions = (WORD)thunkData->u1.AddressOfData;
                    printf("Ordinal: %x\n", OrdinalOfImportedFunctions);
                }
                else {

                    NameOfImportedFunctions = (rawOffset + (thunkData->u1.AddressOfData - importSection->VirtualAddress + 2));
                    printf("%s\n", NameOfImportedFunctions);
                }
            }
        }
    }
    return 0;
}