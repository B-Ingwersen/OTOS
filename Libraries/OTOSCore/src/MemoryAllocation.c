
#include "SystemCalls.h"
#include "MemoryAllocation.h"

// BLOCK OPERATIONS
    // get the minimum power of two which the requested size will fit in
    u32 memoryAllocation_GetBlockBinSize(u32 size) {
        u32 i;
        for (i = 0; i < 32; i++) {
            if (size & 0x80000000) {
                return 31 - i;
            }
            size <<= 1;
        }
        return 32;
    }

    // add a block to the bins of a block allocator
    void memoryAllocation_AddBlockToBins(struct MemoryAllocation_BlockAllocationDescriptor * descriptor, struct MemoryAllocation_BlockHeader * block) {

        IntegerPointer blockSize = ( (IntegerPointer)(block -> nextPhysicalBlock) & MEMORY_ALLOCATOR_BLOCK_REVERSE_FLAGS) - (IntegerPointer)block;
        //if block size is zero, cannot add to bins
        if (blockSize == 0) {
            return;
        }

        u32 binSize = memoryAllocation_GetBlockBinSize(blockSize);
        if (binSize >= descriptor -> maxBinSize) {
            return;
        }

        // when bin is empty add the block, circularly linked to itself
        if (descriptor -> bins[binSize] == NULL) {
            descriptor -> bins[binSize] = block;
            block -> nextBinBlock = block;
            block -> previousBinBlock = block;
        }
        // otherwise insert at the end of the circular linked list (the previous
        // block to the current head)
        else {
            // set the blocks references in the linked list
            block -> nextBinBlock = descriptor -> bins[binSize];
            block -> previousBinBlock = descriptor -> bins[binSize] -> previousBinBlock;

            // adjust the references of the blocks it was attached to
            block -> nextBinBlock -> previousBinBlock = block;
            block -> previousBinBlock -> nextBinBlock = block;
        }
    }

    // remove a block from its bin's linked list; the caller MUST GUARENTEE
    // that the given block is currently in the bin's linked list
    void memoryAllocation_RemoveBlockFromBins(struct MemoryAllocation_BlockAllocationDescriptor * descriptor, struct MemoryAllocation_BlockHeader * block) {
        // determine which bin the block came from
        IntegerPointer blockSize = ( (IntegerPointer)(block -> nextPhysicalBlock) & MEMORY_ALLOCATOR_BLOCK_REVERSE_FLAGS) - (IntegerPointer)block;
        u32 binSize = memoryAllocation_GetBlockBinSize(blockSize);
        
        // if the block was the last in its bin, NULL the bin pointer so that
        // the bin is marked as empty
        if (block -> nextBinBlock == block) {
            descriptor -> bins[binSize] = NULL;
        }
        else {
            // if the block was the head of hte linked list, replace the head
            // with the next block in the bin
            if (block == descriptor -> bins[binSize]) {
                descriptor -> bins[binSize] = block -> nextBinBlock;
            }

            // link the preceding and following blocks together to cut the 
            // removed block out of the linked list
            block -> nextBinBlock -> previousBinBlock = block -> previousBinBlock;
            block -> previousBinBlock -> nextBinBlock = block -> nextBinBlock;
        }
    }

    // get a block of given size from a block allocator; return NULL if the
    // operation cannot be performed; the size must be a multiple of 16
    struct MemoryAllocation_BlockHeader * memoryAllocation_GetBlock(struct MemoryAllocation_BlockAllocationDescriptor * descriptor, u32 size) {
        // error for zero sized blocks
        if (size == 0) {
            return NULL;
        }
        
        // determine the needed block size and corresponding bin
            // round up the size to a multiple of 16
            if ( (size & 0xF) != 0) {
                size &= 0xFFFFFFF0;
                size += 0x10;
            }

            // get the minimum bin size, and move up a bin if exactly the right
            // size
            u32 blockBin = memoryAllocation_GetBlockBinSize(size);
            if ( (size & (0xFFFFFFFF >> (32 - blockBin))) != 0 ) {
                blockBin++;
            }

            // find a bin where a block of the requisite size is available
            u32 availbleBin;
            for (availbleBin = blockBin; availbleBin < descriptor -> maxBinSize; ++availbleBin) {
                if (descriptor -> bins[availbleBin] != NULL) {
                    break;
                }
            }

        // retreieve the block from the bins
            // if there is no suitable block size, seek more allocation space
            struct MemoryAllocation_BlockHeader * block;
            if (availbleBin >= descriptor -> maxBinSize) {
                block = descriptor -> getNewAllocationSpace(descriptor -> data);

                // return an error if getting new space fails
                if (block == NULL) {
                    return NULL;
                }
            }

            // otherwise, grab the block from the block allocator bins
            else {
                block = descriptor -> bins[availbleBin];
                memoryAllocation_RemoveBlockFromBins(descriptor, block);
            }

        // try to cut off the remaining space from the block and give it back
        // to the block allocator

            // calculate the unused size at the end of the block
            IntegerPointer nextPhysicalBlockNoFlags = (IntegerPointer)(block -> nextPhysicalBlock) & MEMORY_ALLOCATOR_BLOCK_REVERSE_FLAGS;
            u32 blockSize = nextPhysicalBlockNoFlags - (IntegerPointer)block;
            u32 remainingBlockSize = blockSize - size;
            
            // give back the end of the block if it is large enough to contain
            // a block header
            if (remainingBlockSize >= 0x10) {
                struct MemoryAllocation_BlockHeader * remainingBlock = (struct MemoryAllocation_BlockHeader *)( (IntegerPointer)block + size );
                
                // initialize the new block
                remainingBlock -> nextPhysicalBlock = block -> nextPhysicalBlock;
                remainingBlock -> previousPhysicalBlock = block;

                // adjust the sorrounding block's pointers
                if ( ( ((IntegerPointer)(block -> nextPhysicalBlock)) & MEMORY_ALLOCATOR_BLOCK_FLAG_END_OF_SEGMENT ) == 0 ) {
                    IntegerPointer nextBlock = ((IntegerPointer)(block -> nextPhysicalBlock)) & MEMORY_ALLOCATOR_BLOCK_REVERSE_FLAGS;
                    ((struct MemoryAllocation_BlockHeader *)nextBlock) -> previousPhysicalBlock = remainingBlock;
                }
                block -> nextPhysicalBlock = remainingBlock;

                // give the remaining block back to the block allocator
                memoryAllocation_AddBlockToBins(descriptor, remainingBlock);
            }

        // add the used flag to the block
        block -> nextPhysicalBlock = (struct MemoryAllocation_BlockHeader *) ( (IntegerPointer)(block -> nextPhysicalBlock) | MEMORY_ALLOCATOR_BLOCK_FLAG_USED );

        return block;
    }

    // return a block to a block allocator
    void memoryAllocation_ReturnBlock(struct MemoryAllocation_BlockAllocationDescriptor * descriptor, struct MemoryAllocation_BlockHeader * block) {

        if (block == NULL) {asm volatile("xor eax, eax\n\tmov ebx, 0x12345\n\tdiv edx");}

        IntegerPointer flags = (IntegerPointer)(block -> nextPhysicalBlock) & MEMORY_ALLOCATOR_BLOCK_FLAGS;
        IntegerPointer nextPhysicalBlockNoFlags = (IntegerPointer)(block -> nextPhysicalBlock) & MEMORY_ALLOCATOR_BLOCK_REVERSE_FLAGS;
        IntegerPointer nextBlock = (IntegerPointer)(block -> nextPhysicalBlock);

        // try merging the block with the following block if there is one
        if ( (flags & MEMORY_ALLOCATOR_BLOCK_FLAG_END_OF_SEGMENT) == 0) {
            if (nextPhysicalBlockNoFlags == 0) {asm volatile("xor eax, eax\n\tmov ebx, 0x12347\n\tdiv edx");}

            // if the following segment is NOT used, perform the merge
            IntegerPointer nextNextBlock = (IntegerPointer) ( ((struct MemoryAllocation_BlockHeader *)(nextPhysicalBlockNoFlags)) -> nextPhysicalBlock );
            if ( (nextNextBlock & MEMORY_ALLOCATOR_BLOCK_FLAG_USED) == 0) {

                // change the new nextBlock location (which steals nextBlock's flags as well)
                nextBlock = nextNextBlock;
                memoryAllocation_RemoveBlockFromBins(descriptor, (struct MemoryAllocation_BlockHeader *)nextPhysicalBlockNoFlags);
            }
        }

        // try merging the block with the preceding block
        if (block != block -> previousPhysicalBlock) { // check that block is not the start of the segment

            // merge if the previous block is unused
            IntegerPointer previousBlockFlags = (IntegerPointer) ( block -> previousPhysicalBlock -> nextPhysicalBlock ) & MEMORY_ALLOCATOR_BLOCK_FLAGS;
            if ( (previousBlockFlags & MEMORY_ALLOCATOR_BLOCK_FLAG_USED) == 0) {

                memoryAllocation_RemoveBlockFromBins(descriptor, block -> previousPhysicalBlock);
                block = block -> previousPhysicalBlock;
            }
        }

        // update the next block pointer and mark as unused
        block -> nextPhysicalBlock = (struct MemoryAllocation_BlockHeader *)(nextBlock & MEMORY_ALLOCATOR_BLOCK_REVERSE_FLAG_USED);

        // update next block references if the block is not the end of the segment
        if (!((IntegerPointer)(block -> nextPhysicalBlock) & MEMORY_ALLOCATOR_BLOCK_FLAG_END_OF_SEGMENT)) {
            struct MemoryAllocation_BlockHeader * nextPhysicalBlock = (struct MemoryAllocation_BlockHeader *)(nextBlock & MEMORY_ALLOCATOR_BLOCK_REVERSE_FLAGS);
            nextPhysicalBlock -> previousPhysicalBlock = block;
        }

        // return the allocation space if the whole segement is unused
        if (block == block -> previousPhysicalBlock && ((IntegerPointer)(block -> nextPhysicalBlock) & MEMORY_ALLOCATOR_BLOCK_FLAG_END_OF_SEGMENT) ) {
            descriptor -> returnAllocationSpace(descriptor -> data, block);
        }
        else {
            memoryAllocation_AddBlockToBins(descriptor, block);
        }
    }
