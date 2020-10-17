
#ifndef IPC_H
#define IPC_H

#include "OTOSCore/Definitions.h"
#include "OTOSCore/SharedMemory.h"

#define MAX_NUMBER_OF_PROCESSES 512
#define MAX_BUFFER_PAGES 16

struct ProcessPermission {
    PID pid;
    struct Queues_IPCQueue_Writer * queue;
};

extern struct ProcessPermission * processPermissions;
extern PID currentForwardProcess;

void ipc_Initialize();

u32 ipc_AddProcess(PID pid, u32 nBufferPages, MemoryPage thread, SharedMemory_SegmentID * returnSegment, u32 * returnQueueOffset);

u32 forwardByte_WriteData(u8 byte);

u32 forwardByte_Wakeup();

#endif