#ifndef DISK_MANAGER___MESSAGING_DEFINITIONS_H
#define DISK_MANAGER___MESSAGING_DEFINITIONS_H

#include "OTOSCore/Definitions.h"
#include "OTOSCore/SharedMemory.h"

#define DISK_MANAGER_MESSAGING_OPERATION_TEST 0
#define DISK_MANAGER_MESSAGING_OPERATION_OPEN_FILE_HANDLE 1
#define DISK_MANAGER_MESSAGING_OPERATION_READ_FILE_HANDLE 2
#define DISK_MANAGER_MESSAGING_OPERATION_CLOSE_FILE_HANDLE 3
#define DISK_MANAGER_MESSAGING_OPERATION_READ_DIRECTORY_ENTRIES 4

#define DISK_MANAGER_MESSAGING_READ_FILE_HANDLE_ERROR 0
#define DISK_MANAGER_MESSAGING_READ_FILE_HANDLE_SUCCESS 1
#define DISK_MANAGER_MESSAGING_READ_FILE_HANDLE_END_OF_FILE 2

struct DiskManager_Messaging_Test {
    u32 operation;
    u32 result;
} ExactBinaryStructure;

struct DiskManager_Messaging_OpenFile {
    u32 operation;
    u32 result;
    u32 fileNameLength;
    u32 bufferNPages;
    u32 fileHandleID;
    SharedMemory_SegmentID sharedMemorySegment;
    u32 attributes;
    u64 length;
    u8 fileName[2000];
} ExactBinaryStructure;

struct DiskManager_Messaging_ReadFile {
    u32 operation;
    u32 result;
    u32 fileHandleID;
    u64 filePosition;
    u64 readSize;
    u64 bytesActuallyRead;
} ExactBinaryStructure;

struct DiskManager_Messaging_CloseFile {
    u32 operation;
    u32 result;
    u32 fileHandleID;
} ExactBinaryStructure;

struct DiskManager_Messaging_ReadDirectoryEntries {
    u32 operation;
    u32 result;
    u32 fileHandleID;
    u32 nEntries;
    u32 entriesActuallyRead;
} ExactBinaryStructure;

struct DiskManager_FileDescriptor {
    u8 fileName[256];
    u32 fileNameLength;
    u32 attributes;
    u64 length;
    u32 creationTime[4];
    u32 modificationTime[4];
    u32 lastAccessTime[4];
    u8 __paddingTo512Bytes[228];
} ExactBinaryStructure;

#endif

