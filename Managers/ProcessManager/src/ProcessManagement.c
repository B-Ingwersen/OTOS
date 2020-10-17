
#include "ProcessManagement.h"
#include "OTOSCore/SystemCalls.h"
#include "OTOSCore/Threads.h"
#include "OTOSCore/MemoryAllocation.h"
#include "OTOSCore/CStandardLibrary/string.h"
#include "OTOSCore/Disk.h"
#include "Debugging.h"

struct ProcessInfo ** processInfoTable = NULL;
u32 * usedPIDTable = NULL;
Synchronization_Spinlock processInfoTable_spinlock = 0;

void waitForProcessToInitialize(PID pid) {
    u32 result;
    while (true) {
        MemoryPage messageDataPage = threads_GetLocalStorage() -> messagingDataPage;
        struct { u32 operation; u32 result; } ExactBinaryStructure * data = (void*)( ((IntegerPointer)messageDataPage) + SYSTEM_CALL_MESSAGING_DATA_OFFSET);
        data -> operation = 0;

        u32 result = systemCall_Messaging_MessageProcess(pid, messageDataPage);
        if (result != GENERIC_ERROR_RESULT) {
            result = data -> result;
        }
        
        if (result == GENERIC_SUCCESS_RESULT) {
            return;
        }
        systemCall_ProcessThread_YieldTimeSlice();
    }
}

void initializeProcessManagement() {
    MemoryPage tablesBaseAddress = getMappedPage(true);
    struct ProcessInfo** processTable = (struct ProcessInfo**)tablesBaseAddress;
    u32 * pidTable = (u32 *) ( ((IntegerPointer)tablesBaseAddress) + MAXIMUM_NUMBER_OF_PROCESSES * sizeof(struct ProcessInfo*) );
    
    u32 i;
    for (i = 0; i < MAXIMUM_NUMBER_OF_PROCESSES; i++) {
        processTable[i] = NULL;
        pidTable[i] = 0x00010000;
    }

    struct ProcessInfo* processManager_ProcessInfo = (struct ProcessInfo*)getMappedPage(true);
    struct ProcessInfo* diskManager_ProcessInfo = (struct ProcessInfo*)getMappedPage(true);

    //setup entry for processManager
        processManager_ProcessInfo-> pid = 0x00010001;
        strncpy(processManager_ProcessInfo-> fullFileName, "/Managers/ProcessManager", 24);
        strncpy(processManager_ProcessInfo-> searchableName, "ProcessManager", 14);
        processManager_ProcessInfo-> fullFileName_length = 24;
        processManager_ProcessInfo-> searchableName_length = 14;

    //setup entry for diskManager
        diskManager_ProcessInfo-> pid = 0x00010002;
        strncpy(diskManager_ProcessInfo-> fullFileName, "/Managers/DiskManager", 21);
        strncpy(diskManager_ProcessInfo-> searchableName, "DiskManager", 11);
        diskManager_ProcessInfo-> fullFileName_length = 21;
        diskManager_ProcessInfo-> searchableName_length = 11;
    
    processTable[1] = processManager_ProcessInfo;
    processTable[2] = diskManager_ProcessInfo;

    //asm volatile("jmp $"::"a"(&processInfoTable),"b"(0x5432), "c"(&processInfoTable_spinlock));
    processInfoTable = processTable;
    usedPIDTable = pidTable;
}

