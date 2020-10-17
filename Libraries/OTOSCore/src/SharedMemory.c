#include "SharedMemory.h"
#include "SystemCalls.h"
#include "Synchronization.h"
#include "MemoryAllocation.h"

SharedMemory_SegmentID sharedMemory_CreateSegment_Spinlock_InfoPage_Allocator(MemoryPage mappedBaseAddress, u32 nPages, Synchronization_Spinlock * sharedMemory_Lock, MemoryPage infoPage, struct MemoryAllocation_BufferAllocator * allocator) {
    synchronization_OpenSpinlock(sharedMemory_Lock);

    MemoryPage * descriptorList = (MemoryPage *)infoPage;

    u32 i;
    SharedMemory_SegmentID segment = SHARED_MEMORY_ERROR_SEGMENT;
    for (i = 0; i < SHARED_MEMORY_MAX_SEGMENT; i++) {
        if (descriptorList[i] == NULL) {
            segment = i;
            break;
        }
    }

    if (segment != SHARED_MEMORY_ERROR_SEGMENT) {
        MemoryPage segmentDescriptorPage = memoryAllocation_GetUnmappedPages(allocator, 1);
        u32 result = systemCall_SharedMemory_CreateSegment(mappedBaseAddress, nPages, segmentDescriptorPage, segment);
        if (result == GENERIC_ERROR_RESULT) {
            segment = SHARED_MEMORY_ERROR_SEGMENT;
            memoryAllocation_ReturnUnmappedPages(allocator, segmentDescriptorPage, 1);
        }
    }

    synchronization_CloseSpinlock(sharedMemory_Lock);
    return segment;
}

SharedMemory_SegmentID sharedMemory_MapAndCreateSegment_Spinlock_InfoPage_Allocator(u32 nPages, MemoryPage * returnBuffer, Synchronization_Spinlock * lock, MemoryPage infoPage, struct MemoryAllocation_BufferAllocator * allocator) {
    MemoryPage buffer = memoryAllocation_GetMappedPages(allocator, nPages, true);// multiPageBuffers_GetMappedBuffer(nPages, true);
    if (buffer == NULL) {
        return SHARED_MEMORY_ERROR_SEGMENT;
    }

    SharedMemory_SegmentID id = sharedMemory_CreateSegment_Spinlock_InfoPage_Allocator(buffer, nPages, lock, infoPage, allocator);
    if (id == SHARED_MEMORY_ERROR_SEGMENT) {
        memoryAllocation_ReturnMappedPages(allocator, buffer, nPages);//multiPageBuffers_ReturnMappedBuffer(buffer, nPages);
        return id;
    }

    *returnBuffer = buffer;
    return id;
}

u32 sharedMemory_GrantProcessPermission_Spinlock(SharedMemory_SegmentID segment, PID pid, bool32 writeAccess, Synchronization_Spinlock * sharedMemory_Lock) {
    u32 result;

    synchronization_OpenSpinlock(sharedMemory_Lock);

    u32 access = SYSTEM_CALL_SHARED_MEMORY_PERMISSION_ACCESS;
    if (writeAccess) {
        access |= SYSTEM_CALL_SHARED_MEMORY_PERMISSION_WRITE;
    }
    result = systemCall_SharedMemory_ModifySegmentPermissions(segment, pid, access);

    synchronization_CloseSpinlock(sharedMemory_Lock);
    return result;
}

u32 sharedMemory_GetSegment(PID pid, SharedMemory_SegmentID segment, MemoryPage baseAddress, u32 nPages) {
    return systemCall_SharedMemory_MapSegment(pid, segment, 0, baseAddress, nPages, SYSTEM_CALL_NO_PERMISSION);
}

u32 sharedMemory_DestroySegment(SharedMemory_SegmentID segment) {
    //TODO: free the unmapped pages in memoryAllocation
    return systemCall_SharedMemory_DestroySegment(segment);
}
