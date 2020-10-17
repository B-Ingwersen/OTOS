
#include "DiskDriver_BIOS.h"
#include "OTOSCore/SystemCalls.h"

struct DiskDriver biosDisk_DiskDriver = {
    0,
    "BIOS INT 13H",
    NULL,
    NULL
};

u32 biosDisk_GetDriveGeometry(struct BiosDisk_DriveGeometry * geometryPointer) {
    struct SystemCall_BiosCall_Variables vars;
    vars.eax = (8 << 8);
    vars.edx = 0x80;
    vars.interruptNumber = 0x13;

    SystemCallResult result = systemCall_BiosCall(&vars, SYSTEM_CALL_BIOS_CALL_RW_NONE, NULL, 0, NULL, 0);
    if (result == GENERIC_ERROR_RESULT) {
        return result;
    }

    geometryPointer -> numberOfHeads = ((vars.edx >> 8) & 0xFF) + 1;
    geometryPointer -> sectorsPerTrack = vars.ecx & 0x3F;
    return GENERIC_SUCCESS_RESULT;
}

u32 biosDisk_ReadDisk(struct DiskDescriptor * disk, u64 diskBaseAddress, MemoryPage loadToLocation, u32 nPages) {
    struct BiosDisk_DiskDriverInformation * driveInformation = (struct BiosDisk_DiskDriverInformation *)(disk -> diskDriverInformation);
    struct BiosDisk_DriveGeometry * driveGeometry = &(driveInformation -> driveGeometry);
    struct SystemCall_BiosCall_Variables biosVars;

    u32 sectorLBA = (u32)(diskBaseAddress >> 9);
    u32 sector = (sectorLBA % (driveGeometry -> sectorsPerTrack)) + 1;
    u32 temp = sectorLBA / (driveGeometry -> sectorsPerTrack);
    u32 head = temp % (driveGeometry -> numberOfHeads);
    u32 cylinder = temp / (driveGeometry -> numberOfHeads);

    u32 totalSectorsToRead = nPages * 8;
    u32 sectorsToRead = (driveGeometry -> sectorsPerTrack) - sector + 1;

    u32 continuing = true;
    while (continuing) {
        if (sectorsToRead >= totalSectorsToRead) {
            sectorsToRead = totalSectorsToRead;
            continuing = false;
        }

        biosVars.eax = sectorsToRead | (0x2 << 8);
        biosVars.ebx = 0;
        biosVars.ecx = ((cylinder & 0xFF) << 8) | ((cylinder >> 2) & 0xC0) | sector;
        biosVars.edx = (head << 8) | 0x80;
        biosVars.interruptNumber = 0x13;
        
        u32 result = systemCall_BiosCall(&biosVars, SYSTEM_CALL_BIOS_CALL_READ, (void *)loadToLocation, sectorsToRead * BIOS_DISK_SECTOR_SIZE, NULL, 0);
        if (result == GENERIC_ERROR_RESULT) {
            return result;
        }

        loadToLocation = (u8*)loadToLocation + sectorsToRead * BIOS_DISK_SECTOR_SIZE;

        totalSectorsToRead -= sectorsToRead;
        sectorsToRead = driveGeometry -> sectorsPerTrack;

        sector = 1;
        head++;
        if (head == driveGeometry -> numberOfHeads) {
            head = 0;
            cylinder++;
        }
    }
    return GENERIC_SUCCESS_RESULT;
}

u32 biosDisk_initializeDisk(struct DiskDescriptor * disk, u32 diskID) {
    if (diskID >= 2) {
        return GENERIC_ERROR_RESULT;
    }

    struct BiosDisk_DiskDriverInformation * diskInfo = (struct BiosDisk_DiskDriverInformation *)(disk -> diskDriverInformation);
    diskInfo -> diskNumber = diskID + 0x80;

    u32 result = biosDisk_GetDriveGeometry(&(diskInfo -> driveGeometry));
    if (result == GENERIC_ERROR_RESULT) {
        return result;
    }

    return sizeof(struct BiosDisk_DiskDriverInformation);
}

u32 biosDisk_initializeDriver() {
    biosDisk_DiskDriver.readDisk = biosDisk_ReadDisk;
    biosDisk_DiskDriver.initializeDisk = biosDisk_initializeDisk;
    biosDisk_DiskDriver.initialized = 1;
    return GENERIC_SUCCESS_RESULT;
}