PID createProcess(u8 * name, u32 nameLength, u8 * searchableName, u32 searchableNameLength,
    MemoryPage discardableCodeBuffer, u32 codeBufferNPages, u32 * initialPermissions, u32 initialPermissionsLength) {

    synchronization_OpenSpinlock(&(processInfoTable_spinlock));

    //setup the process info
        u32 i;
        if (searchableName != NULL) {
            for (i = 0; i < MAXIMUM_NUMBER_OF_PROCESSES; i++) {
                struct ProcessInfo * entry = processInfoTable[i];
                if (entry != NULL) {
                    if (entry -> searchableName_length == searchableNameLength && strncmp(entry -> searchableName, searchableName, searchableNameLength) == 0) {
                        synchronization_CloseSpinlock(&(processInfoTable_spinlock));
                        return ERROR_PID;
                    }
                }
            }
        }
        for (i = 1; i < MAXIMUM_NUMBER_OF_PROCESSES; i++) {
            if (processInfoTable[i] == NULL) {
                break;
            }
        }
        if (i == MAXIMUM_NUMBER_OF_PROCESSES) {
            synchronization_CloseSpinlock(&(processInfoTable_spinlock));
            return ERROR_PID;
        }

        PID pid = usedPIDTable[i] + i;
        usedPIDTable[i] += 0x0001000;

        struct ProcessInfo * processInfo = (struct ProcessInfo *)getMappedPage(true);
        processInfoTable[i] = processInfo;
        
        processInfo -> pid = pid;
        strncpy(processInfo -> fullFileName, name, nameLength);
        processInfo -> fullFileName_length = nameLength;
        if (searchableName != NULL) {
            strncpy(processInfo -> searchableName, searchableName, searchableNameLength);
            processInfo -> searchableName_length = searchableNameLength;
        }
        else {
            processInfo -> searchableName_length = 0;
        }

    //setup the actual process
        MemoryPage codeLocation = (MemoryPage)0x100000;
        MemoryPage threadContextLocation = (MemoryPage)0xFFB01000;
        MemoryPage stackPage = (MemoryPage)0xFFB00000;
        MemoryPage stackLocation = (MemoryPage)stackPage + PAGE_SIZE - sizeof(void *);
        
        systemCall_ProcessThread_CreateProcess(pid);

        if (initialPermissions != NULL) {
            u32 i;
            for (i = 0; i < initialPermissionsLength; i++) {
                systemCall_ProcessThread_ChangeProcessPermissions(pid, initialPermissions[i], 1);
            }
        }

        systemCall_Memory_MoveMemory(0, pid, discardableCodeBuffer, codeLocation, codeBufferNPages, SYSTEM_CALL_MEMORY_MAP_TYPE_READ_WRITE, SYSTEM_CALL_NO_PERMISSION, SYSTEM_CALL_PROCESS_PERMISSION);
        systemCall_Memory_MapNewMemory(pid, stackPage, 1, SYSTEM_CALL_MEMORY_MAP_TYPE_READ_WRITE, SYSTEM_CALL_PROCESS_PERMISSION);
        systemCall_ProcessThread_CreateThread(pid, threadContextLocation, codeLocation, stackLocation , SYSTEM_CALL_PROCESS_PERMISSION);
        systemCall_ProcessThread_ChangeThreadExecution(pid, threadContextLocation, SYSTEM_CALL_CHANGE_THREAD_EXECUTION_SCHEDULE, SYSTEM_CALL_PROCESS_PERMISSION);

    synchronization_CloseSpinlock(&(processInfoTable_spinlock));

    return pid;
}

PID createProcessFromDiskFile(u8 * fileName, u32 fileNameLength, u8 * searchableName, u32 searchableNameLength, u32 * initialPermissions, u32 initialPermissionsLength) {
    u64 fileSize; u32 bufferNPages;
    MemoryPage * buffer = disk_ReadFullFileToBuffer(fileName, fileNameLength, 255 * 1024 * 1024, &fileSize, &bufferNPages);

    if (buffer == NULL) {
        return ERROR_PID;
    }
    PID pid = createProcess(fileName, fileNameLength, searchableName, searchableNameLength, buffer, bufferNPages, initialPermissions, initialPermissionsLength);
    if (pid == ERROR_PID) {
        returnMappedPages(buffer, bufferNPages);
    }

    return pid;
}

PID getProcessPID_SearchableName(u8 * name, u32 nameLength) {
    if (nameLength > 256) {
        return ERROR_PID;
    }

    synchronization_OpenSpinlock(&(processInfoTable_spinlock));

    u32 i;
    for (i = 0; i < MAXIMUM_NUMBER_OF_PROCESSES; i++) {
        struct ProcessInfo * entry = processInfoTable[i];
        if (entry != NULL) {
            if (entry -> searchableName_length == nameLength && strncmp(entry -> searchableName, name, nameLength) == 0) {
                synchronization_CloseSpinlock(&(processInfoTable_spinlock));
                return entry -> pid;
            }
        }
    }

    synchronization_CloseSpinlock(&(processInfoTable_spinlock));
    return ERROR_PID;
}