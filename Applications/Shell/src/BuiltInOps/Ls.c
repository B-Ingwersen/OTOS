
#include "BuiltInOps/Ls.h"
#include "PrintFunctions.h"
#include "Interaction.h"
#include "OTOSCore/Disk.h"
#include "OTOSCore/CStandardLibrary/string.h"
#include "OTOSCore/CStandardLibrary/stdlib.h"

void ls_command(u8 * input) {
    struct Disk_FileHandle fileHandle;
    u32 result = disk_OpenFile(currentDir, strlen(currentDir), 4, &fileHandle);
    if (result == GENERIC_ERROR_RESULT) {
        print_string("Error: could not open current working directory\n");
        return;
    }

    struct Disk_FileDescriptor * fileDescriptors = malloc(16 * sizeof(struct Disk_FileDescriptor));

    print_string("    Type:\tSize:\tName:\n");
    while (true) {
        u32 entriesRead;
        result = disk_ReadDirectoryEntries(&fileHandle, fileDescriptors, 16, &entriesRead);
        if (result == DISK_READ_FILE_ERROR) {
            print_string("Error: disk manager error when reading directory\n");
            free(fileDescriptors);
            disk_CloseFile(&fileHandle);
            return;
        }
        
        u32 i; for (i = 0; i < entriesRead; ++i) {
            if (fileDescriptors[i].attributes & DISK_FILE_DESCRIPTOR_ATTRIBUTE_DIRECTORY) {
                print_string("    Directory\t\t");
            }
            else {
                print_string("    File \t");
                print_int(fileDescriptors[i].length), print_char('\t');
            }

            print_chars(fileDescriptors[i].fileName, fileDescriptors[i].fileNameLength), print_char('\n');
        }

        if (result == DISK_READ_FILE_END_OF_FILE) {
            break;
        }
    }

    free(fileDescriptors);
    disk_CloseFile(&fileHandle);
}