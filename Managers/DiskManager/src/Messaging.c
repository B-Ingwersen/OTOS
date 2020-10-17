
#include "Messaging.h"
#include "VirtualFileSystem.h"
#include "OTOSCore/SystemCalls.h"

void messaging_DefaultMessageHandler(PID pid, MemoryPage dataPage) {
    void * data = (void *) ( ((IntegerPointer)dataPage) + SYSTEM_CALL_MESSAGING_DATA_OFFSET );
    u32 operation = *(u32*)data;

    if (operation == DISK_MANAGER_MESSAGING_OPERATION_TEST) {
        messaging_Test(pid, data);
    }
    else if (operation == DISK_MANAGER_MESSAGING_OPERATION_OPEN_FILE_HANDLE) {
        messaging_OpenFileHandle(pid, data);
    }
    else if (operation == DISK_MANAGER_MESSAGING_OPERATION_READ_FILE_HANDLE) {
        messaging_ReadFileHandle(pid, data);
    }
    else if (operation == DISK_MANAGER_MESSAGING_OPERATION_CLOSE_FILE_HANDLE) {
        messaging_CloseFileHandle(pid, data);
    }
    else if (operation == DISK_MANAGER_MESSAGING_OPERATION_READ_DIRECTORY_ENTRIES) {
        messaging_ReadDirectoryEntries(pid, data);
    }

    systemCall_Messaging_MessageReturn();
}

u32 messaging_InitializeMessaging() {
    u32 result = systemCall_Messaging_Initialize((MemoryPage)0xFF000000);
    u32 result2 = systemCall_Messaging_SetMessageHandler(messaging_DefaultMessageHandler);
}

void messaging_Test(PID pid, void * dataPointer) {
    struct DiskManager_Messaging_Test * data = (struct DiskManager_Messaging_Test *)dataPointer;
    data -> result = GENERIC_SUCCESS_RESULT;
}

void messaging_OpenFileHandle(PID pid, void * dataPointer) {
    struct DiskManager_Messaging_OpenFile * data = (struct DiskManager_Messaging_OpenFile *)dataPointer;

    if (data -> fileNameLength > 2000 || data -> bufferNPages > 256 || data -> bufferNPages < 1) {
        data -> result = GENERIC_ERROR_RESULT;
        return;
    }

    SharedMemory_SegmentID shmID;
    u32 handle = virtualFileSystem_OpenFileHandle(data -> fileName, data -> fileNameLength, data -> bufferNPages, pid, &(data -> sharedMemorySegment), &(data -> attributes), &(data -> length));
    if (handle == VIRTUAL_FILE_SYSTEM_ERROR_HANDLE) {
        data -> result = GENERIC_ERROR_RESULT;
        return;
    }

    data -> fileHandleID = handle;
    data -> result = GENERIC_SUCCESS_RESULT;
}

void messaging_ReadFileHandle(PID pid, void * dataPointer) {

    struct DiskManager_Messaging_ReadFile * data = (struct DiskManager_Messaging_ReadFile *)dataPointer;

    u32 result = virtualFileSystem_ReadFileHandle(data -> fileHandleID, pid, data -> filePosition, data -> readSize, &(data -> bytesActuallyRead));
    data -> result = result;

    systemCall_Messaging_MessageReturn();
}

void messaging_CloseFileHandle(PID pid, void * dataPointer) {
    struct DiskManager_Messaging_CloseFile * data = (struct DiskManager_Messaging_CloseFile *)dataPointer;

    u32 result = virtualFileSystem_CloseFileHandle(data -> fileHandleID, pid);
    data -> result = result;
}

void messaging_ReadDirectoryEntries(PID pid, void * dataPointer) {
    struct DiskManager_Messaging_ReadDirectoryEntries * data = (struct DiskManager_Messaging_ReadDirectoryEntries *)dataPointer;

    u32 result = virtualFileSystem_ReadDirectoryEntries(data -> fileHandleID, pid, data -> nEntries, &(data -> entriesActuallyRead));
    data -> result = result;
}
