
#ifndef LIBRARY_MANAGER___MESSAGING_DEFINITIONS_H
#define LIBRARY_MANAGER___MESSAGING_DEFINITIONS_H

#include "OTOSCore/Definitions.h"

#define LIBRARY_MANAGER_MESSAGING_OPERATION_TEST 0
#define LIBRARY_MANAGER_MESSAGING_OPERATION_GET_LIBRARY 1

struct LibraryManager_Messaging_Test {
    u32 operation;
    u32 result;
} ExactBinaryStructure;

struct LibraryManager_Messaging_GetLibrary {
    u32 operation;
    u32 result;
    u32 segment;
    u32 bufferNPages;
    u32 libraryNameLength;
    u8 libraryName[256];
} ExactBinaryStructure;

#endif