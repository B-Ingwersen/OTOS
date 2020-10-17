
#include "Initialization.h"
#include "Debugging.h"
#include "VirtualFileSystem.h"
#include "Messaging.h"

#include "OTOSCore/Definitions.h"
#include "OTOSCore/MemoryAllocation.h"

#include "FileSystemDriver_FAT32.h"
#include "DiskDriver_BIOS.h"

struct DiskDescriptor * initializeMBRDisk(struct DiskDriver * driver, u32 diskID) {
    MemoryPage buffer = getMappedPage(true);

    struct DiskDescriptor * disk = (struct DiskDescriptor *)buffer;
    disk -> driver = driver;
    void * diskInfo = (void *) ( (IntegerPointer)disk + sizeof(struct DiskDescriptor) );
    disk -> diskDriverInformation = diskInfo;

    u32 diskInfoSize = driver -> initializeDisk(disk, diskID);
    if (diskInfoSize == GENERIC_ERROR_RESULT) {
        debugging_Panic("Initialize Root Disk Error");
        returnMappedPage(buffer);
        return NULL;
    }

    struct PartitionDescriptor * partitions = (struct PartitionDescriptor *)( (IntegerPointer)diskInfo + diskInfoSize );

    MemoryPage * mbrBuffer = getMappedPage(true);
    u32 result = driver -> readDisk(disk, 0x0000000000000000, mbrBuffer, 1);
    if (result == GENERIC_ERROR_RESULT) {
        debugging_Panic("Root Disk Read Error");
        return NULL;
    }

    u32 * partitionEntries = (u32*)((u8*)mbrBuffer + 0x01BE);
    u32 nPartitions = 0;
    u32 i;
    for (i = 0; i < 4 ; i++) {
        u32 baseAddress = partitionEntries[4 * i + 2];
        u32 length = partitionEntries[4 * i + 3];
        if (baseAddress == 0x00000000) {
            continue;
        }

        partitions[nPartitions].baseAddress = (u64)baseAddress << 9;
        partitions[nPartitions].length = (u64)length << 9;

        u8 fsID = (u8)(partitionEntries[4 * i + 1] & 0xFF);
        if (fsID == 0xB || fsID == 0xC) {
            partitions[nPartitions].disk = disk;
            partitions[nPartitions].fsDriver = &fat32_FileSystemDriver;
            u32 result = partitions[nPartitions].fsDriver -> initializePartition(&partitions[nPartitions]);
        }
        else {
            continue;
        }

        nPartitions++;
    }

    disk -> partitions = partitions;
    disk -> nPartitions = nPartitions;

    returnMappedPage(mbrBuffer);
    return disk;
}

u32 initializeDiskManager() {
    u32 result;

    //initialize disk drivers
        result = biosDisk_initializeDriver();
        if (result == GENERIC_ERROR_RESULT) {
            debugging_Panic("Disk Manager Error: Failed to initialize BIOS Disk Driver");
        }
    
    //initialize file system drivers
        result = fat32_InitializeFileSystemDriver();
        if (result == GENERIC_ERROR_RESULT) {
            debugging_Panic("Disk Manager Error: Failed to initialize FAT32 Driver");
        }
    
    //set up volumes:
        struct DiskDescriptor * rootDisk = initializeMBRDisk( &biosDisk_DiskDriver, 0x00);
        if (rootDisk == NULL) {
            debugging_Panic("Disk Manager Error: Failed to initialize root disk");
            return GENERIC_ERROR_RESULT;
        }
    
    //setup virtual file system
        result = virtualFileSystem_Initialize(rootDisk, 0);
        if (result == GENERIC_ERROR_RESULT) {
            return result;
        }

    //set up IPC services
        messaging_InitializeMessaging();

    return GENERIC_SUCCESS_RESULT;
}