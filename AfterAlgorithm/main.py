ReadingFileData = open("OutputOfParsing.txt", "r")
OutputImport = open("ImportedFunctions.txt", "w")
OutputExport = open("ExportedFunctions.txt", "w")
ReadingImportFile = open("ImportedFunctions.txt", "r")
ReadingExportFile = open("ExportedFunctions.txt", "r")
OutputAfterAlgorithm = open("NoRepeatingFunctions.txt", "w")

Reader = ReadingFileData.readlines()
ImportIndex = Reader.index("DLL IMPORTS:\n")
for elem in Reader:
    if elem.find("DLL IMPORTS:") != -1:
        break
    if elem.find("EXPORTS OF FILE:") == -1 and elem != '\n':
        OutputExport.write(elem)

for elem in Reader[ImportIndex:]:
    if elem.find("DLL IMPORTS") == -1:
        if elem.find(".dll:") == -1 and elem != '\n':
            OutputImport.write(elem)

ReadingFileData.close()
OutputImport.close()
OutputExport.close()

ReaderImport = ReadingImportFile.readlines()
ReaderImport = set(ReaderImport)

ReaderExport = ReadingExportFile.readlines()
ReaderExport = set(ReaderExport)

for ElementOfImport in set(ReaderImport):
    for ElementOfExport in ReaderExport:
        if ElementOfExport == ElementOfImport:
            ReaderImport.remove(ElementOfImport)
            break

for elem in ReaderImport:
    OutputAfterAlgorithm.write(elem)

ReadingImportFile.close()
ReadingExportFile.close()
OutputAfterAlgorithm.close()

print("\nCheck your files!")