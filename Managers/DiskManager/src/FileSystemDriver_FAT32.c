
#include "Definitions.h"
#include "VirtualFileSystem.h"
#include "FileSystemDriver_FAT32.h"
#include "OTOSCore/MemoryAllocation.h"
#include "OTOSCore/SystemCalls.h"
#include "OTOSCore/CStandardLibrary/string.h"

struct FileSystemDriver fat32_FileSystemDriver = {
    0,
    "FAT32",
    NULL,
    NULL,
    NULL,
    NULL
};

u32 fat32_GetNextCluster(struct PartitionDescriptor * partition, u32 clusterNumber, MemoryPage buffer) {
    struct Fat32_PartitionInfo * info = (struct Fat32_PartitionInfo *)(partition -> fileSystemDriverInformation);

    u32 fatClusterOffset = 4 * clusterNumber;
    
    u64 absoluteOffset = (u64)partition -> baseAddress + (u64)( (info -> bootRecord.numberReservedSectors) * (info -> bootRecord.bytesPerSector) ) + (u64)fatClusterOffset;
    u32 entryOffset = ((u32)absoluteOffset & 0xFFF) / 4;
    absoluteOffset &= 0xFFFFFFFFFFFFF000;

    u32 result = partition -> disk -> driver -> readDisk(partition -> disk, absoluteOffset, buffer, 1);
    if (result == GENERIC_ERROR_RESULT) {
        return 0x0FFFFFF7;
    }

    u32 newCluster = ((u32*)buffer)[entryOffset] & 0x0FFFFFFF;
    return newCluster;
}

u32 fat32_ReadClusterChain(struct Fat32_ClusterChainReader * chainReader) {
    struct PartitionDescriptor * partition = chainReader -> partition;
    struct Fat32_BootRecord * bootRecord = &(chainReader -> info -> bootRecord);

    u32 sectorSize = bootRecord -> bytesPerSector;
    u32 clusterSize = sectorSize * bootRecord -> sectorsPerCluster;
    u32 firstDataSector = bootRecord -> numberReservedSectors + (bootRecord -> sectorsPerFAT * bootRecord -> numberOfFATs);

    // if the current cluster has been exhausted, advance to the cluster in the
    // chain
    if (chainReader -> currentReadOffset >= clusterSize) {
        u32 fatClusterOffset = 4 * chainReader -> currentCluster;

        u64 absoluteOffset = (u64)partition -> baseAddress + (u64)(bootRecord -> numberReservedSectors * sectorSize) + (u64)fatClusterOffset;
        u32 entryOffset = ((u32)absoluteOffset & 0xFFF) / 4;
        absoluteOffset &= 0xFFFFFFFFFFFFF000;
        //biosDisk_ReadDisk(absoluteOffset, chainReader -> buffer, 1);
        u32 result = partition -> disk -> driver -> readDisk(partition -> disk, absoluteOffset, chainReader -> buffer, 1);
        if (result == GENERIC_ERROR_RESULT) {
            return FAT32_READ_CLUSTER_CHAIN_ERROR;
        }

        u32 newCluster = ((u32*)chainReader -> buffer)[entryOffset];
        newCluster &= 0x0FFFFFFF;
        if (newCluster >= 0x0FFFFFF8) {
            return FAT32_READ_CLUSTER_CHAIN_END_OF_CHAIN;
        }
        else if (newCluster == 0x0FFFFFF7) {
            return FAT32_READ_CLUSTER_CHAIN_ERROR;
        }

        chainReader -> currentCluster = newCluster;
        chainReader -> currentReadOffset = 0;
        chainReader -> currentReadSize = 0;
    }

    u32 firstSectorOfCluster = (chainReader -> currentCluster - 2) * bootRecord -> sectorsPerCluster + firstDataSector;
    u64 absoluteBaseAddress = (u64)firstSectorOfCluster * (u64)sectorSize + partition -> baseAddress;

    u32 readSize = clusterSize - chainReader -> currentReadOffset;
    if (readSize > chainReader -> bufferSize) {
        readSize = chainReader -> bufferSize;
    }
    u32 readSizePages = (readSize + PAGE_SIZE - 1) / PAGE_SIZE;
    //biosDisk_ReadDisk(absoluteBaseAddress, chainReader -> buffer, readSizePages);
    u32 result = partition -> disk -> driver -> readDisk(partition -> disk, absoluteBaseAddress, chainReader -> buffer, 1);
    if (result == GENERIC_ERROR_RESULT) {
        return FAT32_READ_CLUSTER_CHAIN_ERROR;
    }
    if (chainReader -> currentReadOffset != 0) {
        memmove(chainReader -> buffer, (u8*)(chainReader -> buffer) + chainReader -> currentReadOffset, readSize);
    }
    chainReader -> currentReadOffset += readSize;
    chainReader -> currentReadSize = readSize;

    return FAT32_READ_CLUSTER_CHAIN_SUCCESS;
}

