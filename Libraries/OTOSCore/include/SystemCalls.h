#ifndef OTOS_CORE___SYSTEM_CALLS_H
#define OTOS_CORE___SYSTEM_CALLS_H

#define SYSTEM_CALL_RESULT_FAIL 0x0
#define SYSTEM_CALL_RESULT_SUCCESS 0x1

#define SYSTEM_CALL_NO_PERMISSION 0x0
#define SYSTEM_CALL_PROCESS_PERMISSION 0xFFFFFFFF

#define SYSTEM_CALL_MEMORY_MAP_TYPE_READ_ONLY 0x0
#define SYSTEM_CALL_MEMORY_MAP_TYPE_READ_WRITE 0x1

#define SYSTEM_CALL_BIOS_CALL_RW_NONE 0
#define SYSTEM_CALL_BIOS_CALL_READ 1
#define SYSTEM_CALL_BIOS_CALL_WRITE 2

#define SYSTEM_CALL_CHANGE_THREAD_EXECUTION_YIELD_TIME_SLICE 0
#define SYSTEM_CALL_CHANGE_THREAD_EXECUTION_SCHEDULE 1
#define SYSTEM_CALL_CHANGE_THREAD_EXECUTION_HALT 2

#define SYSTEM_CALL_SHARED_MEMORY_PERMISSION_ACCESS 0x1
#define SYSTEM_CALL_SHARED_MEMORY_PERMISSION_WRITE 0x2

#define SYSTEM_CALL_MESSAGING_DATA_OFFSET 0x800

#include "Definitions.h"

typedef u32 SystemCallResult;
typedef u32 SystemCallPermission;

struct SystemCall_BiosCall_Variables {
    u32 interruptNumber;
    u32 edi;
    u32 esi;
    u32 ebp;
    u32 esp_unused;
    u32 ebx;
    u32 edx;
    u32 ecx;
    u32 eax;
    u32 eflags;
} ExactBinaryStructure;

SystemCallResult systemCall_Memory_MapNewMemory(PID pid, MemoryPage basePage, u32 nPages, u32 readWrite, SystemCallPermission permission);

SystemCallResult systemCall_Memory_UnmapMemory(PID pid, MemoryPage basePage, u32 nPages, u32 allowKernelMemory, SystemCallPermission permission);

SystemCallResult systemCall_Memory_MoveMemory(PID sourcePID, PID destinationPID, MemoryPage sourceBase, MemoryPage destinationBase, u32 nPages, u32 readWrite, SystemCallPermission sourcePermission, SystemCallPermission destinationPermission);

SystemCallResult systemCall_Memory_MapKernelMemory(PID pid, MemoryPage basePage, u32 kernelMemoryLocation, u32 nPages, u32 readWrite, SystemCallPermission permission);

SystemCallResult systemCall_Capabilities_SetupCapabilities(MemoryPage baseAddress, u32 nPages);

SystemCallResult systemCall_Capabilities_DeleteCapabilities();

SystemCallResult systemCall_BiosCall(struct SystemCall_BiosCall_Variables * arguments, u32 readWriteActions, void * readBackToLocation, u32 readSize, void * writeFromLocation, u32 writeSize);

SystemCallResult systemCall_ProcessThread_CreateProcess(PID pid);

SystemCallResult systemCall_ProcessThread_CreateThread(PID pid, MemoryPage threadContext, void * entryPoint, void * stackLocation, SystemCallPermission permission);

SystemCallResult systemCall_ProcessThread_ChangeThreadExecution(PID pid, MemoryPage threadContext, u32 operation, SystemCallPermission permission);

SystemCallResult systemCall_ProcessThread_DestroyThread(PID pid, MemoryPage threadContext, MemoryPage stackFreePage, u32 stackFreeNPages, SystemCallPermission permission);

SystemCallResult systemCall_ProcessThread_DestroyProcess(PID pid, SystemCallPermission permission);

SystemCallResult systemCall_ProcessThread_ChangeProcessPermissions(PID pid, u32 processPermissionNumber, u32 newValue);

SystemCallResult systemCall_ProcessThread_YieldTimeSlice();

SystemCallResult systemCall_SharedMemory_Initialize(MemoryPage sharedMemoryInfoPage);

SystemCallResult systemCall_SharedMemory_CreateSegment(MemoryPage baseAddress, u32 nPages, MemoryPage segmentDescriptorPage, u32 segmentID);

SystemCallResult systemCall_SharedMemory_ModifySegmentPermissions(u32 segmentID, PID pid, u32 newPermissions);

SystemCallResult systemCall_SharedMemory_MapSegment(PID sourcePID, u32 segmentID, PID destinationPID, MemoryPage destinationBaseAddress, u32 nPages, SystemCallPermission permission);

SystemCallResult systemCall_SharedMemory_UnmapSegment(PID sourcePID, u32 segmentID, PID destinationPID);

SystemCallResult systemCall_SharedMemory_DestroySegment(u32 segmentID);

SystemCallResult systemCall_SharedMemory_DestroyAll();

SystemCallResult systemCall_Messaging_Initialize(MemoryPage initPage);

SystemCallResult systemCall_Messaging_SetMessageHandler(void * handlerFunction);

SystemCallResult systemCall_Messaging_MessageProcess(PID pid, MemoryPage dataPage);

void systemCall_Messaging_MessageReturn();

#endif
