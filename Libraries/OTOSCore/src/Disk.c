#include "Disk.h"
#include "SystemCalls.h"
#include "Synchronization.h"
#include "MemoryAllocation.h"
#include "SharedMemory.h"
#include "Threads.h"
#include "CStandardLibrary/string.h"

u32 disk_Test() {
    MemoryPage messageDataPage = threads_GetLocalStorage() -> messagingDataPage;
    struct DiskManager_Messaging_Test * data = (struct DiskManager_Messaging_Test *)( ((IntegerPointer)messageDataPage) + SYSTEM_CALL_MESSAGING_DATA_OFFSET);
    data -> operation = DISK_MANAGER_MESSAGING_OPERATION_TEST;

    u32 result = systemCall_Messaging_MessageProcess(DISK_MANAGER_PID, messageDataPage);
    if (result != GENERIC_ERROR_RESULT) {
        result = data -> result;
    }
    return result;
}

u32 disk_OpenFile_Allocator(u8 * fileName, u32 fileNameLength, u32 bufferNPages, struct Disk_FileHandle * fileHandle, struct MemoryAllocation_BufferAllocator * allocator) {
    MemoryPage messageDataPage = threads_GetLocalStorage() -> messagingDataPage;
    struct DiskManager_Messaging_OpenFile * data = (struct DiskManager_Messaging_OpenFile *)( ((IntegerPointer)messageDataPage) + SYSTEM_CALL_MESSAGING_DATA_OFFSET);
    
    data -> operation = DISK_MANAGER_MESSAGING_OPERATION_OPEN_FILE_HANDLE;
    data -> fileNameLength = fileNameLength;
    data -> bufferNPages = bufferNPages;
    strncpy(data -> fileName, fileName, fileNameLength);

    MemoryPage buffer = memoryAllocation_GetUnmappedPages(allocator, bufferNPages);
    if (buffer == NULL) {
        return GENERIC_ERROR_RESULT;
    }

    u32 result = systemCall_Messaging_MessageProcess(DISK_MANAGER_PID, messageDataPage);
    if (result != GENERIC_SUCCESS_RESULT || data -> result != GENERIC_SUCCESS_RESULT) {
        memoryAllocation_ReturnUnmappedPages(allocator, buffer, bufferNPages);
        return GENERIC_ERROR_RESULT;
    }

    result = sharedMemory_GetSegment(DISK_MANAGER_PID, data -> sharedMemorySegment, buffer, bufferNPages);

    fileHandle -> attributes = data -> attributes;
    fileHandle -> length = data -> length;
    fileHandle -> sharedMemoryID = data -> sharedMemorySegment;
    fileHandle -> fileHandle = data -> fileHandleID;
    fileHandle -> buffer = buffer;
    fileHandle -> bufferNPages = bufferNPages;

    return GENERIC_SUCCESS_RESULT;
}

u32 disk_ReadFile(struct Disk_FileHandle * fileHandle, u64 filePosition, u64 readSize, u64 * bytesRead, void * buffer) {
    MemoryPage messageDataPage = threads_GetLocalStorage() -> messagingDataPage;
    struct DiskManager_Messaging_ReadFile * data = (struct DiskManager_Messaging_ReadFile *)( ((IntegerPointer)messageDataPage) + SYSTEM_CALL_MESSAGING_DATA_OFFSET);

    data -> operation = DISK_MANAGER_MESSAGING_OPERATION_READ_FILE_HANDLE;
    data -> fileHandleID = fileHandle -> fileHandle;

    //IntegerPointer p = ((((IntegerPointer)(fileHandle -> buffer)) >> 10) & 0xFFFFFFFC) + 0xFFC00000;

    u64 goalBytesRead = readSize; // how much should be real
    u64 totalBytesRead = 0; // how much actually has been read
    while (true) {

        // limit the read to the IPC buffer size if needed
        if (readSize > (PAGE_SIZE * fileHandle -> bufferNPages) ) {
            data -> readSize = (PAGE_SIZE * fileHandle -> bufferNPages);
        }
        else {
            data -> readSize = readSize;
        }
        data -> filePosition = filePosition;

        // try reading the data
        u32 result = systemCall_Messaging_MessageProcess(DISK_MANAGER_PID, messageDataPage);

        // abort if there was an error
        if (result == GENERIC_ERROR_RESULT || data -> result == DISK_MANAGER_MESSAGING_READ_FILE_HANDLE_ERROR) {
            return DISK_READ_FILE_ERROR;
        }
        //return if the end of the file is reached
        else if (data -> result == DISK_MANAGER_MESSAGING_READ_FILE_HANDLE_END_OF_FILE) {
            memcpy((u8*)buffer + totalBytesRead, fileHandle -> buffer, data -> bytesActuallyRead);
            *bytesRead = totalBytesRead + data -> bytesActuallyRead;
            return DISK_READ_FILE_END_OF_FILE;
        }

        // update position in the file
        memcpy((u8*)buffer + totalBytesRead, fileHandle -> buffer, data -> bytesActuallyRead);
        totalBytesRead += data -> bytesActuallyRead;
        filePosition += data -> bytesActuallyRead;
        readSize -= data -> bytesActuallyRead;

        // check if the desired amount has been read
        if (totalBytesRead >= goalBytesRead) {
            *bytesRead = totalBytesRead;
            return DISK_READ_FILE_SUCCESS;
        }
    }
}

