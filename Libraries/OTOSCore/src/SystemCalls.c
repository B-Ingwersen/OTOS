#include "SystemCalls.h"

#define MEMORY_SYSTEM_CALL_TYPE_MAP_MEMORY 0x0
#define MEMORY_SYSTEM_CALL_TYPE_UNMAP_MEMORY 0x1
#define MEMORY_SYSTEM_CALL_TYPE_MOVE_MEMORY 0x2
#define MEMORY_SYSTEM_CALL_TYPE_MAP_KERNEL_MEMORY 0x3

SystemCallResult systemCall_Memory_MapNewMemory(PID pid, MemoryPage basePage, u32 nPages, u32 readWrite, SystemCallPermission permission) {
    SystemCallResult result;
    asm volatile (
        "mov eax, %2    \n\t"\
        "push eax       \n\t"\
        "mov eax, %1    \n\t"\
        "int 0x40       \n\t"\
        "add esp, 4"
        : "=a" (result)
        : "g" (MEMORY_SYSTEM_CALL_TYPE_MAP_MEMORY), "g" (permission), "b" ((u32)basePage | (readWrite & 1)), "c" (nPages), "d" (pid)
        : "cc", "memory"
    );
    return result;
}
SystemCallResult systemCall_Memory_UnmapMemory(PID pid, MemoryPage basePage, u32 nPages, u32 allowKernelMemory, SystemCallPermission permission) {
    SystemCallResult result;
    asm volatile (
        "mov eax, %2    \n\t"\
        "push eax       \n\t"\
        "mov eax, %1    \n\t"\
        "int 0x40       \n\t"\
        "add esp, 4"
        : "=a" (result)
        : "g" (MEMORY_SYSTEM_CALL_TYPE_UNMAP_MEMORY), "g" (0), "b" ((u32)basePage | (allowKernelMemory & 1)), "c" (nPages), "d" (pid)
        : "cc", "memory"
    );
    return result;
}
SystemCallResult systemCall_Memory_MoveMemory(PID sourcePID, PID destinationPID, MemoryPage sourceBase, MemoryPage destinationBase, u32 nPages, u32 readWrite, SystemCallPermission sourcePermission, SystemCallPermission destinationPermission) {
    SystemCallResult result;
    asm volatile (
        "mov eax, %2    \n\t"\
        "push eax       \n\t"\
        "mov eax, %3    \n\t"\
        "push eax       \n\t"\
        "mov eax, %1    \n\t"\
        "int 0x40       \n\t"\
        "add esp, 8"
        : "=a" (result)
        : "g" (MEMORY_SYSTEM_CALL_TYPE_MOVE_MEMORY), "g" (sourcePermission), "g" (destinationPermission), "b" ((u32)destinationBase | (readWrite & 1)), "c" (nPages), "d" (destinationPID), "D" (sourceBase), "S" (sourcePID)
        : "cc", "memory"
    );
    return result; 
}
SystemCallResult systemCall_Memory_MapKernelMemory(PID pid, MemoryPage basePage, u32 kernelMemoryLocation, u32 nPages, u32 readWrite, SystemCallPermission permission) {
    SystemCallResult result;
    asm volatile (
        "mov eax, %2    \n\t"\
        "push eax       \n\t"\
        "mov eax, %1    \n\t"\
        "int 0x40       \n\t"\
        "add esp, 4"
        : "=a" (result)
        : "g" (MEMORY_SYSTEM_CALL_TYPE_MAP_KERNEL_MEMORY), "g" (permission), "b" ((u32)basePage | (readWrite & 1)), "c" (nPages), "d" (pid), "D" (kernelMemoryLocation)
        : "cc", "memory"
    );
    return result;
}

#define SYSTEM_CALL_CAPABILITIES_SETUP 0
#define SYSTEM_CALL_CAPABILITIES_DELETE 1

SystemCallResult systemCall_Capabilities_SetupCapabilities(MemoryPage baseAddress, u32 nPages) {
    SystemCallResult result;
    asm volatile (
        "int 0x42"
        : "=a" (result)
        : "b" (baseAddress), "c" (nPages), "a" (SYSTEM_CALL_CAPABILITIES_SETUP)
        : "cc", "memory"
    );
    return result;
}
SystemCallResult systemCall_Capabilities_DeleteCapabilities() {
    SystemCallResult result;
    asm volatile (
        "int 0x42"
        : "=a" (result)
        : "a" (SYSTEM_CALL_CAPABILITIES_DELETE)
        : "cc", "memory"
    );
    return result;
}

SystemCallResult systemCall_BiosCall(struct SystemCall_BiosCall_Variables * arguments, u32 readWriteActions, void * readBackToLocation, u32 readSize, void * writeFromLocation, u32 writeSize) {
    SystemCallResult result;
    asm volatile (
        "int 0x45"
        : "=a" (result)
        : "a" (readWriteActions), "b" (arguments), "D" (readBackToLocation), "S" (writeFromLocation), "c" (readSize), "d" (writeSize)
        : "cc", "memory"
    );
    return result;
}

