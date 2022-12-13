# Программа для автоматического определения внешних зависимостей исполняемых файлов
## Для чего этот проект:
Этот проект создан как курсовая работа по предмету ТиМП кафедры ИУ8
Он нужен для чтения функций из PE-файла для дальнейшего предоставления их в удобочитаемом формате для дальнейшей возможности взаимодействия с этими функциями
Также же эта программа частично заменяет ПО DependencyWalker

## Что проект делает:
Эта программа сохраняет в себе все импортируемые и экспортируемые функции и выводит их в txt файл
Вторая программа проверяет наличие взаимосвязи импортируемых функций с экспортируемыми

## Что использовалось:
В работе над этим проектом использовались 2 языка программирования: C++ и Python версии 3.11.0

## Краткое описание алгоритма работы программы (Base.cpp):
Программа использует встроенные компанией Microsoft структуры, которые также используются и при создании PE-файлов
Программа постепенно обрабатывает нужные ей сегменты PE-файла и получает из них информацию

### Перечень структур, которые используются в этом проекте:
IMAGE_NT_HEADER
```
typedef struct _IMAGE_NT_HEADERS {
  DWORD                 Signature;
  IMAGE_FILE_HEADER     FileHeader;
  IMAGE_OPTIONAL_HEADER OptionalHeader;
} IMAGE_NT_HEADERS, *PIMAGE_NT_HEADERS;
```
IMAGE_FILE_HEADER
```
typedef struct _IMAGE_FILE_HEADER {
  WORD  Machine;
  WORD  NumberOfSections;
  DWORD TimeDateStamp;
  DWORD PointerToSymbolTable;
  DWORD NumberOfSymbols;
  WORD  SizeOfOptionalHeader;
  WORD  Characteristics;
} IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;
```
IMAGE_OPTIONAL_HEADER
```
typedef struct _IMAGE_OPTIONAL_HEADER {
  WORD                 Magic;
  BYTE                 MajorLinkerVersion;
  BYTE                 MinorLinkerVersion;
  DWORD                SizeOfCode;
  DWORD                SizeOfInitializedData;
  DWORD                SizeOfUninitializedData;
  DWORD                AddressOfEntryPoint;
  DWORD                BaseOfCode;
  DWORD                BaseOfData;
  DWORD                ImageBase;
  DWORD                SectionAlignment;
  DWORD                FileAlignment;
  WORD                 MajorOperatingSystemVersion;
  WORD                 MinorOperatingSystemVersion;
  WORD                 MajorImageVersion;
  WORD                 MinorImageVersion;
  WORD                 MajorSubsystemVersion;
  WORD                 MinorSubsystemVersion;
  DWORD                Win32VersionValue;
  DWORD                SizeOfImage;
  DWORD                SizeOfHeaders;
  DWORD                CheckSum;
  WORD                 Subsystem;
  WORD                 DllCharacteristics;
  DWORD                SizeOfStackReserve;
  DWORD                SizeOfStackCommit;
  DWORD                SizeOfHeapReserve;
  DWORD                SizeOfHeapCommit;
  DWORD                LoaderFlags;
  DWORD                NumberOfRvaAndSizes;
  IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} IMAGE_OPTIONAL_HEADE
R, *PIMAGE_OPTIONAL_HEADER;
```
IMAGE_DATA_DIRECTORY
```
typedef struct _IMAGE_DATA_DIRECTORY {
  DWORD VirtualAddress;
  DWORD Size;
} IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;
```
IMAGE_SECTION_HEADER
```
typedef struct _IMAGE_SECTION_HEADER {
  BYTE  Name[IMAGE_SIZEOF_SHORT_NAME];
  union {
    DWORD PhysicalAddress;
    DWORD VirtualSize;
  } Misc;
  DWORD VirtualAddress;
  DWORD SizeOfRawData;
  DWORD PointerToRawData;
  DWORD PointerToRelocations;
  DWORD PointerToLinenumbers;
  WORD  NumberOfRelocations;
  WORD  NumberOfLinenumbers;
  DWORD Characteristics;
} IMAGE_SECTION_HEADER, *PIMAGE_SECTION_HEADER;
```
IMAGE_EXPORT_DIRECTORY
```
typedef struct _IMAGE_EXPORT_DIRECTORY {
                		DWORD   Characteristics;
                		DWORD   TimeDateStamp;
                		WORD    MajorVersion;
                		WORD    MinorVersion;
                		DWORD   Name;
                		DWORD   Base;
                		DWORD   NumberOfFunctions;
                		DWORD   NumberOfNames;
			DWORD   AddressOfFunctions;
			DWORD   AddressOfNames;
			DWORD   AddressOfNameOrdinals;
	} IMAGE_EXPORT_DIRECTORY,*PIMAGE_EXPORT_DIRECTORY;
```
IMAGE_IMPORT_DESCRIPTOR
```
typedef struct _IMAGE_IMPORT_DESCRIPTOR {
               	union {
                             	DWORD   Characteristics;
			     	DWORD   OriginalFirstThunk; 
			} DUMMYUNIONNAME;
			DWORD   TimeDateStamp;
			DWORD   ForwarderChain;
			DWORD   Name;
			DWORD   FirstThunk;
		} IMAGE_IMPORT_DESCRIPTOR,*PIMAGE_IMPORT_DESCRIPTOR;
```
IMAGE_THUNK_DATA32
```
typedef struct _IMAGE_THUNK_DATA32 {
                		union {
                	         DWORD ForwarderString;
                	         DWORD Function;
                	         DWORD Ordinal;
                	         DWORD AddressOfData;
                	     } u1;
                	 } IMAGE_THUNK_DATA32,*PIMAGE_THUNK_DATA32;
```

После того, как программа получила все нужные ей данные из PE-файла, она созраняет их в txt файл, который будет обработан уже другой программой

## Краткое описание алгоритма работы программы (main.py):
Эта программа создана для того, чтобы показать алгоритм того, как можно обрабатывать далее данные, полученные в результате работы Base.cpp
В ней полученный ранее txt файл разбивается на ещё несколько txt файлов: в первый выводятся все функции, экспортируемые PE-файлом, во второй выводятся все функции, импортируемые разными динамическими библиотеками в PE-файле
В третьй же файл выводятся все функции импорта, которые ранее не экспортировались в этот PE-файл (остаётся только внешний импорт)
