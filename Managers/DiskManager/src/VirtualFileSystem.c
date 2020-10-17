
#include "VirtualFileSystem.h"
#include "Messaging.h"
#include "OTOSCore/Synchronization.h"
#include "OTOSCore/CStandardLibrary/string.h"

struct DiskDescriptor * virtualFileSystem_RootDevice = NULL;
struct PartitionDescriptor * virtualFileSystem_RootPartition = NULL;

u32 virtualFileSystem_NDevices = 0;
struct DiskDescriptor ** virtualFileSystem_DeviceList = NULL;

struct VirtualFileSystem_FileHandle ** virtualFileSystem_FileHandles = NULL;
Synchronization_Spinlock virtualFileSystem_FileHandles_Spinlock = 0;

struct DiskDescriptor * virtualFileSystem_GetDevice(u8 * name, u32 nameLength) {
    if (nameLength > 31) {
        return NULL;
    }

    struct DiskDescriptor ** deviceList = virtualFileSystem_DeviceList;
    u32 nDevices = virtualFileSystem_NDevices;

    u32 i;
    for (i = 0; i < nDevices; i++) {
        u8 * deviceName = deviceList[i] -> name;
        if ( strncmp(deviceName, name, nameLength) == 0 && deviceName[i] == 0 ) {
            return deviceList[i];
        }
    }
    return NULL;
}

struct PartitionDescriptor * virtualFileSystem_GetPartition(struct DiskDescriptor * disk, u8 * name, u32 nameLength) {
    u32 i;
    for (i = 0; i < disk -> nPartitions; i++) {
        u8 * partitionName = disk -> partitions[i].name;
        if (strncmp(partitionName, name, nameLength) == 0 && partitionName[nameLength] == 0) {
            return &(disk -> partitions[i]);
        }
    }
    return NULL;
}

u32 virtualFileSystem_ResolveFileName(u8 * fileName, u32 fileNameLength, struct PartitionDescriptor ** partitionReturnAddress, u8 ** fileNameReturnAddress, u32 * fileNameLengthReturnAddress) {
    struct DiskDescriptor * disk = virtualFileSystem_RootDevice;
    struct PartitionDescriptor * partition = virtualFileSystem_RootPartition;

    i32 i;
    if (fileNameLength > 4) {
        if (strncmp(fileName, "//d:", 4) == 0) {
            fileName += 4;
            fileNameLength -= 4;

            i = 0;
            while (i < fileNameLength && fileName[i] != '/') {i++;}

            disk = virtualFileSystem_GetDevice(fileName, i);
            if (disk == NULL) {
                return GENERIC_ERROR_RESULT;
            }

            fileName += i;
            fileNameLength -= i;
            if (fileNameLength <= 4) {
                return GENERIC_ERROR_RESULT;
            }
            if (strncmp(fileName, "//p:", 4) != 0) {
                return GENERIC_ERROR_RESULT;
            }
        }

        if (strncmp(fileName, "//p:", 4) == 0) {
            fileName += 4;
            fileNameLength -= 4;

            i = 0;
            while (i < fileNameLength && fileName[i] != '/') {i++;}

            partition = virtualFileSystem_GetPartition(disk, fileName, i);
            if (partition == NULL) {
                return GENERIC_ERROR_RESULT;
            }

            fileName += i;
            fileNameLength -= i;
        }
    }

    if (fileNameLength <= 0) {
        return GENERIC_ERROR_RESULT;
    }
    if (fileName[0] != '/') {
        return GENERIC_ERROR_RESULT;
    }

    *partitionReturnAddress = partition;
    *fileNameReturnAddress = fileName;

    fileName += 1;
    fileNameLength -= 1;
    i32 treeLevel = 0;
    while (true) {
        i = 0;
        while (i < fileNameLength && fileName[i] != '/') {i++;}

        if (i == 0 && treeLevel > 0) { // if at root level, allow 
            return GENERIC_ERROR_RESULT;
        }
        
        if (i == 1 && strncmp(fileName, ".", 1) == 0) {
            u8 * copyPoint = fileName + 1;
            u32 copyLength = fileNameLength - 1;
            fileName--;
            memmove(fileName, copyPoint, copyLength);
            fileNameLength = copyLength;
            i = 0;

            if (fileNameLength == 0 && treeLevel == 0) {
                fileName[0] = '/';
            }
        }
        else if (i == 2 && strncmp(fileName, "..", 2) == 0) {
            if (treeLevel <= 0) {
                return GENERIC_ERROR_RESULT;
            }
            treeLevel--;
            u8 * copyPoint = fileName + 2;
            u32 copyLength = fileNameLength - 2;
            i = -1; while (fileName[i - 1] != '/') {i--;}

            fileName += (i - 1);
            memmove(fileName, copyPoint, copyLength);
            fileNameLength = copyLength;
            i = 0;

            if (fileNameLength == 0 && treeLevel == 0) {
                fileName[0] = '/';
            }
        }
        else {
            treeLevel++;
        }

        if (i >= fileNameLength) {
            *fileNameLengthReturnAddress = ( fileName - (*fileNameReturnAddress) ) + i;
            return GENERIC_SUCCESS_RESULT;
        }

        fileName += i + 1;
        fileNameLength -= (i + 1);
    }
}

