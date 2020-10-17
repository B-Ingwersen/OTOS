#ifndef UTILITIES___DEFINITIONS_H
#define UTILITIES___DEFINITIONS_H

#include "OTOSCore/Definitions.h"
#include "OTOSCore/SharedMemory.h"
#include "OTOSCore/SystemCalls.h"
#include "OTOSCore/CStandardLibrary/stddef.h"

struct _Utilities_ExternalStructure {
    void * (*getMappedPages)(u32 nPages, bool32 readWrite);
    void (*returnMappedPages)(MemoryPage buffer, u32 nPages);
    void * (*getUnmappedPages)(u32 nPages);
    void (*returnUnmappedPages)(MemoryPage buffer, u32 nPages);

    void * (*malloc)(size_t size);
    void (*free)(void * ptr);
    
    SharedMemory_SegmentID (*sharedMemory_CreateSegment)(MemoryPage mappedBaseAddress, u32 nPages);
    u32 (*sharedMemory_GrantProcessPermission)(SharedMemory_SegmentID segment, PID pid, bool32 writeAccess);
    SystemCallResult (*systemCall_SharedMemory_MapSegment)(PID sourcePID, u32 segmentID, PID destinationPID, MemoryPage destinationBaseAddress, u32 nPages, SystemCallPermission permission);
    SystemCallResult (*systemCall_ProcessThread_ChangeThreadExecution)(PID pid, MemoryPage threadContext, u32 operation, SystemCallPermission permission);
} ExactBinaryStructure;

extern struct _Utilities_ExternalStructure _Utilities_External;

void utilities_initialize();
void utilities_load();

#endif
