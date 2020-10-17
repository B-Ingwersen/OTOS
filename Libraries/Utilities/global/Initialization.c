#ifndef UTILITIES___INITIALIZATION_H
#define UTILITIES___INITIALIZATION_H

#include "Definitions.h"
#include "OTOSCore/CStandardLibrary/stdlib.h"

struct _Utilities_ExternalStructure _Utilities_External = {};

void utilities_initialize() {
    _Utilities_External.getMappedPages = getMappedPages;
    _Utilities_External.returnMappedPages = returnMappedPages;
    _Utilities_External.getUnmappedPages = getUnmappedPages;
    _Utilities_External.returnUnmappedPages = returnUnmappedPages;
    _Utilities_External.malloc = malloc;
    _Utilities_External.free = free;
    _Utilities_External.sharedMemory_CreateSegment = sharedMemory_CreateSegment;
    _Utilities_External.sharedMemory_GrantProcessPermission = sharedMemory_GrantProcessPermission;
    _Utilities_External.systemCall_SharedMemory_MapSegment = systemCall_SharedMemory_MapSegment;
    _Utilities_External.systemCall_ProcessThread_ChangeThreadExecution = systemCall_ProcessThread_ChangeThreadExecution;
}

#endif
