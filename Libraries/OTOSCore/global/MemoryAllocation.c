#include "SystemCalls.h"
#include "MemoryAllocation.h"

struct MemoryAllocation_BufferAllocator * memoryAllocation_DefaultBufferAllocator = NULL;

u32 memoryAllocation_DefaultInitialization(IntegerPointer basePage, IntegerPointer maxPage, MemoryPage _5UnmappedPages) {
    systemCall_Memory_MapNewMemory(0, _5UnmappedPages, 5, true, SYSTEM_CALL_NO_PERMISSION);
    struct MemoryAllocation_BufferAllocator * allocator = (struct MemoryAllocation_BufferAllocator *)((IntegerPointer)_5UnmappedPages + 4 * PAGE_SIZE);
    memoryAllocation_DefaultBufferAllocator = allocator;
    return memoryAllocation_CreateBufferAllocator(allocator, _5UnmappedPages, basePage, maxPage);
}

MemoryPage getUnmappedPages(u32 nPages) {
    return memoryAllocation_GetUnmappedPages(memoryAllocation_DefaultBufferAllocator, nPages);
}

void returnUnmappedPages(MemoryPage buffer, u32 nPages) {
    memoryAllocation_ReturnUnmappedPages(memoryAllocation_DefaultBufferAllocator, buffer, nPages);
}

MemoryPage getMappedPages(u32 nPages, bool32 writeAccess) {
    return memoryAllocation_GetMappedPages(memoryAllocation_DefaultBufferAllocator, nPages, writeAccess);
}

void returnMappedPages(MemoryPage buffer, u32 nPages) {
    memoryAllocation_ReturnMappedPages(memoryAllocation_DefaultBufferAllocator, buffer, nPages);
}

MemoryPage getUnmappedPage() {
	return getUnmappedPages(1);
}

MemoryPage getMappedPage(bool32 writeAccess) {
	return getMappedPages(1, writeAccess);
}

void returnUnmappedPage(MemoryPage buffer) {
	returnUnmappedPages(buffer, 1);
}

void returnMappedPage(MemoryPage buffer) {
	returnMappedPages(buffer, 1);
}