#define SYSTEM_CALL_PROCESS_THREAD_CREATE_THREAD 0
#define SYSTEM_CALL_PROCESS_THREAD_DESTROY_THREAD 1
#define SYSTEM_CALL_PROCESS_THREAD_CHANGE_THREAD_EXECUTION 2
#define SYSTEM_CALL_PROCESS_THREAD_CREATE_PROCESS 3
#define SYSTEM_CALL_PROCESS_THREAD_DESTROY_PROCESS 4
#define SYSTEM_CALL_PROCESS_THREAD_CHANGE_PROCESS_PERMISSION 5
SystemCallResult systemCall_ProcessThread_CreateProcess(PID pid) {
    SystemCallResult result;
    asm volatile (
        "int 0x41"
        : "=a" (result)
        : "a" (SYSTEM_CALL_PROCESS_THREAD_CREATE_PROCESS), "d" (pid)
        : "cc", "memory"
    );
    return result;
}
SystemCallResult systemCall_ProcessThread_CreateThread(PID pid, MemoryPage threadContext, void * entryPoint, void * stackLocation, SystemCallPermission permission) {
    SystemCallResult result;
    asm volatile (
        "push %1    \n\t"\
        "int 0x41   \n\t"\
        "add esp, 4"
        : "=a" (result)
        : "g" (permission), "a" (SYSTEM_CALL_PROCESS_THREAD_CREATE_THREAD), "d" (pid), "c" (threadContext), "D" (entryPoint), "S" (stackLocation)
        : "cc", "memory"
    );
    return result;
}
SystemCallResult systemCall_ProcessThread_ChangeThreadExecution(PID pid, MemoryPage threadContext, u32 operation, SystemCallPermission permission) {
    SystemCallResult result;
    asm volatile (
        "push %1    \n\t"\
        "int 0x41   \n\t"\
        "add esp, 4"
        : "=a" (result)
        : "g" (permission), "a" (SYSTEM_CALL_PROCESS_THREAD_CHANGE_THREAD_EXECUTION), "d" (pid), "b" (threadContext), "c" (operation)
        : "cc", "memory"
    );
    return result;
}
SystemCallResult systemCall_ProcessThread_DestroyThread(PID pid, MemoryPage threadContext, MemoryPage stackFreePage, u32 stackFreeNPages, SystemCallPermission permission) {
    SystemCallResult result;
    asm volatile (
        "push %1    \n\t"\
        "int 0x41   \n\t"\
        "add esp, 4"
        : "=a" (result)
        : "g" (permission), "a" (SYSTEM_CALL_PROCESS_THREAD_DESTROY_THREAD), "d" (pid), "b" (threadContext), "c" (stackFreeNPages), "S" (stackFreePage)
        : "cc", "memory"
    );
    return result;
}
SystemCallResult systemCall_ProcessThread_DestroyProcess(PID pid, SystemCallPermission permission) {
    SystemCallResult result;
    asm volatile (
        "push %1    \n\t"\
        "int 0x41   \n\t"\
        "add esp, 4"
        : "=a" (result)
        : "g" (permission), "a" (SYSTEM_CALL_PROCESS_THREAD_DESTROY_PROCESS), "d" (pid)
        : "cc", "memory"
    );
    return result;
}
SystemCallResult systemCall_ProcessThread_ChangeProcessPermissions(PID pid, u32 processPermissionNumber, u32 newValue) {
    SystemCallResult result;
    asm volatile (
        "int 0x41   \n\t"\
        : "=a" (result)
        : "a" (SYSTEM_CALL_PROCESS_THREAD_CHANGE_PROCESS_PERMISSION), "d" (pid), "b" (processPermissionNumber), "c" (newValue)
        : "cc", "memory"
    );
    return result;
}
SystemCallResult systemCall_ProcessThread_YieldTimeSlice() {
    SystemCallResult result;
    asm volatile (
        "int 0x41"
        : "=a" (result)
        : "a" (SYSTEM_CALL_PROCESS_THREAD_CHANGE_THREAD_EXECUTION), "c" (SYSTEM_CALL_CHANGE_THREAD_EXECUTION_YIELD_TIME_SLICE)
        : "cc", "memory"
    );
    return result;
}

#define SYSTEM_CALL_SHARED_MEMORY_INITIALIZE 0
#define SYSTEM_CALL_SHARED_MEMORY_CREATE_SEGMENT 1
#define SYSTEM_CALL_SHARED_MEMORY_MODIFY_SEGMENT_PERMISSIONS 2
#define SYSTEM_CALL_SHARED_MEMORY_MAP_SEGMENT 3
#define SYSTEM_CALL_SHARED_MEMORY_UNMAP_SEGMENT 4
#define SYSTEM_CALL_SHARED_MEMORY_DESTROY_SEGMENT 5
#define SYSTEM_CALL_SHARED_MEMORY_DESTROY_ALL 6