u32 fat32_process83fileName(u8 * _83fileName, u8 * processedFileName, u32 * length) {
    // copy filename stem
    i32 i;
    u32 stemLength = 0;
    for (i = 7; i >= 0; --i) {
        if (stemLength == 0) {
            if (_83fileName[i] != ' ') {
                processedFileName[i] = _83fileName[i];
                stemLength = i + 1;
            }
        }
        else {
            processedFileName[i] = _83fileName[i];
        }
    }

    // copy file extension
    u32 extensionLength = 0;
    for (i = 2; i >= 0; --i) {
        if (extensionLength == 0) {
            if (_83fileName[i + 8] != ' ') {
                processedFileName[stemLength + i + 1] = _83fileName[i + 8];
                stemLength = i + 1;
            }
        }
        else {
            processedFileName[stemLength + i + 1] = _83fileName[i + 8];
        }
    }

    if (extensionLength != 0) {
        processedFileName[stemLength] = '.';
        processedFileName[stemLength + 1 + extensionLength] = 0;
        *length = stemLength + 1 + extensionLength;
    }
    else {
        processedFileName[stemLength] = 0;
        *length = stemLength;
    }
}

u32 fat32_FindNameInDirectory(u8 * name, u32 nameLength, struct Fat32_ClusterChainReader * clusterReader) {
    u32 foundFileName = 0;
    u32 longFileNameMatch = 0;
    u32 foundNameCluster = 0;
    while (true) {
        u32 clusterReadResult = fat32_ReadClusterChain(clusterReader);
        if (clusterReadResult == FAT32_READ_CLUSTER_CHAIN_END_OF_CHAIN || clusterReadResult == FAT32_READ_CLUSTER_CHAIN_ERROR) {
            break;
        }

        u32 checkOffset = 0;
        struct Fat32_Directory_Standard83 * entry = (struct Fat32_Directory_Standard83 *)clusterReader -> buffer;
        while (checkOffset < clusterReader -> currentReadSize) {

            if (entry -> fileName[0] == 0) {
                break;
            }
            else if (entry -> fileAttributes == 0x0F) { //long file name
                struct Fat32_Directory_LongFileName * lfnEntry = (struct Fat32_Directory_LongFileName *)entry;

                u8 lfn[13];
                int i;
                for (i = 0; i < 5; i++) {
                    lfn[i] = lfnEntry -> nameEntry_first5[2 * i];
                }
                for (i = 0; i < 6; i++) {
                    lfn[i + 5] = lfnEntry -> nameEntry_next6[2 * i];
                }
                lfn[11] = lfnEntry -> nameEntry_final2[0];
                lfn[12] = lfnEntry -> nameEntry_final2[2];

                u32 sequenceNumber = lfnEntry -> sequence & 0x1F;
                if (lfnEntry -> sequence & 0x40) {
                    longFileNameMatch = 1;
                }
                
                u32 fileNameCheckOffset = (sequenceNumber - 1) * 13;
                for (i = 0; i < 13; i++) {
                    if (fileNameCheckOffset >= nameLength) {
                        if (lfn[i] != 0) {
                            longFileNameMatch = 0;
                        }
                        break;
                    }
                    else if (lfn[i] != name[fileNameCheckOffset]) {
                        longFileNameMatch = 0;
                        break;
                    }
                    fileNameCheckOffset++;
                }
            }
            else if (entry -> fileName[0] != 0xE5) {
                if (longFileNameMatch) {
                    foundNameCluster = ((u32)entry -> clusterNumber_high << 16) + (u32)entry -> clusterNumber_low;
                    clusterReader -> directoryEntry = entry;
                    break;
                }
                else {
                    u8 fileName[13];
                    u32 fileNameLength;
                    fat32_process83fileName(entry -> fileName, fileName, &fileNameLength);

                    if (fileNameLength == nameLength && strncmp(fileName, name, nameLength) == 0) {
                        foundNameCluster = ((u32)entry -> clusterNumber_high << 16) + (u32)entry -> clusterNumber_low;
                        clusterReader -> directoryEntry = entry;
                        break;
                    }
                    //TODO -- implement check for 8.3 filenames
                }
                longFileNameMatch = 0;
            }

            entry++;
            checkOffset += 32;
        }

        if (foundNameCluster != 0) {
            break;
        }
    }
    
    return foundNameCluster;
}

