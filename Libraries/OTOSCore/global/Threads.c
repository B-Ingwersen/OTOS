#include "Threads.h"

void threads_SetupThread() {
    threads_SetupThread_2MappedPages(getMappedPage(true), getMappedPage(true));
}

void threads_CleanupThread() {
    threads_CleanupThread_Allocator(memoryAllocation_DefaultBufferAllocator);
}