// BLOCK OPERATIONS

// BUFFER ALLOCATOR
    // no extra allocation space can be retrieved for tier 3 blocks, so it
    // always fails (return NULL)
    struct MemoryAllocation_BlockHeader * memoryAllocation_Tier3_GetNewAllocationSpace(void * data) {
        return NULL;
    }

    // return allocation space handler for tier 3 block allocators
    void memoryAllocation_Tier3_ReturnAllocationSpace(void * data, struct MemoryAllocation_BlockHeader * block) {
        struct MemoryAllocation_BufferAllocator * allocator = (struct MemoryAllocation_BufferAllocator *)data;
        memoryAllocation_AddBlockToBins(&(allocator -> tier3Allocator), block);
    }

    // get a tier 3 buffer of a certain size in tier 3 units (4MB chunks);
    // return NULL on failure
    void * memoryAllocation_Tier3_GetBuffer(struct MemoryAllocation_BufferAllocator * allocator, u32 size) {   // size in tier 3 units
        // try to get a block from the tier 3 allocator, chceking for failure
        IntegerPointer blockAddress = (IntegerPointer)memoryAllocation_GetBlock(&(allocator -> tier3Allocator), size * sizeof(struct MemoryAllocation_BlockHeader));
        if (blockAddress == (IntegerPointer)NULL) {
            return NULL;
        }

        // get the tier3 managed address corresponding to the block returned
        IntegerPointer address = allocator -> allocationSpaceBase;
        address += ( blockAddress - (IntegerPointer)(allocator -> tier3RootBlock) ) * (MEMORY_ALLOCATION_TIER_3_UNIT_SIZE / sizeof(struct MemoryAllocation_BlockHeader) );
        return (void*)address;
    }

    // return a tier3 buffer to a buffer allocator
    void memoryAllocation_Tier3_ReturnBuffer(struct MemoryAllocation_BufferAllocator * allocator, void * buffer) {
        // convert the buffer address to a block in the tier3RootBlock
        IntegerPointer offset = ((IntegerPointer)buffer - allocator -> allocationSpaceBase) / ( MEMORY_ALLOCATION_TIER_3_UNIT_SIZE / sizeof(struct MemoryAllocation_BlockHeader) );
        struct MemoryAllocation_BlockHeader * block = (struct MemoryAllocation_BlockHeader *)( (IntegerPointer)(allocator -> tier3RootBlock) + offset );

        // return the block to the block allocator
        memoryAllocation_ReturnBlock(&(allocator -> tier3Allocator), block);
    }

    // new allocation space handler for a tier 2 block allocator
    struct MemoryAllocation_BlockHeader * memoryAllocation_Tier2_GetNewAllocationSpace(void * data) {
        
        // retrieve a block from the tier 3 allocator
        struct MemoryAllocation_BufferAllocator * allocator = (struct MemoryAllocation_BufferAllocator *)data;
        IntegerPointer tier3Block = (IntegerPointer)memoryAllocation_Tier3_GetBuffer(allocator, 1);

        // error if the block doesn't exist
        if ((void*)tier3Block == NULL) {
            asm volatile("mov eax, 0xE021\n\tjmp $");
            return NULL;
        }

        // map the bottom 4 pages of the segement which will be used to keep
        // track of the tier 2 blocks; abort if thre is an error
        u32 result = systemCall_Memory_MapNewMemory(0, (void*)tier3Block, 4, true, SYSTEM_CALL_NO_PERMISSION);
        if (result == GENERIC_ERROR_RESULT) {
            asm volatile("mov eax, 0xE020\n\tjmp $");
            return NULL;
        }

        // calcualte the start and end locations of the block to use for the
        // allocation space
        struct MemoryAllocation_BlockHeader * rootBlock = (struct MemoryAllocation_BlockHeader *)(tier3Block + 4 * sizeof(struct MemoryAllocation_BlockHeader));
        struct MemoryAllocation_BlockHeader * endBlock = (struct MemoryAllocation_BlockHeader *)(tier3Block + 1024 * sizeof(struct MemoryAllocation_BlockHeader) | MEMORY_ALLOCATOR_BLOCK_FLAG_END_OF_SEGMENT);

        // construct and return the root block which spans the segment
        rootBlock -> nextPhysicalBlock = endBlock;
        rootBlock -> previousPhysicalBlock = rootBlock;
        return rootBlock;
    }

    // return allocation space handler for tier 2 allocators
    void memoryAllocation_Tier2_ReturnAllocationSpace(void * data, struct MemoryAllocation_BlockHeader * block) {
        struct MemoryAllocation_BufferAllocator * allocator = (struct MemoryAllocation_BufferAllocator *)data;
        
        // retrieve the tier 3 block that the tier 2 allocator is housed in,
        // free the 4 pages at the bottom, and return the buffer to tier 3
        IntegerPointer tier3Block = (IntegerPointer)block - 4 * sizeof(struct MemoryAllocation_BlockHeader);
        systemCall_Memory_UnmapMemory(0, (void*)tier3Block, 4, false, SYSTEM_CALL_NO_PERMISSION);
        memoryAllocation_Tier3_ReturnBuffer(allocator, (void*)tier3Block);
    }

    // get a buffer by its size in tier 2 unites (4k chunks)
    void * memoryAllocation_Tier2_GetBuffer(struct MemoryAllocation_BufferAllocator * allocator, u32 size) {   // size in tier 2 units

        // get a tier 2 block, checking that an error didn't occur
        IntegerPointer blockAddress = (IntegerPointer)memoryAllocation_GetBlock(&(allocator -> tier2Allocator), size * sizeof(struct MemoryAllocation_BlockHeader));
        if (blockAddress == (IntegerPointer)NULL) {
            return NULL;
        }

        // find the tier 3 block that the tier 2 block is housed in
        IntegerPointer tier3BlockNumber = (blockAddress - allocator -> allocationSpaceBase) / MEMORY_ALLOCATION_TIER_3_UNIT_SIZE;
        IntegerPointer tier3Block = allocator -> allocationSpaceBase + tier3BlockNumber * MEMORY_ALLOCATION_TIER_3_UNIT_SIZE;

        // find and return the corresponding buffer location in the tier 3 block
        IntegerPointer buffer = tier3Block + (blockAddress - tier3Block) * (MEMORY_ALLOCATION_TIER_2_UNIT_SIZE / sizeof(struct MemoryAllocation_BlockHeader));
        return (void*)buffer;
    }

    // return a tier 2 buffer
    void memoryAllocation_Tier2_ReturnBuffer(struct MemoryAllocation_BufferAllocator * allocator, void * buffer) {
        
        // determine the tier 3 block that the buffer is contianied in
        IntegerPointer tier3BlockNumber = ((IntegerPointer)buffer - allocator -> allocationSpaceBase) / MEMORY_ALLOCATION_TIER_3_UNIT_SIZE;
        IntegerPointer tier3Block = allocator -> allocationSpaceBase + tier3BlockNumber * MEMORY_ALLOCATION_TIER_3_UNIT_SIZE;

        // find and return the tier 2 block to the tier 2 block allocator
        IntegerPointer block = tier3Block + ((IntegerPointer)buffer - tier3Block) / (MEMORY_ALLOCATION_TIER_2_UNIT_SIZE / sizeof(struct MemoryAllocation_BlockHeader) );
        memoryAllocation_ReturnBlock(&(allocator -> tier2Allocator), (struct MemoryAllocation_BlockHeader *)block);
    }

    // get new allocation space handler for tier 1 blocks, returning NULL if an
    // error is encountered
    struct MemoryAllocation_BlockHeader * memoryAllocation_Tier1_GetNewAllocationSpace(void * data) {

        // get a block from the tier 2 allocator
        struct MemoryAllocation_BufferAllocator * allocator = (struct MemoryAllocation_BufferAllocator *)data;
        struct MemoryAllocation_BlockHeader * block = (struct MemoryAllocation_BlockHeader *)memoryAllocation_Tier2_GetBuffer(allocator, 1);

        // check if the block is valid
        if ((void*)block == NULL) {
            asm volatile("mov eax, 0xE022\n\tjmp $");
            return NULL;
        }

        // map the memory page, returning NULL if there is an error
        u32 result = systemCall_Memory_MapNewMemory(0, (void*)block, 1, true, SYSTEM_CALL_NO_PERMISSION);
        if (result == GENERIC_ERROR_RESULT) {
            asm volatile("mov eax, 0xE023\n\tjmp $");
            return NULL;
        }

        // initialize and return the root block
        block -> nextPhysicalBlock = (struct MemoryAllocation_BlockHeader *)( ((IntegerPointer)block + MEMORY_ALLOCATION_TIER_2_UNIT_SIZE) | MEMORY_ALLOCATOR_BLOCK_FLAG_END_OF_SEGMENT);
        block -> previousPhysicalBlock = block;
        return block;
    }

    // return allocation space handler for tier 1 block allocators
    void memoryAllocation_Tier1_ReturnAllocationSpace(void * data, struct MemoryAllocation_BlockHeader * block) {

        // unmap the page and return the block to the tier 2 allocator
        struct MemoryAllocation_BufferAllocator * allocator = (struct MemoryAllocation_BufferAllocator *)data;
        systemCall_Memory_UnmapMemory(0, (void*)block, 1, false, SYSTEM_CALL_NO_PERMISSION);
        memoryAllocation_Tier2_ReturnBuffer(allocator, (void*)block);
    }

    // get a buffer from a tier 1 block by its size in bytes
    void * memoryAllocation_Tier1_GetBuffer(struct MemoryAllocation_BufferAllocator * allocator, u32 size) {   // size in bytes

        // get a tier 1 block, returning NULL upon error; note that enough space
        // is allocated to deal with the block header as well
        IntegerPointer blockAddress = (IntegerPointer)memoryAllocation_GetBlock(&(allocator -> tier1Allocator), size + sizeof(struct MemoryAllocation_BlockHeader));
        if (blockAddress == (IntegerPointer)NULL) {
            return NULL;
        }

        // return the space after the block header
        IntegerPointer buffer = blockAddress + sizeof(struct MemoryAllocation_BlockHeader);
        return (void*)buffer;
    }

    // return a tier 1 buffer to a buffer allocator
    void memoryAllocation_Tier1_ReturnBuffer(struct MemoryAllocation_BufferAllocator * allocator, void * buffer) {

        // get the block header under the block and retur it to the tier 1 block
        // allocator
        IntegerPointer block = (IntegerPointer)buffer - sizeof(struct MemoryAllocation_BlockHeader);
        memoryAllocation_ReturnBlock(&(allocator -> tier1Allocator), (struct MemoryAllocation_BlockHeader *)block);
    }