u32 virtualFileSystem_OpenFileHandle(u8 * fileName, u32 fileNameLength, u32 bufferNPages, PID pid, SharedMemory_SegmentID * sharedMemorySegment, u32 * attributes, u64 * length) {
    synchronization_OpenSpinlock(&virtualFileSystem_FileHandles_Spinlock);

    // get the partitions from the filename, and get the file path relative to
    // that partition
    struct PartitionDescriptor * vfs_partition;
    u8 * vfs_fileName;
    u32 vfs_fileNameLength;
    u32 result = virtualFileSystem_ResolveFileName(fileName, fileNameLength, &vfs_partition, &vfs_fileName, &vfs_fileNameLength);
    if (result == GENERIC_ERROR_RESULT) {
        synchronization_CloseSpinlock(&virtualFileSystem_FileHandles_Spinlock);
        return VIRTUAL_FILE_SYSTEM_ERROR_HANDLE;
    }

    // get a new file handle id
    u32 i;
    for (i = 0; i < VIRTUAL_FILE_SYSTEM_MAX_FILE_HANDLES; i++) {
        if (virtualFileSystem_FileHandles[i] == NULL) {
            break;
        }
    }
    if (i == VIRTUAL_FILE_SYSTEM_MAX_FILE_HANDLES) {
        synchronization_CloseSpinlock(&virtualFileSystem_FileHandles_Spinlock);
        return VIRTUAL_FILE_SYSTEM_ERROR_HANDLE;
    }
    u32 fileHandleID = i;

    // construct the file handle and call the file system driver
    struct VirtualFileSystem_FileHandle * fileHandle = getMappedPage(true);
    fileHandle -> partition = vfs_partition;
    strncpy(fileHandle -> fullFilePath, vfs_fileName, vfs_fileNameLength);
    fileHandle -> fullFilePath_Length = vfs_fileNameLength;
    fileHandle -> handlingProcess = pid;
    result = vfs_partition -> fsDriver -> openFileHandle(fileHandle);
    if (result == GENERIC_ERROR_RESULT) {
        returnMappedPage(fileHandle);
        synchronization_CloseSpinlock(&virtualFileSystem_FileHandles_Spinlock);
        return VIRTUAL_FILE_SYSTEM_ERROR_HANDLE;
    }

    // construct the shared memory segment for the transfer buffer
    fileHandle -> sharedMemorySegment = sharedMemory_MapAndCreateSegment(bufferNPages, &(fileHandle -> buffer));
    fileHandle -> bufferNPages = bufferNPages;
    if (fileHandle -> sharedMemorySegment == SHARED_MEMORY_ERROR_SEGMENT) {
        returnMappedPage(fileHandle);
        synchronization_CloseSpinlock(&virtualFileSystem_FileHandles_Spinlock);
        return VIRTUAL_FILE_SYSTEM_ERROR_HANDLE;
    }
    sharedMemory_GrantProcessPermission(fileHandle -> sharedMemorySegment, pid, true);

    // set return parameters
    *sharedMemorySegment = fileHandle -> sharedMemorySegment;
    *attributes = fileHandle -> fileDescriptor.attributes;
    *length = fileHandle -> fileDescriptor.length;

    // clean up and mark the file handle as used
    synchronization_CloseSpinlock( &(fileHandle -> lock) );
    virtualFileSystem_FileHandles[fileHandleID] = fileHandle;
    synchronization_CloseSpinlock(&virtualFileSystem_FileHandles_Spinlock);

    return fileHandleID;
}

