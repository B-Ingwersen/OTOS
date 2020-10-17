#ifndef OTOS_CORE___THREADS_H
#define OTOS_CORE___THREADS_H

#include "Definitions.h"
#include "MemoryAllocation.h"

struct Threads_LocalStorageStructure {
    MemoryPage threadContext;
    MemoryPage messagingDataPage;
} ExactBinaryStructure;

void threads_SetupThread_2MappedPages(MemoryPage rwMappedPage1, MemoryPage rwMappedPage2);

struct Threads_LocalStorageStructure * threads_GetLocalStorage();

void threads_CleanupThread_Allocator(struct MemoryAllocation_BufferAllocator * allocator);

void threads_SetupThread();

void threads_CleanupThread();

#endif
