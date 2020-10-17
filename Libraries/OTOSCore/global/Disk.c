#include "Disk.h"

u32 disk_OpenFile(u8 * fileName, u32 fileNameLength, u32 bufferNPages, struct Disk_FileHandle * fileHandle) {
    return disk_OpenFile_Allocator(fileName, fileNameLength, bufferNPages, fileHandle, memoryAllocation_DefaultBufferAllocator);
}

u32 disk_CloseFile(struct Disk_FileHandle * fileHandle) {
    return disk_CloseFile_Allocator(fileHandle, memoryAllocation_DefaultBufferAllocator);
}

MemoryPage disk_ReadFullFileToBuffer(u8 * fileName, u32 fileNameLength, u32 maxBufferSize, u64 * fileSize, u32 * bufferNPages) {
    return disk_ReadFullFileToBuffer_Allocator(fileName, fileNameLength, maxBufferSize, fileSize, bufferNPages, memoryAllocation_DefaultBufferAllocator);
}
