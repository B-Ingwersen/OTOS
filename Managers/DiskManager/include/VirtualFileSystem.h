
#ifndef VIRTUAL_FILE_SYSTEM_H
#define VIRTUAL_FILE_SYSTEM_H

#include "Definitions.h"
#include "OTOSCore/Definitions.h"
#include "OTOSCore/Synchronization.h"
#include "OTOSCore/SharedMemory.h"

/*
Virtual File System Format:
    A full filename is of format:
        //d:[Device Name]//p:[Partition Name]/path/to/file.extension
    
    A filename starting with just "/" assumes the root device & partition:
        /path/to/file.extension goes to
        //d:RootDevice//p:RootPartition/path/to/file.extension
    
    A fileName that starts with the partition identifier assumes the root device:
        //p:DataPartition/path/to/file.extension    goes to
        //d:RootDevice//p:DataPartition/path/to/file.extension

*/
#define VIRTUAL_FILE_SYSTEM_ERROR_HANDLE 0xFFFF
#define VIRTUAL_FILE_SYSTEM_MAX_DEVICES (PAGE_SIZE / sizeof(struct DiskDescriptor *))
#define VIRTUAL_FILE_SYSTEM_MAX_FILE_HANDLES (PAGE_SIZE / sizeof(struct VirtualFileSystem_FileHandle *))

struct VirtualFileSystem_FileHandle {
    Synchronization_Spinlock lock;
    struct PartitionDescriptor * partition;
    
    u8 fullFilePath[2000];
    u32 fullFilePath_Length;
    struct FileDescriptor fileDescriptor;

    SharedMemory_SegmentID sharedMemorySegment;
    MemoryPage buffer;
    u32 bufferNPages;
    PID handlingProcess;

    u8 fsDriverLocationData[256];
};

extern struct DiskDescriptor * virtualFileSystem_RootDevice;
extern struct PartitionDescriptor * virtualFileSystem_RootPartition;

extern u32 virtualFileSystem_NDevices;
extern struct DiskDescriptor ** virtualFileSystem_DeviceList;

extern struct VirtualFileSystem_FileHandle ** virtualFileSystem_FileHandles;
extern Synchronization_Spinlock virtualFileSystem_FileHandles_Spinlock;

struct DiskDescriptor * virtualFileSystem_GetDevice(u8 * name, u32 nameLength);

struct PartitionDescriptor * virtualFileSystem_GetPartition(struct DiskDescriptor * disk, u8 * name, u32 nameLength);

u32 virtualFileSystem_ResolveFileName(u8 * fileName, u32 fileNameLength, struct PartitionDescriptor ** partitionReturnAddress, u8 ** fileNameReturnAddress, u32 * fileNameLengthReturnAddress);

/* Open a new file handle, returning the file handle id or the error file handle
    fileName, fileNameLength: the pointer and length of a string containing the
    absolute file path to be opened

    bufferNPages: the number of pages to use for the shared memory buffer for
    reading this file handle

    pid: the pid of the process requesting the file

    sharedMemorySegment: a pointer for returning the shared memory segment of
    the transfer buffer

    attributes: a pointer for returning the file attributes

    length: a pointer for returning the file's length in bytes
*/
u32 virtualFileSystem_OpenFileHandle(u8 * fileName, u32 fileNameLength, u32 bufferNPages, PID pid, SharedMemory_SegmentID * sharedMemorySegment, u32 * attributes, u64 * length);

u32 virtualFileSystem_ReadFileHandle(u32 handleNumber, PID pid, u64 filePosition, u64 readSize, u64 * actualReadSize);

u32 virtualFileSystem_ReadDirectoryEntries(u32 handleNumber, PID pid, u32 nEntries, u32 * entriesActuallyRead);

u32 virtualFileSystem_CloseFileHandle(u32 handleNumber, PID pid);

u32 virtualFileSystem_Initialize(struct DiskDescriptor * rootDisk, u32 rootPartitionNumber);

#endif
