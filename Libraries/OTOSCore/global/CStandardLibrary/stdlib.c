#include "CStandardLibrary/stdlib.h"
#include "CStandardLibrary/string.h"
#include "MemoryAllocation.h"

unsigned long int stdlib_RandomNextPointer = 1;

int rand() {
    return rand_NextValuePointer(&stdlib_RandomNextPointer);
}

void srand(unsigned int seed) {
    srand_NextValuePointer(seed, &stdlib_RandomNextPointer);
}

void * malloc(size_t size) {
    return memoryAllocation_Malloc(memoryAllocation_DefaultBufferAllocator, size);
}

void free(void * pointer) {
    memoryAllocation_Free(memoryAllocation_DefaultBufferAllocator, pointer);
}

void * calloc(size_t nItems, size_t size) {
    void * buffer = malloc(nItems * size);
    if (buffer != NULL) {
        memset(buffer, 0, nItems * size);
    }
    return buffer;
}

void * realloc(void * pointer, size_t size) {
    if (pointer != NULL) {
        free(pointer);
    }
    return malloc(size);
}