SystemCallResult systemCall_SharedMemory_Initialize(MemoryPage sharedMemoryInfoPage) {
    SystemCallResult result;
    asm volatile (
        "int 0x44"
        : "=a" (result)
        : "a" (SYSTEM_CALL_SHARED_MEMORY_INITIALIZE), "b" (sharedMemoryInfoPage)
        : "cc", "memory"
    );
    return result;
}
SystemCallResult systemCall_SharedMemory_CreateSegment(MemoryPage baseAddress, u32 nPages, MemoryPage segmentDescriptorPage, u32 segmentID) {
    SystemCallResult result;
    asm volatile (
        "int 0x44"
        : "=a" (result)
        : "a" (SYSTEM_CALL_SHARED_MEMORY_CREATE_SEGMENT), "b" (baseAddress), "c" (nPages), "d" (segmentDescriptorPage), "D" (segmentID)
        : "cc", "memory"
    );
    return result;
}
SystemCallResult systemCall_SharedMemory_ModifySegmentPermissions(u32 segmentID, PID pid, u32 newPermissions) {
    SystemCallResult result;
    asm volatile (
        "int 0x44"
        : "=a" (result)
        : "a" (SYSTEM_CALL_SHARED_MEMORY_MODIFY_SEGMENT_PERMISSIONS), "b" (newPermissions), "d" (pid), "D" (segmentID)
        : "cc", "memory"
    );
    return result;
}
SystemCallResult systemCall_SharedMemory_MapSegment(PID sourcePID, u32 segmentID, PID destinationPID, MemoryPage destinationBaseAddress, u32 nPages, SystemCallPermission permission) {
    SystemCallResult result;
    asm volatile (
        "push %1    \n\t"\
        "int 0x44   \n\t"\
        "add esp, 4"
        : "=a" (result)
        : "g" (permission), "a" (SYSTEM_CALL_SHARED_MEMORY_MAP_SEGMENT), "b" (destinationBaseAddress), "c" (nPages), "d" (destinationPID), "D" (segmentID), "S" (sourcePID)
        : "cc", "memory"
    );
    return result;
}
SystemCallResult systemCall_SharedMemory_UnmapSegment(PID sourcePID, u32 segmentID, PID destinationPID) {
    SystemCallResult result;
    asm volatile (
        "int 0x44"
        : "=a" (result)
        : "a" (SYSTEM_CALL_SHARED_MEMORY_UNMAP_SEGMENT), "d" (destinationPID), "D" (segmentID), "S" (sourcePID)
        : "cc", "memory"
    );
    return result;
}
SystemCallResult systemCall_SharedMemory_DestroySegment(u32 segmentID) {
    SystemCallResult result;
    asm volatile (
        "int 0x44"
        : "=a" (result)
        : "a" (SYSTEM_CALL_SHARED_MEMORY_DESTROY_SEGMENT), "D" (segmentID)
        : "cc", "memory"
    );
    return result;
}
SystemCallResult systemCall_SharedMemory_DestroyAll() {
    SystemCallResult result;
    asm volatile (
        "int 0x44"
        : "=a" (result)
        : "a" (SYSTEM_CALL_SHARED_MEMORY_DESTROY_ALL)
        : "cc", "memory"
    );
    return result;
}

#define SYSTEM_CALL_MESSAGING_INITIALIZE 0
#define SYSTEM_CALL_MESSAGING_SET_MESSAGE_HANDLER 1
#define SYSTEM_CALL_MESSAGING_MESSAGE_PROCESS 2
#define SYSTEM_CALL_MESSAGING_MESSAGE_RETURN 3

SystemCallResult systemCall_Messaging_Initialize(MemoryPage initPage) {
    SystemCallResult result;
    asm volatile (
        "int 0x43"
        : "=a" (result)
        : "a" (SYSTEM_CALL_MESSAGING_INITIALIZE), "b" (initPage)
        : "cc", "memory"
    );
    return result;
}
SystemCallResult systemCall_Messaging_SetMessageHandler(void * handlerFunction) {
    SystemCallResult result;
    asm volatile (
        "int 0x43"
        : "=a" (result)
        : "a" (SYSTEM_CALL_MESSAGING_SET_MESSAGE_HANDLER), "b" (handlerFunction)
        : "cc", "memory"
    );
    return result;
}
SystemCallResult systemCall_Messaging_MessageProcess(PID pid, MemoryPage dataPage) {
    SystemCallResult result;
    asm volatile (
        "push ebx   \n\t"
        "push ecx   \n\t"
        "push edx   \n\t"
        "push esi   \n\t"
        "push edi   \n\t"
        "push ebp   \n\t"
        "pushf      \n\t"
        "int 0x43   \n\t"
        "popf       \n\t"
        "pop ebp    \n\t"
        "pop edi    \n\t"
        "pop esi    \n\t"
        "pop edx    \n\t"
        "pop ecx    \n\t"
        "pop ebx        "
        : "=a" (result)
        : "a" (SYSTEM_CALL_MESSAGING_MESSAGE_PROCESS), "b" (dataPage), "d" (pid)
        : "cc", "memory"
    );
    return result;
}
void systemCall_Messaging_MessageReturn() {
    asm volatile (
        "int 0x43"
        :: "a" (SYSTEM_CALL_MESSAGING_MESSAGE_RETURN)
        : "cc", "memory"
    );
}