u32 fat32_GetFileNameCluster(u8 * fileName, u32 fileNameLength, struct Fat32_ClusterChainReader * clusterReader) {

    if (fileNameLength == 1 && *fileName == '/') {
        clusterReader -> directoryEntry = NULL;
        return clusterReader -> currentCluster;
    }
    
    u32 fileNamePosition = 0;
    u32 foundCluster = 0;
    while (true) {
        if (fileNamePosition == fileNameLength) {
            if (fileNamePosition > 0) {
                foundCluster = clusterReader -> currentCluster;
            }
            break;
        }
        if (fileName[fileNamePosition] != '/') {
            break;
        }
        fileNamePosition++;

        u32 i = fileNamePosition;
        while (i < fileNameLength) {
            u8 character = fileName[i];
            if (character == '/' || character == 0) {
                break;
            }
            i++;
        }

        u32 length = i - fileNamePosition;
        u32 clusterNumber = fat32_FindNameInDirectory(fileName + fileNamePosition, length, clusterReader);
        if (clusterNumber == 0) {
            break;
        }
        fileNamePosition += length;
        
        clusterReader -> currentCluster = clusterNumber;
        clusterReader -> currentReadOffset = 0;
        clusterReader -> currentReadSize = 0;
    }
    
    return foundCluster;
}

void fat32_OpenFullFileToBuffer(struct PartitionDescriptor * partition, u8 * fileName, u32 fileNameLength, struct fat32_FileOpenBuffer * fileBuffer) {
    struct Fat32_ClusterChainReader clusterReader;

    // setup partition and boot record
        clusterReader.partition = partition;
        clusterReader.info = (struct Fat32_PartitionInfo *)(partition -> fileSystemDriverInformation);
        struct Fat32_BootRecord * bootRecord = &(clusterReader.info -> bootRecord);
    
    // setup buffer
        MemoryPage buffer = getMappedPage(true);
        clusterReader.buffer = buffer;
        clusterReader.bufferSize = PAGE_SIZE;

    // finish chain reader setup
        clusterReader.currentCluster = bootRecord -> rootDirectoryClusterNumber;
        clusterReader.currentReadOffset = 0;
        clusterReader.currentReadSize = 0;
    
    u32 clusterNumber = fat32_GetFileNameCluster(fileName, fileNameLength, &clusterReader);    

    MemoryPage returnBuffer = NULL;
    if (clusterNumber != 0 && (clusterReader.directoryEntry -> fileAttributes & 0x10) == 0 && clusterReader.directoryEntry -> fileSize > 0) {
        u32 fileSize = clusterReader.directoryEntry -> fileSize;
        u32 bufferNPages = (fileSize + PAGE_SIZE - 1) / PAGE_SIZE;

        fileBuffer -> fileLength = fileSize;
        fileBuffer -> bufferNPages = bufferNPages;

        returnBuffer = getMappedPage(true);

        clusterReader.currentCluster = clusterNumber;
        clusterReader.currentReadOffset = 0;
        clusterReader.currentReadSize = 0;
        u32 returnBufferPointer = 0;
        while (true) {
            u32 clusterReadResult = fat32_ReadClusterChain(&clusterReader);
            if (clusterReadResult == FAT32_READ_CLUSTER_CHAIN_END_OF_CHAIN || clusterReadResult == FAT32_READ_CLUSTER_CHAIN_ERROR) {
                systemCall_Memory_UnmapMemory(0, returnBuffer, bufferNPages, false, SYSTEM_CALL_NO_PERMISSION);
                returnBuffer = NULL;
                break;
            }

            if (returnBufferPointer + clusterReader.currentReadSize >= fileSize) {
                memcpy((u8*)returnBuffer + returnBufferPointer, buffer, fileSize - returnBufferPointer);
                break;
            }

            memcpy((u8*)returnBuffer + returnBufferPointer, buffer, clusterReader.currentReadSize);
            returnBufferPointer += clusterReader.currentReadSize;
        }
    }
    // free buffer
        systemCall_Memory_UnmapMemory(0, buffer, 1, false, SYSTEM_CALL_NO_PERMISSION);

    fileBuffer -> buffer = returnBuffer;
}

