#ifndef PROCESS_MANAGEMENT___MESSAGING_DEFINITIONS_H
#define PROCESS_MANAGEMENT___MESSAGING_DEFINITIONS_H

#include "OTOSCore/Definitions.h"

#define PROCESS_MANAGER_MESSAGING_OPERATION_GET_PROCESS_PID 1

struct ProcessManager_Messaging_GetProcessPID {
    u32 operation;
    u32 result;

    PID pid;

    char name[256];
    u32 nameLength;
} ExactBinaryStructure;

#endif