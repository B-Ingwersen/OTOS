
#ifndef LIBRARY_MANAGEMENT_H
#define LIBRARY_MANAGEMENT_H

#include "OTOSCore/Definitions.h"
#include "OTOSCore/Synchronization.h"
#include "OTOSCore/SharedMemory.h"

#define MAX_LIBRARY_DESCRIPTORS 1024

struct LibraryDescriptor {
    u8 searchableName[256];
    u8 fullName[1024];
    u32 searchableName_Length;
    u32 fullName_Length;
    MemoryPage * buffer;
    u32 bufferNPages;
    SharedMemory_SegmentID segment;
};

extern struct LibraryDescriptor ** libraryDescriptors;
extern Synchronization_Spinlock libraryDescriptors_Spinlock;

u32 setupLibraryManagement();

struct LibraryDescriptor * addNewLibrary(char * searchableName, u32 searchableName_Length, char * fullName, u32 fullNameLength);

SharedMemory_SegmentID getLibraryFromSearchableName(char * name, u32 nameLength);

#endif