u32 fat32_InitializePartition(struct PartitionDescriptor * partition) {
    MemoryPage buffer = getMappedPage(true);
    u32 result = partition -> disk -> driver -> readDisk(partition -> disk, partition -> baseAddress, buffer, 1);
    if (result == GENERIC_ERROR_RESULT) {
        returnMappedPage(buffer);
        return result;
    }

    struct Fat32_PartitionInfo * partitionInfo = (struct Fat32_PartitionInfo *)buffer;
    partition -> fileSystemDriverInformation = partitionInfo;
    
    memmove(&(partitionInfo -> bootRecord), buffer, sizeof(struct Fat32_BootRecord) );
    return GENERIC_SUCCESS_RESULT;
}

//u32 fat32_GetFileDescriptor(struct PartitionDescriptor * partition, u8 * fileName, u32 fileNameLength, struct FileDescriptor * fileDescriptor, void * saveLocationData) {
u32 fat32_OpenFileHandle(struct VirtualFileSystem_FileHandle * fileHandle) {
    struct PartitionDescriptor * partition = fileHandle -> partition;
    u8 * fileName = fileHandle -> fullFilePath;
    u32 fileNameLength = fileHandle -> fullFilePath_Length;
    struct FileDescriptor * fileDescriptor = &(fileHandle -> fileDescriptor);
    void * saveLocationData = &(fileHandle -> fsDriverLocationData);


    struct Fat32_ClusterChainReader clusterReader;

    // setup partition and boot record
        clusterReader.partition = partition;
        clusterReader.info = (struct Fat32_PartitionInfo *)(partition -> fileSystemDriverInformation);
        struct Fat32_BootRecord * bootRecord = &(clusterReader.info -> bootRecord);
    
    // setup buffer
        MemoryPage buffer = getMappedPage(true);
        clusterReader.buffer = buffer;
        clusterReader.bufferSize = PAGE_SIZE;

    // finish chain reader setup
        clusterReader.currentCluster = bootRecord -> rootDirectoryClusterNumber;
        clusterReader.currentReadOffset = 0;
        clusterReader.currentReadSize = 0;
    
    u32 clusterNumber = fat32_GetFileNameCluster(fileName, fileNameLength, &clusterReader);
    u32 result = GENERIC_ERROR_RESULT;
    if (clusterNumber != 0) {
        // copy file name stem
        i32 i = 0;
        while (fileName[fileNameLength - i - 1] != '/') {i++;}
        strncpy(fileDescriptor -> fileName, fileName + fileNameLength - i, i);
        fileDescriptor -> fileNameLength = fileNameLength;

        if (
            clusterReader.directoryEntry == NULL || // fs root
            clusterReader.directoryEntry -> fileAttributes & 0x10
        ) {
            fileDescriptor -> attributes = FILE_DESCRIPTOR_ATTRIBUTE_DIRECTORY;
        }
        else {
            fileDescriptor -> attributes = FILE_DESCRIPTOR_ATTRIBUTE_FILE;
            fileDescriptor -> length = clusterReader.directoryEntry -> fileSize;
        }

        //TODO fill in creation times

        struct Fat32_LocationData * location = (struct Fat32_LocationData *)saveLocationData;
        location -> startCluster = clusterNumber;
        location -> startClusterOffset = 0;

        location -> currentCluster = clusterNumber;
        location -> currentClusterOffset = 0;
        location -> currentFileOffset = 0;

        result = GENERIC_SUCCESS_RESULT;
    }

    // free buffer
        systemCall_Memory_UnmapMemory(0, buffer, 1, false, SYSTEM_CALL_NO_PERMISSION);

    return result;
}

