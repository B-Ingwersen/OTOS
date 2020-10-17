#ifndef OTOS_CORE___MEMORY_ALLOCATION_H
#define OTOS_CORE___MEMORY_ALLOCATION_H

#define MEMORY_ALLOCATOR_BLOCK_FLAG_USED            0x1
#define MEMORY_ALLOCATOR_BLOCK_REVERSE_FLAG_USED    0xFFFFFFFE
#define MEMORY_ALLOCATOR_BLOCK_FLAG_END_OF_SEGMENT  0x2
#define MEMORY_ALLOCATOR_BLOCK_FLAGS                0xF
#define MEMORY_ALLOCATOR_BLOCK_REVERSE_FLAGS        0xFFFFFFF0

#define MEMORY_ALLOCATION_TIER_2_UNIT_SIZE PAGE_SIZE
#define MEMORY_ALLOCATION_TIER_3_UNIT_SIZE (MEMORY_ALLOCATION_TIER_2_UNIT_SIZE * 1024)

#include "Definitions.h"

struct MemoryAllocation_BlockHeader {
    // hold information about preceding and following blocks when the block is
    // being actively used
    struct MemoryAllocation_BlockHeader * nextPhysicalBlock;
    struct MemoryAllocation_BlockHeader * previousPhysicalBlock;

    // hold information for when blocks are in a linked list of available but
    // currently unused blocks
    struct MemoryAllocation_BlockHeader * nextBinBlock;
    struct MemoryAllocation_BlockHeader * previousBinBlock;
} ExactBinaryStructure;

// allocates blocks in a specific size range
struct MemoryAllocation_BlockAllocationDescriptor {
    struct MemoryAllocation_BlockHeader * bins[16];     // 16 bytes to 2048 bytes
    struct MemoryAllocation_BlockHeader * (* getNewAllocationSpace)(void * data);
    void (* returnAllocationSpace)(void * data, struct MemoryAllocation_BlockHeader * allocation);
    void * data;
    u32 maxBinSize;
} ExactBinaryStructure;

// allocates buffers at any size
struct MemoryAllocation_BufferAllocator {
    struct MemoryAllocation_BlockAllocationDescriptor tier1Allocator;
    struct MemoryAllocation_BlockAllocationDescriptor tier2Allocator;
    struct MemoryAllocation_BlockAllocationDescriptor tier3Allocator;

    struct MemoryAllocation_BlockHeader * tier3RootBlock;
    IntegerPointer allocationSpaceBase;
} ExactBinaryStructure;

u32 memoryAllocation_GetBlockBinSize(u32 size);

void memoryAllocation_AddBlockToBins(struct MemoryAllocation_BlockAllocationDescriptor * descriptor, struct MemoryAllocation_BlockHeader * block);

void memoryAllocation_RemoveBlockFromBins(struct MemoryAllocation_BlockAllocationDescriptor * descriptor, struct MemoryAllocation_BlockHeader * block);

struct MemoryAllocation_BlockHeader * memoryAllocation_GetBlock(struct MemoryAllocation_BlockAllocationDescriptor * descriptor, u32 size);

void memoryAllocation_ReturnBlock(struct MemoryAllocation_BlockAllocationDescriptor * descriptor, struct MemoryAllocation_BlockHeader * block);

struct MemoryAllocation_BlockHeader * memoryAllocation_Tier3_GetNewAllocationSpace(void * data);

void memoryAllocation_Tier3_ReturnAllocationSpace(void * data, struct MemoryAllocation_BlockHeader * block);

void * memoryAllocation_Tier3_GetBuffer(struct MemoryAllocation_BufferAllocator * allocator, u32 size);

void memoryAllocation_Tier3_ReturnBuffer(struct MemoryAllocation_BufferAllocator * allocator, void * buffer);

struct MemoryAllocation_BlockHeader * memoryAllocation_Tier2_GetNewAllocationSpace(void * data);

void memoryAllocation_Tier2_ReturnAllocationSpace(void * data, struct MemoryAllocation_BlockHeader * block);

void * memoryAllocation_Tier2_GetBuffer(struct MemoryAllocation_BufferAllocator * allocator, u32 size);

void memoryAllocation_Tier2_ReturnBuffer(struct MemoryAllocation_BufferAllocator * allocator, void * buffer);

struct MemoryAllocation_BlockHeader * memoryAllocation_Tier1_GetNewAllocationSpace(void * data);

void memoryAllocation_Tier1_ReturnAllocationSpace(void * data, struct MemoryAllocation_BlockHeader * block);

void * memoryAllocation_Tier1_GetBuffer(struct MemoryAllocation_BufferAllocator * allocator, u32 size);

void memoryAllocation_Tier1_ReturnBuffer(struct MemoryAllocation_BufferAllocator * allocator, void * buffer);

u32 memoryAllocation_CreateBufferAllocator(struct MemoryAllocation_BufferAllocator * allocator, MemoryPage tier3_4MappedPages, IntegerPointer basePage, IntegerPointer maxPage);

MemoryPage memoryAllocation_GetUnmappedPages(struct MemoryAllocation_BufferAllocator * allocator, u32 nPages);

void memoryAllocation_ReturnUnmappedPages(struct MemoryAllocation_BufferAllocator * allocator, MemoryPage buffer, u32 nPages);

MemoryPage memoryAllocation_GetMappedPages(struct MemoryAllocation_BufferAllocator * allocator, u32 nPages, bool32 writeAccess);

void memoryAllocation_ReturnMappedPages(struct MemoryAllocation_BufferAllocator * allocator, MemoryPage buffer, u32 nPages);

void * memoryAllocation_Malloc(struct MemoryAllocation_BufferAllocator * allocator, u32 size);

void memoryAllocation_Free(struct MemoryAllocation_BufferAllocator * allocator, void * buffer);

extern struct MemoryAllocation_BufferAllocator * memoryAllocation_DefaultBufferAllocator;

u32 memoryAllocation_DefaultInitialization(IntegerPointer basePage, IntegerPointer maxPage, MemoryPage _5UnmappedPages);

MemoryPage getUnmappedPages(u32 nPages);

void returnUnmappedPages(MemoryPage buffer, u32 nPages);

MemoryPage getMappedPages(u32 nPages, bool32 writeAccess);

void returnMappedPages(MemoryPage buffer, u32 nPages);

MemoryPage getUnmappedPage();

MemoryPage getMappedPage(bool32 writeAccess);

void returnUnmappedPage(MemoryPage buffer);

void returnMappedPage(MemoryPage buffer);

#endif
