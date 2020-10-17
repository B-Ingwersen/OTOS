
#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include "OTOSCore/Definitions.h"

#define MAXIMUM_PATH_LENGTH 2000;

#define FILE_DESCRIPTOR_ATTRIBUTE_FILE 0x00000001
#define FILE_DESCRIPTOR_ATTRIBUTE_DIRECTORY 0x00000002

#define FILE_SYSTEM_DRIVER_ERROR 0
#define FILE_SYSTEM_DRIVER_SUCCESS 1
#define FILE_SYSTEM_DRIVER_END_OF_FILE 2

struct DiskDescriptor;
struct PartitionDescriptor;
struct DiskDriver;
struct FileSystemDriver;
struct VirtualFileSystem_FileHandle;

struct FileDescriptor {
    u8 fileName[256];
    u32 fileNameLength;
    u32 attributes;
    u64 length;
    u32 creationTime[4];
    u32 modificationTime[4];
    u32 lastAccessTime[4];

    u8 __paddingTo512Bytes[228];
} ExactBinaryStructure;

struct DiskDescriptor {
    u8 name[32];
    u32 nPartitions;
    struct PartitionDescriptor * partitions;
    struct DiskDriver * driver;

    void * diskDriverInformation;
};

struct PartitionDescriptor {
    u8 name[32];
    u64 baseAddress;
    u64 length;
    struct DiskDescriptor * disk;
    struct FileSystemDriver * fsDriver;

    void * fileSystemDriverInformation;
};

struct DiskDriver {
    bool8 initialized;
    u8 name[32];

    u32 (*readDisk)(struct DiskDescriptor * disk, u64 diskBaseAddress, MemoryPage loadToLocation, u32 nPages);
    u32 (*initializeDisk)(struct DiskDescriptor * disk, u32 diskID);
};

struct FileSystemDriver {
    bool8 initialized;
    u8 name[32];

    //u32 (*getFileDescriptor)(struct PartitionDescriptor * partition, u8 * fileName, u32 fileNameLength, struct FileDescriptor * fileDescriptor, void * saveLocationData);
    u32 (*openFileHandle)(struct VirtualFileSystem_FileHandle * fileHandle);
    u32 (*readFile)(struct VirtualFileSystem_FileHandle * fileHandle, u64 filePosition, u64 readSize, u64 * actualReadSize);
    //u32 (*loadFile)(u8 * fileName, u32 fileNameLength, MemoryPage basePage, u32 maxLoadPages);
    //u32 (*loadDirectoryInformation)(u8 * fileName, u32 fileNameLength, MemoryPage basePage, u32 maxLoadPages);
    u32 (*readDirectoryEntries)(struct VirtualFileSystem_FileHandle * fileHandle, u32 nEntries, u32 * entriesActuallYread);
    u32 (*initializePartition)(struct PartitionDescriptor * partition);
};

#endif
