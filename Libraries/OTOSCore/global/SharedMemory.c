#include "SystemCalls.h"
#include "SharedMemory.h"

Synchronization_Spinlock sharedMemory_Lock = 0;
MemoryPage sharedMemory_InfoPage = NULL;

u32 sharedMemory_Initialize() {
    u32 result;

    synchronization_InitializeSpinlock(&sharedMemory_Lock);
    synchronization_OpenSpinlock(&sharedMemory_Lock);

    sharedMemory_InfoPage = getUnmappedPage();
    result = systemCall_SharedMemory_Initialize(sharedMemory_InfoPage);
    if (result == GENERIC_ERROR_RESULT) {
        returnUnmappedPage(sharedMemory_InfoPage);
        return result;
    }

    synchronization_CloseSpinlock(&sharedMemory_Lock);
    return GENERIC_SUCCESS_RESULT;
}

SharedMemory_SegmentID sharedMemory_MapAndCreateSegment(u32 nPages, MemoryPage * returnBuffer) {
    return sharedMemory_MapAndCreateSegment_Spinlock_InfoPage_Allocator(nPages, returnBuffer, &sharedMemory_Lock, sharedMemory_InfoPage, memoryAllocation_DefaultBufferAllocator);
}

SharedMemory_SegmentID sharedMemory_CreateSegment(MemoryPage mappedBaseAddress, u32 nPages) {
    return sharedMemory_CreateSegment_Spinlock_InfoPage_Allocator(mappedBaseAddress, nPages, &sharedMemory_Lock, sharedMemory_InfoPage, memoryAllocation_DefaultBufferAllocator);
}

u32 sharedMemory_GrantProcessPermission(SharedMemory_SegmentID segment, PID pid, bool32 writeAccess) {
    return sharedMemory_GrantProcessPermission_Spinlock(segment, pid, writeAccess, &sharedMemory_Lock);
}