u32 virtualFileSystem_ReadFileHandle(u32 handleNumber, PID pid, u64 filePosition, u64 readSize, u64 * actualReadSize) {
    if (handleNumber >= VIRTUAL_FILE_SYSTEM_MAX_FILE_HANDLES) {
        return FILE_SYSTEM_DRIVER_ERROR;
    }

    synchronization_OpenSpinlock(&virtualFileSystem_FileHandles_Spinlock);

    struct VirtualFileSystem_FileHandle * fileHandle = virtualFileSystem_FileHandles[handleNumber];
    if (fileHandle == NULL) {
        synchronization_CloseSpinlock(&virtualFileSystem_FileHandles_Spinlock);
        return FILE_SYSTEM_DRIVER_ERROR;
    }

    //file handle read operations:
        synchronization_OpenSpinlock( &(fileHandle -> lock) );
        synchronization_CloseSpinlock( &virtualFileSystem_FileHandles_Spinlock );

        if (fileHandle -> handlingProcess != pid) {
            synchronization_CloseSpinlock( &(fileHandle -> lock) );
            return FILE_SYSTEM_DRIVER_ERROR;
        }
        if ( (fileHandle -> fileDescriptor.attributes & FILE_DESCRIPTOR_ATTRIBUTE_DIRECTORY) != 0) {
            synchronization_CloseSpinlock( &(fileHandle -> lock) );
            return FILE_SYSTEM_DRIVER_ERROR;
        }

        if (readSize >= PAGE_SIZE * fileHandle -> bufferNPages) {
            readSize = PAGE_SIZE * fileHandle -> bufferNPages;
        }
        
        bool32 endOfFile = false;
        if ( filePosition >= fileHandle -> fileDescriptor.length) {
            endOfFile = true;
            readSize = 0;
        }
        else if ( filePosition + readSize > fileHandle -> fileDescriptor.length) {
            endOfFile = true;
            readSize = fileHandle -> fileDescriptor.length - filePosition;
        }

        IntegerPointer p = ((((IntegerPointer)(fileHandle -> buffer)) >> 10) & 0xFFFFFFFC) + 0xFFC00000;

        u32 result = fileHandle -> partition -> fsDriver -> readFile(fileHandle, filePosition, readSize, actualReadSize);
        if (endOfFile && result == FILE_SYSTEM_DRIVER_SUCCESS) {
            result = FILE_SYSTEM_DRIVER_END_OF_FILE;
        }

        synchronization_CloseSpinlock( &(fileHandle -> lock) );

    return result;
}

