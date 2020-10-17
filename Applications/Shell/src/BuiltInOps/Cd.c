
#include "BuiltInOps/Cd.h"
#include "PrintFunctions.h"
#include "Interaction.h"
#include "OTOSCore/Disk.h"
#include "OTOSCore/CStandardLibrary/string.h"
#include "OTOSCore/CStandardLibrary/stdlib.h"

void cd_command(u8 * input) {
    u8 * dirName = parseNextArgument(input);

    if (dirName == NULL) {
        strcpy(currentDir, "/");
        return;
    }

    u32 dirNameSize = parseArgumentEnd(dirName) - dirName;
    u8 * newDirName = malloc(2048);
    u32 newDirSize = 2048;
    if (!processPath(dirName, dirNameSize, newDirName, &newDirSize)) {
        free(newDirName);
        print_string("Error: malformed path\n");
        return;
    }

    struct Disk_FileHandle fileHandle;
    u32 result = disk_OpenFile(newDirName, newDirSize, 1, &fileHandle);
    if (result == GENERIC_ERROR_RESULT) {
        free(newDirName);
        print_string("Error: path not found\n");
        return;
    }
    else if (!(fileHandle.attributes & DISK_FILE_DESCRIPTOR_ATTRIBUTE_DIRECTORY)) {
        free(newDirName);
        disk_CloseFile(&fileHandle);
        print_string("Error: path is not a directory\n");
        return;
    }

    strcpy(currentDir, newDirName);

    free(newDirName);
    disk_CloseFile(&fileHandle);
}