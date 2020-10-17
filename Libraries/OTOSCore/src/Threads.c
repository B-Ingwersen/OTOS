#include "Threads.h"
#include "SystemCalls.h"
#include "MemoryAllocation.h"

void threads_SetupThread_2MappedPages(MemoryPage rwMappedPage1, MemoryPage rwMappedPage2) {

    // get the current page of the stack
    IntegerPointer stackPointer; asm volatile("mov eax, esp":"=a" (stackPointer));
    stackPointer &= PAGING_ISOLATE_PAGE_ADDRESS_MASK;

    MemoryPage threadContext = (MemoryPage)(stackPointer + PAGE_SIZE);
    struct Threads_LocalStorageStructure * localStorage = (struct Threads_LocalStorageStructure *)rwMappedPage1;
    MemoryPage messagingDataPage = rwMappedPage2;

    *(struct Threads_LocalStorageStructure **)(stackPointer + PAGE_SIZE - sizeof(IntegerPointer)) = localStorage;

    systemCall_Memory_MapNewMemory(0, localStorage, 2, true, SYSTEM_CALL_NO_PERMISSION);
    localStorage -> threadContext = threadContext;
    localStorage -> messagingDataPage = messagingDataPage;
}

struct Threads_LocalStorageStructure * threads_GetLocalStorage() {
    IntegerPointer stackPointer; asm volatile("mov eax, esp":"=a" (stackPointer));
    stackPointer &= PAGING_ISOLATE_PAGE_ADDRESS_MASK;
    IntegerPointer * memoryMapSearch = (IntegerPointer*)( PAGING_RECURSIVE_MEMORY_MAP_LOCATION + (stackPointer / (PAGE_SIZE / sizeof(IntegerPointer)) ) );

    while (true) {
        memoryMapSearch++;
        if ( (*memoryMapSearch & PAGING_OS_FLAG_THREAD_CONTEXT) == PAGING_OS_FLAG_THREAD_CONTEXT) {
            IntegerPointer localStoragePointer = ((IntegerPointer)memoryMapSearch) * (PAGE_SIZE / sizeof(IntegerPointer)) - sizeof(IntegerPointer);
            return *(struct Threads_LocalStorageStructure **)localStoragePointer;
            break;
        }
    }
}

void threads_CleanupThread_Allocator(struct MemoryAllocation_BufferAllocator * allocator) {
    struct Threads_LocalStorageStructure * threadLocalStorage = threads_GetLocalStorage();

    memoryAllocation_ReturnMappedPages(allocator, threadLocalStorage -> messagingDataPage, 1);
    memoryAllocation_ReturnMappedPages(allocator, (MemoryPage)threadLocalStorage, 1);
}