u32 disk_ReadDirectoryEntries(struct Disk_FileHandle * fileHandle, struct Disk_FileDescriptor * fileDescriptors, u32 nEntries, u32 * entriesRead) {
    MemoryPage messageDataPage = threads_GetLocalStorage() -> messagingDataPage;
    struct DiskManager_Messaging_ReadDirectoryEntries * data = (struct DiskManager_Messaging_ReadDirectoryEntries *)( ((IntegerPointer)messageDataPage) + SYSTEM_CALL_MESSAGING_DATA_OFFSET);

    u32 totalEntriesRead = 0;
    while (true) {
        data -> operation = DISK_MANAGER_MESSAGING_OPERATION_READ_DIRECTORY_ENTRIES;
        data -> fileHandleID = fileHandle -> fileHandle;
        data -> nEntries = nEntries;

        if (sizeof(struct DiskManager_FileDescriptor) * nEntries > fileHandle -> bufferNPages * PAGE_SIZE) {
            data -> nEntries = fileHandle -> bufferNPages * PAGE_SIZE / sizeof(struct DiskManager_FileDescriptor);
        }

        u32 result = systemCall_Messaging_MessageProcess(DISK_MANAGER_PID, messageDataPage);

        if (result == GENERIC_ERROR_RESULT || data -> result == DISK_MANAGER_MESSAGING_READ_FILE_HANDLE_ERROR) {
            return DISK_READ_FILE_ERROR;
        }

        memcpy(fileDescriptors + totalEntriesRead, fileHandle -> buffer, data -> entriesActuallyRead * sizeof(struct DiskManager_FileDescriptor));
        totalEntriesRead += data -> entriesActuallyRead;
        nEntries -= data -> entriesActuallyRead;

        if (data -> result == DISK_MANAGER_MESSAGING_READ_FILE_HANDLE_END_OF_FILE) {
            break;
        }
        if (nEntries == 0) {
            break;
        }
    }

    *entriesRead = totalEntriesRead;

    return data -> result;
}

u32 disk_CloseFile_Allocator(struct Disk_FileHandle * fileHandle, struct MemoryAllocation_BufferAllocator * allocator) {
    MemoryPage messageDataPage = threads_GetLocalStorage() -> messagingDataPage;
    struct DiskManager_Messaging_ReadFile * data = (struct DiskManager_Messaging_ReadFile *)( ((IntegerPointer)messageDataPage) + SYSTEM_CALL_MESSAGING_DATA_OFFSET);

    data -> operation = DISK_MANAGER_MESSAGING_OPERATION_CLOSE_FILE_HANDLE;
    data -> fileHandleID = fileHandle -> fileHandle;

    u32 result = systemCall_Messaging_MessageProcess(DISK_MANAGER_PID, messageDataPage);
    if (result != GENERIC_ERROR_RESULT) {
        memoryAllocation_ReturnUnmappedPages(allocator, fileHandle -> buffer, fileHandle -> bufferNPages);
    }
    return result;
}

MemoryPage disk_ReadFullFileToBuffer_Allocator(u8 * fileName, u32 fileNameLength, u32 maxBufferSize, u64 * fileSize, u32 * bufferNPages, struct MemoryAllocation_BufferAllocator * allocator) {
    struct Disk_FileHandle file;
    u32 result = disk_OpenFile_Allocator(fileName, fileNameLength, 4, &file, allocator);
    if (result == GENERIC_ERROR_RESULT) {
        *fileSize = 0;
        return NULL;
    }

    *fileSize = file.length;
    u32 bufferSize = (file.length + PAGE_SIZE - 1) / PAGE_SIZE;
    if ((u64)bufferSize * PAGE_SIZE >= maxBufferSize) {
        return NULL;
    }

    MemoryPage buffer = memoryAllocation_GetMappedPages(allocator, bufferSize, true);
    if (buffer == NULL) {
        disk_CloseFile_Allocator(&file, allocator);
        return NULL;
    }

    u64 bytesRead = 0;
    result = disk_ReadFile(&file, 0, file.length, &bytesRead, buffer);
    if (result == DISK_READ_FILE_ERROR) {
        memoryAllocation_ReturnMappedPages(allocator, buffer, bufferSize);
        disk_CloseFile_Allocator(&file, allocator);
        return NULL;
    }

    *fileSize = bytesRead;
    *bufferNPages = bufferSize;

    disk_CloseFile_Allocator(&file, allocator);

    return buffer;
}