// BUFFER ALLOCATOR

// BASE LIBRARY FUNCTIONS

    // build a new buffer allocator between a base and max page, including
    // 4 consecutive mapped pages that are NOT between the base and max pages
    u32 memoryAllocation_CreateBufferAllocator(struct MemoryAllocation_BufferAllocator * allocator, MemoryPage tier3_4MappedPages, IntegerPointer basePage, IntegerPointer maxPage) {
        // adjust base and max pages
            // push the base page up to a bage boundary
            if ( (basePage & 0xFFF) != 0) {
                basePage &= 0xFFFFF000;
                basePage += 0x1000;
            }

            // error if the base page is above the max page
            if (maxPage < basePage) {
                return GENERIC_ERROR_RESULT;
            }

            // calculate the number of tier 1 blocks; error if there is not
            // enough space for at least 2 of them
            u32 nTier3Blocks = (maxPage - basePage) / MEMORY_ALLOCATION_TIER_3_UNIT_SIZE;
            if (nTier3Blocks < 2) {
                return GENERIC_ERROR_RESULT;
            }

            // calculate the new max page that is a tier3 unit size multiple of
            // the base page
            maxPage = basePage + MEMORY_ALLOCATION_TIER_3_UNIT_SIZE * nTier3Blocks;
            allocator -> allocationSpaceBase = basePage;
        
        // zero bins
            u32 i;
            for (i = 0; i < 16; i++) {
                allocator -> tier1Allocator.bins[i] = NULL;
                allocator -> tier2Allocator.bins[i] = NULL;
                allocator -> tier3Allocator.bins[i] = NULL;
            }

        // setup tier 3
            struct MemoryAllocation_BlockHeader * tier3RootBlock = (struct MemoryAllocation_BlockHeader *)tier3_4MappedPages;
            tier3RootBlock -> nextPhysicalBlock = (struct MemoryAllocation_BlockHeader *) ((IntegerPointer)tier3RootBlock + nTier3Blocks * sizeof(struct MemoryAllocation_BlockHeader));
            tier3RootBlock -> nextPhysicalBlock = (struct MemoryAllocation_BlockHeader *) ((IntegerPointer)(tier3RootBlock -> nextPhysicalBlock)|MEMORY_ALLOCATOR_BLOCK_FLAG_END_OF_SEGMENT);
            tier3RootBlock -> previousPhysicalBlock = tier3RootBlock;
            allocator -> tier3RootBlock = tier3RootBlock;

            allocator -> tier3Allocator.data = (void*)allocator;
            allocator -> tier3Allocator.getNewAllocationSpace = memoryAllocation_Tier3_GetNewAllocationSpace;
            allocator -> tier3Allocator.returnAllocationSpace = memoryAllocation_Tier3_ReturnAllocationSpace;
            allocator -> tier3Allocator.maxBinSize = 16;

            memoryAllocation_AddBlockToBins(&(allocator -> tier3Allocator), tier3RootBlock);
        // setup tier 2
            allocator -> tier2Allocator.data = (void*)allocator;
            allocator -> tier2Allocator.getNewAllocationSpace = memoryAllocation_Tier2_GetNewAllocationSpace;
            allocator -> tier2Allocator.returnAllocationSpace = memoryAllocation_Tier2_ReturnAllocationSpace;
            allocator -> tier2Allocator.maxBinSize = 16;
        
        // setup tier 1
            allocator -> tier1Allocator.data = (void*)allocator;
            allocator -> tier1Allocator.getNewAllocationSpace = memoryAllocation_Tier1_GetNewAllocationSpace;
            allocator -> tier1Allocator.returnAllocationSpace = memoryAllocation_Tier1_ReturnAllocationSpace;
            allocator -> tier1Allocator.maxBinSize = 12;
    }

    // get unmapped pages from the buffer allocator
    MemoryPage memoryAllocation_GetUnmappedPages(struct MemoryAllocation_BufferAllocator * allocator, u32 nPages) {
        // requests less than 1020 pages can be serviced from tier 2
        if (nPages <= 1020) {
            MemoryPage buffer = memoryAllocation_Tier2_GetBuffer(allocator, nPages);
            return buffer;
        }

        // larger requests are given to the tier 3 allocator
        else {
            
            // the size is rounded up the nearest tier 3 block size
            if ( (nPages & 0x3FF) != 0) {
                nPages &= 0xFFFFFC00;
                nPages += 0x400;
            }
            u32 nTier3Blocks = nPages / 1024;
            
            return memoryAllocation_Tier3_GetBuffer(allocator, nTier3Blocks);
        }
    }

    // return unmapped pages to a buffer allocator
    void memoryAllocation_ReturnUnmappedPages(struct MemoryAllocation_BufferAllocator * allocator, MemoryPage buffer, u32 nPages) {
        // buffers less than 1020 pages are returned to the tier 2 allocator,
        // larger ones are returned to tier 3

        if (nPages <= 1020) {
            memoryAllocation_Tier2_ReturnBuffer(allocator, buffer);
        }
        else {
            memoryAllocation_Tier3_ReturnBuffer(allocator, buffer);
        }
    }

    // get mapped pages from a buffer allocator
    MemoryPage memoryAllocation_GetMappedPages(struct MemoryAllocation_BufferAllocator * allocator, u32 nPages, bool32 writeAccess) {

        // get unmapped pages first, checking for errors
        MemoryPage buffer = memoryAllocation_GetUnmappedPages(allocator, nPages);
        if (buffer == NULL) {
            //asm volatile("mov eax, 0xE015\n\tjmp $");
            return NULL;
        }

        // try to map them, returning NULL upon failure
        u32 result = systemCall_Memory_MapNewMemory(0, buffer, nPages, writeAccess, SYSTEM_CALL_NO_PERMISSION);
        if (result == GENERIC_ERROR_RESULT) {
            //asm volatile("mov eax, 0xE016\n\tjmp $" :: "b" (nPages), "c" (buffer), "d"(*(u32*)0xFFC40FFC));
            memoryAllocation_ReturnUnmappedPages(allocator, buffer, nPages);
            return NULL;
        }

        return buffer;
    }

    // return mapped pages to a buffer allocator
    void memoryAllocation_ReturnMappedPages(struct MemoryAllocation_BufferAllocator * allocator, MemoryPage buffer, u32 nPages) {
        u32 result = systemCall_Memory_UnmapMemory(0, buffer, nPages, false, SYSTEM_CALL_NO_PERMISSION);
        if (result == GENERIC_ERROR_RESULT) {
            asm volatile("mov eax, 0xE012\n\tjmp $" :: "b" (nPages), "c" (buffer));
        }
        memoryAllocation_ReturnUnmappedPages(allocator, buffer, nPages);
    }

    // allocate a certain number of bytes of memory from a buffer allocator
    void * memoryAllocation_Malloc(struct MemoryAllocation_BufferAllocator * allocator, u32 size) {

        // bump up size to a multiple of 16
        if ( (size & 0xF) != 0) {
            size &= 0xFFFFFFF0;
            size += 0x10;
        }
        size += sizeof(struct MemoryAllocation_BlockHeader);

        // requests of less than 4080 bytes are handled by a block allocator
        if (size <= 4080) {
            struct MemoryAllocation_BlockHeader * block = memoryAllocation_GetBlock(&(allocator -> tier1Allocator), size);
            if (block == NULL) {
                asm volatile("mov eax, 0xE013\n\tjmp $");
                return NULL;
            }

            // save the block size in the previous bin entry
            block -> nextBinBlock = NULL;
            block -> previousBinBlock = (struct MemoryAllocation_BlockHeader *)(size);
            IntegerPointer buffer = (IntegerPointer)block + sizeof(struct MemoryAllocation_BlockHeader);
            return (void*)buffer;
        }
        else {
            // convert the size to a number of pages
            if ( (size & 0xFFF) != 0) {
                size &= 0xFFFFF000;
                size += 0x1000;
            }
            u32 nPages = size / MEMORY_ALLOCATION_TIER_2_UNIT_SIZE;

            struct MemoryAllocation_BlockHeader * block = memoryAllocation_GetMappedPages(allocator, nPages, true);
            if (block == NULL) {
                asm volatile("mov eax, 0xE014\n\tjmp $");
                return NULL;
            }

            // save the block size in pages in the next bin entry
            block -> nextBinBlock = (struct MemoryAllocation_BlockHeader *)1;
            block -> previousBinBlock = (struct MemoryAllocation_BlockHeader *)(nPages);
            IntegerPointer buffer = (IntegerPointer)block + sizeof(struct MemoryAllocation_BlockHeader);
            return (void*)buffer;
        }
    }

    void memoryAllocation_Free(struct MemoryAllocation_BufferAllocator * allocator, void * buffer) {
        /* CHANGE SIZEOF TO NON-POINTER TYPE! */
        struct MemoryAllocation_BlockHeader * block = (struct MemoryAllocation_BlockHeader *)((IntegerPointer)buffer - sizeof(struct MemoryAllocation_BlockHeader));
        if (block -> nextBinBlock == NULL) {
            memoryAllocation_ReturnBlock(&(allocator -> tier1Allocator), block);
        }
        else {
            IntegerPointer size = (IntegerPointer)(block -> previousBinBlock);
            memoryAllocation_ReturnMappedPages(allocator, (MemoryPage)block, size);
        }
    }
// BASE LIBRARY FUNCTIONS

