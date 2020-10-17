
#include "BuiltInOps/ViewFile.h"
#include "PrintFunctions.h"
#include "Interaction.h"
#include "OTOSCore/Disk.h"
#include "OTOSCore/MemoryAllocation.h"
#include "OTOSCore/CStandardLibrary/string.h"
#include "OTOSCore/CStandardLibrary/stdlib.h"

void viewFile_command(u8 * input) {
    u8 * fileName = parseNextArgument(input);

    if (fileName == NULL) {
        print_string("Usage: viewFile [fileName]");
        return;
    }

    u32 fileNameSize = parseArgumentEnd(fileName) - fileName;
    u8 * newFileName = malloc(2048);
    u32 newFileSize = 2048;
    if (!processPath(fileName, fileNameSize, newFileName, &newFileSize)) {
        free(newFileName);
        print_string("Error: malformed file name\n");
        return;
    }

    u64 fileSize;
    u32 nBufferPages;
    MemoryPage * fileContents = disk_ReadFullFileToBuffer(newFileName, newFileSize, 1024 * PAGE_SIZE, &fileSize, &nBufferPages);
    if (fileContents == NULL) {
        free(newFileName);
        print_string("Error: could not open file\n");
        return;
    }

    print_chars((u8*)fileContents, (u32)fileSize);

    free(newFileName);
    returnMappedPages(fileContents, nBufferPages);
}

