#ifndef OTOS_CORE___SHARED_MEMORY_H
#define OTOS_CORE___SHARED_MEMORY_H

#define SHARED_MEMORY_ERROR_SEGMENT 0xFFFF
#define SHARED_MEMORY_MAX_SEGMENT 1024

#include "Definitions.h"
#include "Synchronization.h"
#include "MemoryAllocation.h"

typedef u32 SharedMemory_SegmentID;

SharedMemory_SegmentID sharedMemory_CreateSegment_Spinlock_InfoPage_Allocator(MemoryPage mappedBaseAddress, u32 nPages, Synchronization_Spinlock * sharedMemory_Lock, MemoryPage infoPage, struct MemoryAllocation_BufferAllocator * allocator);

SharedMemory_SegmentID sharedMemory_MapAndCreateSegment_Spinlock_InfoPage_Allocator(u32 nPages, MemoryPage * returnBuffer, Synchronization_Spinlock * lock, MemoryPage infoPage, struct MemoryAllocation_BufferAllocator * allocator);

u32 sharedMemory_GrantProcessPermission_Spinlock(SharedMemory_SegmentID segment, PID pid, bool32 writeAccess, Synchronization_Spinlock * sharedMemory_Lock);

u32 sharedMemory_GetSegment(PID pid, SharedMemory_SegmentID segment, MemoryPage baseAddress, u32 nPages);

u32 sharedMemory_DestroySegment(SharedMemory_SegmentID segment);

extern Synchronization_Spinlock sharedMemory_Lock;
extern MemoryPage sharedMemory_InfoPage;

u32 sharedMemory_Initialize();

SharedMemory_SegmentID sharedMemory_MapAndCreateSegment(u32 nPages, MemoryPage * returnBuffer);

SharedMemory_SegmentID sharedMemory_CreateSegment(MemoryPage mappedBaseAddress, u32 nPages);

u32 sharedMemory_GrantProcessPermission(SharedMemory_SegmentID segment, PID pid, bool32 writeAccess);

#endif