u32 fat32_ReadFile(struct VirtualFileSystem_FileHandle * fileHandle, u64 filePosition, u64 readSize, u64 * actualReadSize) {
    
    struct Fat32_ClusterChainReader clusterReader;

    // setup partition and boot record
        clusterReader.partition = fileHandle -> partition;
        clusterReader.info = (struct Fat32_PartitionInfo *)(fileHandle -> partition -> fileSystemDriverInformation);
        struct Fat32_BootRecord * bootRecord = &(clusterReader.info -> bootRecord);
    
    // setup buffer
        MemoryPage buffer = getMappedPage(true);
        clusterReader.buffer = buffer;
        clusterReader.bufferSize = PAGE_SIZE;

    // setup load locations
        struct Fat32_LocationData * location = (struct Fat32_LocationData *)&(fileHandle -> fsDriverLocationData);

        u32 fileOffset = (u32)filePosition;
        u32 length = (u32)readSize;

        u32 searchCluster = 0;
        u32 searchClusterOffset = 0;
        u32 searchFileOffsetLeft = fileOffset;
        if (fileOffset >= location -> currentFileOffset) {
            searchCluster = location -> currentCluster;
            searchClusterOffset = location -> currentClusterOffset;
            searchFileOffsetLeft = fileOffset - location -> currentFileOffset;
        }

        u32 clusterSize = (clusterReader.info -> bootRecord.sectorsPerCluster) * (clusterReader.info -> bootRecord.bytesPerSector);
        while (true) {
            if (clusterSize - searchClusterOffset > searchFileOffsetLeft) {
                searchClusterOffset += searchFileOffsetLeft;
                break;
            }
            else {
                searchCluster = fat32_GetNextCluster(fileHandle -> partition, searchCluster, buffer);

                if (searchCluster == 0x0FFFFFF7) {
                    systemCall_Memory_UnmapMemory(0, buffer, 1, false, SYSTEM_CALL_NO_PERMISSION);
                    return FILE_SYSTEM_DRIVER_ERROR;
                }
                else if (searchCluster >= 0x0FFFFFF8) {
                    systemCall_Memory_UnmapMemory(0, buffer, 1, false, SYSTEM_CALL_NO_PERMISSION);
                    *actualReadSize = 0;
                    return FILE_SYSTEM_DRIVER_END_OF_FILE;
                }
                searchFileOffsetLeft -= (clusterSize - searchClusterOffset);
                searchClusterOffset = 0;
            }
        }

        clusterReader.currentCluster = searchCluster;
        clusterReader.currentReadOffset = searchClusterOffset;
        clusterReader.currentReadSize = 0;
    // read to buffer
        MemoryPage returnBuffer = fileHandle -> buffer;

        u32 clusterReadResult;
        u32 returnBufferPointer = 0;
        u32 bytesRead = 0;
        while (true) {
            clusterReadResult = fat32_ReadClusterChain(&clusterReader);
            if (clusterReadResult == FAT32_READ_CLUSTER_CHAIN_END_OF_CHAIN || clusterReadResult == FAT32_READ_CLUSTER_CHAIN_ERROR) {
                break;
            }

            if (returnBufferPointer + clusterReader.currentReadSize >= readSize) {
                clusterReadResult = GENERIC_SUCCESS_RESULT;
                memcpy((u8*)returnBuffer + returnBufferPointer, buffer, readSize - returnBufferPointer);
                bytesRead += readSize - returnBufferPointer;
                break;
            }

            memcpy((u8*)returnBuffer + returnBufferPointer, buffer, clusterReader.currentReadSize);
            bytesRead += clusterReader.currentReadSize;
            returnBufferPointer += clusterReader.currentReadSize;
        }

    // clean up
        returnMappedPage(buffer);

        *actualReadSize = bytesRead;
        return clusterReadResult;
}

