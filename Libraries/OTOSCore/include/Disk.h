#ifndef OTOS_CORE___DISK_H
#define OTOS_CORE___DISK_H

#include "Definitions.h"
#include "MemoryAllocation.h"
#include "DiskManager/MessagingDefinitions.h"

#define DISK_MANAGER_PID 0x00010002

#define DISK_READ_FILE_ERROR DISK_MANAGER_MESSAGING_READ_FILE_HANDLE_ERROR
#define DISK_READ_FILE_SUCCESS DISK_MANAGER_MESSAGING_READ_FILE_HANDLE_SUCCESS
#define DISK_READ_FILE_END_OF_FILE DISK_MANAGER_MESSAGING_READ_FILE_HANDLE_END_OF_FILE

#define DISK_FILE_DESCRIPTOR_ATTRIBUTE_FILE 0x00000001
#define DISK_FILE_DESCRIPTOR_ATTRIBUTE_DIRECTORY 0x00000002

typedef u32 Disk_FileHandle;
struct Disk_FileHandle {
    u32 fileHandle;
    u32 sharedMemoryID;
    MemoryPage * buffer;
    u32 bufferNPages;
    u32 attributes;
    u64 length;
};

struct Disk_FileDescriptor {
    u8 fileName[256];
    u32 fileNameLength;
    u32 attributes;
    u64 length;
    u32 creationTime[4];
    u32 modificationTime[4];
    u32 lastAccessTime[4];
    u8 __paddingTo512Bytes[228];
} ExactBinaryStructure;

u32 disk_Test();

u32 disk_OpenFile_Allocator(u8 * fileName, u32 fileNameLength, u32 bufferNPages, struct Disk_FileHandle * fileHandle, struct MemoryAllocation_BufferAllocator * allocator);

u32 disk_ReadFile(struct Disk_FileHandle * fileHandle, u64 filePosition, u64 readSize, u64 * bytesRead, void * buffer);

u32 disk_ReadDirectoryEntries(struct Disk_FileHandle * fileHandle, struct Disk_FileDescriptor * fileDescriptors, u32 nEntries, u32 * entriesRead);

u32 disk_CloseFile_Allocator(struct Disk_FileHandle * fileHandle, struct MemoryAllocation_BufferAllocator * allocator);

MemoryPage disk_ReadFullFileToBuffer_Allocator(u8 * fileName, u32 fileNameLength, u32 maxBufferSize, u64 * fileSize, u32 * bufferNPages, struct MemoryAllocation_BufferAllocator * allocator);


u32 disk_OpenFile(u8 * fileName, u32 fileNameLength, u32 bufferNPages, struct Disk_FileHandle * fileHandle);

u32 disk_CloseFile(struct Disk_FileHandle * fileHandle);

MemoryPage disk_ReadFullFileToBuffer(u8 * fileName, u32 fileNameLength, u32 maxBufferSize, u64 * fileSize, u32 * bufferNPages);

#endif