u32 virtualFileSystem_ReadDirectoryEntries(u32 handleNumber, PID pid, u32 nEntries, u32 * entriesActuallyRead) {
    // quick checks for invalid file handle
    if (handleNumber >= VIRTUAL_FILE_SYSTEM_MAX_FILE_HANDLES) {
        return FILE_SYSTEM_DRIVER_ERROR;
    }
    synchronization_OpenSpinlock(&virtualFileSystem_FileHandles_Spinlock);
    struct VirtualFileSystem_FileHandle * fileHandle = virtualFileSystem_FileHandles[handleNumber];
    if (fileHandle == NULL) {
        synchronization_CloseSpinlock(&virtualFileSystem_FileHandles_Spinlock);
        return FILE_SYSTEM_DRIVER_ERROR;
    }

    // read directory entries operation
        synchronization_OpenSpinlock( &(fileHandle -> lock) );
        synchronization_CloseSpinlock( &virtualFileSystem_FileHandles_Spinlock );

        // verify correct process
        if (fileHandle -> handlingProcess != pid) {
            synchronization_CloseSpinlock( &(fileHandle -> lock) );
            return FILE_SYSTEM_DRIVER_ERROR;
        }

        // check that the file handle is a directory, not a file
        if ( (fileHandle -> fileDescriptor.attributes & FILE_DESCRIPTOR_ATTRIBUTE_FILE) != 0) {
            synchronization_CloseSpinlock( &(fileHandle -> lock) );
            return FILE_SYSTEM_DRIVER_ERROR;
        }

        // cap read size at available space
        if (nEntries * sizeof(struct FileDescriptor) > PAGE_SIZE * fileHandle -> bufferNPages) {
            nEntries = PAGE_SIZE * fileHandle -> bufferNPages / sizeof(struct FileDescriptor);
        }

        u32 result = fileHandle -> partition -> fsDriver -> readDirectoryEntries(fileHandle, nEntries, entriesActuallyRead);

        synchronization_CloseSpinlock( &(fileHandle -> lock) );

    return result;
}

u32 virtualFileSystem_CloseFileHandle(u32 handleNumber, PID pid) {
    if (handleNumber >= VIRTUAL_FILE_SYSTEM_MAX_FILE_HANDLES) {
        return GENERIC_ERROR_RESULT;
    }

    synchronization_OpenSpinlock(&virtualFileSystem_FileHandles_Spinlock);

    struct VirtualFileSystem_FileHandle * fileHandle = virtualFileSystem_FileHandles[handleNumber];
    if (fileHandle == NULL) {
        synchronization_CloseSpinlock(&virtualFileSystem_FileHandles_Spinlock);
        return GENERIC_ERROR_RESULT;
    }

    synchronization_OpenSpinlock( &(fileHandle -> lock) );

    if (fileHandle -> handlingProcess != pid) {
        synchronization_CloseSpinlock( &(fileHandle -> lock) );
        synchronization_CloseSpinlock(&virtualFileSystem_FileHandles_Spinlock);
        return GENERIC_ERROR_RESULT;
    }

    sharedMemory_DestroySegment(fileHandle -> sharedMemorySegment);
    returnMappedPage(fileHandle);
    virtualFileSystem_FileHandles[handleNumber] = NULL;

    synchronization_CloseSpinlock(&virtualFileSystem_FileHandles_Spinlock);
    return GENERIC_SUCCESS_RESULT;
}

u32 virtualFileSystem_Initialize(struct DiskDescriptor * rootDisk, u32 rootPartitionNumber) {
    if (rootDisk -> nPartitions <= rootPartitionNumber) {
        return GENERIC_ERROR_RESULT;
    }

    strcpy(rootDisk -> name, "RootDevice");
    strcpy(rootDisk -> partitions[rootPartitionNumber].name, "RootPartition");
    virtualFileSystem_RootDevice = rootDisk;
    virtualFileSystem_RootPartition = &(rootDisk -> partitions[rootPartitionNumber]);

    struct DiskDescriptor ** deviceList = getMappedPage(true);
    deviceList[0] = rootDisk;

    virtualFileSystem_DeviceList = deviceList;
    virtualFileSystem_NDevices = 1;

    virtualFileSystem_FileHandles = getMappedPage(true);
    u32 i;
    for (i = 0; i < VIRTUAL_FILE_SYSTEM_MAX_FILE_HANDLES; i++) {
        virtualFileSystem_FileHandles[i] = NULL;
    }
    synchronization_InitializeSpinlock(&virtualFileSystem_FileHandles_Spinlock);

    return GENERIC_SUCCESS_RESULT;
}