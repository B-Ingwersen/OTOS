
#include "LibraryManagement.h"
#include "OTOSCore/Synchronization.h"
#include "OTOSCore/MemoryAllocation.h"
#include "OTOSCore/Disk.h"
#include "OTOSCore/CStandardLibrary/string.h"

struct LibraryDescriptor ** libraryDescriptors = NULL;
Synchronization_Spinlock libraryDescriptors_Spinlock = 0;

u32 setupLibraryManagement() {
    libraryDescriptors = getMappedPage(true);

    u32 i;
    for (i = 0; i < MAX_LIBRARY_DESCRIPTORS; i++) {
        libraryDescriptors[i] = NULL;
    }

    synchronization_InitializeSpinlock(&libraryDescriptors_Spinlock);
}

struct LibraryDescriptor * addNewLibrary(char * searchableName, u32 searchableName_Length, char * fullName, u32 fullNameLength) {
    if (searchableName_Length > 256 || fullNameLength > 1024) {
        return NULL;
    }

    u32 i;
    for (i = 0; i < MAX_LIBRARY_DESCRIPTORS; i++) {
        if (libraryDescriptors[i] == NULL) {
            break;
        }
    }
    if (i >= MAX_LIBRARY_DESCRIPTORS) {
        return NULL;
    }

    u32 descriptorNumber = i;
    u64 fileSize; u32 nBufferPages;
    MemoryPage * buffer = disk_ReadFullFileToBuffer(fullName, fullNameLength, 256 * 1024 * 1024, &fileSize, &nBufferPages);
    if (buffer == NULL) {
        return NULL;
    }

    SharedMemory_SegmentID segment = sharedMemory_CreateSegment(buffer, nBufferPages);
    if (segment == SHARED_MEMORY_ERROR_SEGMENT) {
        returnMappedPages(buffer, nBufferPages);
        return NULL;
    }

    // fill in descriptor
        struct LibraryDescriptor * descriptor = (struct LibraryDescriptor *)getMappedPage(true);
        strncpy(descriptor -> searchableName, searchableName, searchableName_Length);
        descriptor -> searchableName_Length = searchableName_Length;
        strncpy(descriptor -> fullName, fullName, fullNameLength);
        descriptor -> fullName_Length = fullNameLength;

        descriptor -> buffer = buffer;
        descriptor -> bufferNPages = nBufferPages;
        descriptor -> segment = segment;
    
    libraryDescriptors[descriptorNumber] = descriptor;
    return descriptor;
}

SharedMemory_SegmentID getLibraryFromSearchableName(char * name, u32 nameLength) {
    if (nameLength > 256) {
        return SHARED_MEMORY_ERROR_SEGMENT;
    }
    u32 i;

    synchronization_OpenSpinlock(&libraryDescriptors_Spinlock);
    SharedMemory_SegmentID segment;

    for (i = 0; i < MAX_LIBRARY_DESCRIPTORS; i++) {
        if (libraryDescriptors[i] != NULL) {
            if (libraryDescriptors[i] -> searchableName_Length == nameLength && strncmp(libraryDescriptors[i] -> searchableName, name, nameLength) == 0) {
                segment = libraryDescriptors[i] -> segment;
                synchronization_CloseSpinlock(&libraryDescriptors_Spinlock);
                return segment;
            }
        }
    }

    u32 fullNameLength = nameLength + 11;
    u8 * fullName = (u8*)getMappedPage(true);
    strcpy(fullName, "/Libraries/");
    strncpy(fullName + 11, name, nameLength);
    struct LibraryDescriptor * newLibrary = addNewLibrary(name, nameLength, fullName, fullNameLength);
    returnMappedPage(fullName);
    if (newLibrary == NULL) {
        synchronization_CloseSpinlock(&libraryDescriptors_Spinlock);
        return SHARED_MEMORY_ERROR_SEGMENT;
    }

    segment = newLibrary -> segment;
    synchronization_CloseSpinlock(&libraryDescriptors_Spinlock);
    return segment;
}