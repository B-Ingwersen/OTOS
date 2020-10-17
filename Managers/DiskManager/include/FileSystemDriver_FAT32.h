
#ifndef FILE_SYSTEM_DRIVE_FAT32_H
#define FILE_SYSTEM_DRIVE_FAT32_H

#include "Definitions.h"

#define FAT32_READ_CLUSTER_CHAIN_ERROR 0
#define FAT32_READ_CLUSTER_CHAIN_SUCCESS 1
#define FAT32_READ_CLUSTER_CHAIN_END_OF_CHAIN 2

#define FAT32_FIND_NAME_IN_DIR_ERROR 0

struct Fat32_BootRecord {
    u8 jmpShortNopBlock[3];
    u8 OEMIdentifier[8];
    u16 bytesPerSector;
    u8 sectorsPerCluster;
    u16 numberReservedSectors;
    u8 numberOfFATs;
    u16 numberOfDirectorEntries;
    u16 totalSectors;
    u8 mediaDescriptorType;
    u16 fat12_16numberOfSectors;
    u16 sectorsPerTrack;
    u16 numberOfHeadsOrSides;
    u32 numberHiddenSectors;
    u32 largeSectorCount;

    u32 sectorsPerFAT;
    u16 flags;
    u16 fatVersionNumber;
    u32 rootDirectoryClusterNumber;
    u16 fsInfoSectorNumber;
    u16 backupBootSectorSectorNumber;
    u8 reserved[12];
    u8 driveNumber;
    u8 windowsNTflagsReserved;
    u8 signature;
    u32 volumeID;
    u8 volumeLabel[11];
} ExactBinaryStructure;

struct Fat32_Directory_Standard83 {
    u8 fileName[11];
    u8 fileAttributes;
    u8 reserved;
    u8 creationTime_TenthsOfSecond;
    u16 creationTime;
    u16 creationDate;
    u16 lastAccessDate;
    u16 clusterNumber_high;
    u16 lastModificationTime;
    u16 lastModificationDate;
    u16 clusterNumber_low;
    u32 fileSize;
} ExactBinaryStructure;

struct Fat32_Directory_LongFileName {
    u8 sequence;
    u8 nameEntry_first5[10];
    u8 attribute;
    u8 longEntryType;
    u8 checksum;
    u8 nameEntry_next6[12];
    u16 zeroBytes;
    u8 nameEntry_final2[4];
} ExactBinaryStructure;

struct Fat32_PartitionInfo {
    struct Fat32_BootRecord bootRecord;
};

struct Fat32_ClusterChainReader {
    struct PartitionDescriptor * partition;
    struct Fat32_PartitionInfo * info;
    u32 currentCluster;
    MemoryPage buffer;
    u32 bufferSize;
    u32 currentReadOffset;
    u32 currentReadSize;
    struct Fat32_Directory_Standard83 * directoryEntry;
};

struct Fat32_LocationData {
    u32 startCluster;
    u32 startClusterOffset;
    
    u32 currentCluster;
    u32 currentClusterOffset;
    u32 currentFileOffset;
};

struct fat32_FileOpenBuffer {
    MemoryPage buffer;
    u32 fileLength;
    u32 bufferNPages;
};

extern struct FileSystemDriver fat32_FileSystemDriver;

u32 fat32_GetNextCluster(struct PartitionDescriptor * partition, u32 clusterNumber, MemoryPage buffer);

u32 fat32_ReadClusterChain(struct Fat32_ClusterChainReader * chainReader);

u32 fat32_process83fileName(u8 * _83fileName, u8 * processedFileName, u32 * length);

u32 fat32_FindNameInDirectory(u8 * name, u32 nameLength, struct Fat32_ClusterChainReader * clusterReader);

u32 fat32_GetFileNameCluster(u8 * fileName, u32 fileNameLength, struct Fat32_ClusterChainReader * clusterReader);

void fat32_OpenFullFileToBuffer(struct PartitionDescriptor * partition, u8 * fileName, u32 fileNameLength, struct fat32_FileOpenBuffer * fileBuffer);

u32 fat32_InitializePartition(struct PartitionDescriptor * partition);

//u32 fat32_GetFileDescriptor(struct PartitionDescriptor * partition, u8 * fileName, u32 fileNameLength, struct FileDescriptor * fileDescriptor, void * saveLocationData) {
u32 fat32_OpenFileHandle(struct VirtualFileSystem_FileHandle * fileHandle);

u32 fat32_ReadFile(struct VirtualFileSystem_FileHandle * fileHandle, u64 filePosition, u64 readSize, u64 * actualReadSize);

u32 fat32_readDirectoryEntries(struct VirtualFileSystem_FileHandle * fileHandle, u32 nEntries, u32 * entriesActuallyRead);

u32 fat32_InitializeFileSystemDriver();

#endif