u32 fat32_readDirectoryEntries(struct VirtualFileSystem_FileHandle * fileHandle, u32 nEntries, u32 * entriesActuallyRead) {
    struct Fat32_ClusterChainReader clusterReader;

    // setup partition and boot record
        clusterReader.partition = fileHandle -> partition;
        clusterReader.info = (struct Fat32_PartitionInfo *)(fileHandle -> partition -> fileSystemDriverInformation);
        struct Fat32_BootRecord * bootRecord = &(clusterReader.info -> bootRecord);
    
    // setup buffer
        MemoryPage buffer = getMappedPage(true);
        clusterReader.buffer = buffer;
        clusterReader.bufferSize = PAGE_SIZE;
    
    // setup load locations
        struct Fat32_LocationData * location = (struct Fat32_LocationData *)&(fileHandle -> fsDriverLocationData);
        clusterReader.currentCluster = location -> currentCluster;
        clusterReader.currentReadOffset = location -> currentClusterOffset;
        clusterReader.currentReadSize = 0;
    
    // reading parameters
        u8 inLfnEntry = 0;
        u32 readResult = FAT32_READ_CLUSTER_CHAIN_SUCCESS;
        u32 nDescriptors = 0;
        struct FileDescriptor * descriptor = (struct FileDescriptor *)fileHandle -> buffer;
        u32 clusterOffset = 0;

    // TODO -- finish implementation
    while (true) {
        clusterOffset = clusterReader.currentReadOffset;
        u32 clusterReadResult = fat32_ReadClusterChain(&clusterReader);
        if (clusterReadResult == FAT32_READ_CLUSTER_CHAIN_END_OF_CHAIN || clusterReadResult == FAT32_READ_CLUSTER_CHAIN_ERROR) {
            readResult = clusterReadResult;
            break;
        }
        
        // iterate through each file entry
        struct Fat32_Directory_Standard83 * entry = (struct Fat32_Directory_Standard83 *)(clusterReader.buffer);
        u32 nDirEntries = clusterReader.currentReadSize / sizeof(struct Fat32_Directory_Standard83);
        int i; for (i = 0; i < nDirEntries; ++i, ++entry) {
            clusterOffset += sizeof(struct Fat32_Directory_Standard83);

            // end of chain
            if (entry -> fileName[0] == 0) {
                readResult = FAT32_READ_CLUSTER_CHAIN_END_OF_CHAIN;
                break;
            }

            // skip empty entries
            else if (entry -> fileName[0] == 0xE5) {
                continue;
            }

            // long file name
            else if (entry -> fileAttributes == 0x0F) {
                struct Fat32_Directory_LongFileName * lfnEntry = (struct Fat32_Directory_LongFileName *)entry;

                // put together long file name part
                    u8 lfn[13];
                    int i;
                    for (i = 0; i < 5; i++) {
                        lfn[i] = lfnEntry -> nameEntry_first5[2 * i];
                    }
                    for (i = 0; i < 6; i++) {
                        lfn[i + 5] = lfnEntry -> nameEntry_next6[2 * i];
                    }
                    lfn[11] = lfnEntry -> nameEntry_final2[0];
                    lfn[12] = lfnEntry -> nameEntry_final2[2];

                // copy filename to the descriptor
                u32 sequenceNumber = lfnEntry -> sequence & 0x1F;              
                u32 fileNameOffset = (sequenceNumber - 1) * 13;

                if (lfnEntry -> sequence & 0x40) {
                    descriptor -> fileNameLength = sequenceNumber * 13;
                }

                for (i = 0; i < 13; ++i) {
                    if (lfn[i] == 0) {
                        descriptor -> fileNameLength = fileNameOffset + i;
                        break;
                    }
                    descriptor -> fileName[fileNameOffset + i] = lfn[i];
                }

                inLfnEntry = 1;
            }

            // regular 8.3 filename
            else {
                // finish off long file name entry
                if (!inLfnEntry) {
                    fat32_process83fileName(entry -> fileName, descriptor -> fileName, &(descriptor -> fileNameLength));
                }
                
                descriptor -> attributes = (entry -> fileAttributes & 0x10)
                    ? FILE_DESCRIPTOR_ATTRIBUTE_DIRECTORY
                    : FILE_DESCRIPTOR_ATTRIBUTE_FILE;
                
                descriptor -> length = entry -> fileSize;
                
                ++nDescriptors;
                ++descriptor;

                inLfnEntry = 0;
            }

            if (nDescriptors >= nEntries) {
                break;
            }
        }

        // check if enough entries have been read
        if (nDescriptors >= nEntries) {
            break;
        }

        // check if the end or error occured parsing the block
        if (readResult != FAT32_READ_CLUSTER_CHAIN_SUCCESS) {
            break;
        }
    }

    // save position in file
    location -> currentCluster = clusterReader.currentCluster;
    location -> currentClusterOffset = clusterOffset;

    // clean up and return
    returnMappedPage(buffer);
    *entriesActuallyRead = nDescriptors;
    return readResult;
}

u32 fat32_InitializeFileSystemDriver() {
    fat32_FileSystemDriver.initializePartition = fat32_InitializePartition;
    fat32_FileSystemDriver.openFileHandle = fat32_OpenFileHandle;
    fat32_FileSystemDriver.readFile = fat32_ReadFile;
    fat32_FileSystemDriver.readDirectoryEntries = fat32_readDirectoryEntries;
    return GENERIC_SUCCESS_RESULT;
}