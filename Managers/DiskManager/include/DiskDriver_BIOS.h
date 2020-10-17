
#ifndef DISK_DRIVER_BIOS_H
#define DISK_DRIVER_BIOS_H

#include "Definitions.h"

#define BIOS_DISK_SECTOR_SIZE 512

struct BiosDisk_DriveGeometry {
    i32 numberOfHeads;
    i32 sectorsPerTrack;
};

struct BiosDisk_DiskDriverInformation {
    u32 diskNumber;
    struct BiosDisk_DriveGeometry driveGeometry;
};

extern struct DiskDriver biosDisk_DiskDriver;

u32 biosDisk_GetDriveGeometry(struct BiosDisk_DriveGeometry * geometryPointer);

u32 biosDisk_ReadDisk(struct DiskDescriptor * disk, u64 diskBaseAddress, MemoryPage loadToLocation, u32 nPages);

u32 biosDisk_initializeDisk(struct DiskDescriptor * disk, u32 diskID);

u32 biosDisk_initializeDriver();

#endif