#ifndef UTILITIES___QUEUES_H
#define UTILITIES___QUEUES_H

#include "Definitions.h"

struct Queues_IPCQueue {
    volatile bool32 readerExecuting;
    volatile u32 readerPointer;
    volatile u32 writerPointer;
    volatile bool32 outOfSpace;
} ExactBinaryStructure;

struct Queues_IPCQueue_Writer {
    u8 * dataLocation;
    u32 queueSize;
    u32 nBufferPages;

    u32 writerPointer;
    Synchronization_Spinlock lock;

    SharedMemory_SegmentID segment;
    PID readerPID;
    MemoryPage thread;
    u32 permission;

    struct Queues_IPCQueue * queue;
};

struct Queues_IPCQueue_Reader {
    u8 * dataLocation;
    u32 queueSize;
    u32 nBufferPages;

    u32 readerPointer;
    Synchronization_Spinlock lock;

    SharedMemory_SegmentID segment;
    PID writerPID;

    struct Queues_IPCQueue * queue;
};

struct Queues_IPCQueue_Writer * queues_IPCQueue_Create_External(u32 nBufferPages, struct _Utilities_ExternalStructure * _Utilities_External);

u32 queues_IPCQueue_AddReader_External(struct Queues_IPCQueue_Writer * queue, PID pid, MemoryPage thread, u32 permission, struct _Utilities_ExternalStructure * _Utilities_External);

struct Queues_IPCQueue_Reader * queues_IPCQueue_GetFromWriter_External(PID writerPID, SharedMemory_SegmentID segment, u32 nBufferPages, u32 queueOffset, struct _Utilities_ExternalStructure * _Utilities_External);

u32 queues_IPCQueue_WriteData_External(struct Queues_IPCQueue_Writer * queue, void * data, u32 size, struct _Utilities_ExternalStructure * _Utilities_External);

u32 queues_IPCQueue_WriteDataNoWakeup(struct Queues_IPCQueue_Writer * queue, void * data, u32 size);

u32 queues_IPCQueue_WriterWakeup_External(struct Queues_IPCQueue_Writer * queue, struct _Utilities_ExternalStructure * _Utilities_External);

u32 queues_IPCQueue_ReadData(struct Queues_IPCQueue_Reader * queue, void * data, u32 size);

u32 queues_IPCQueue_DeleteWriter(struct Queues_IPCQueue_Writer * queue);

struct Queues_IPCQueue_Writer * queues_IPCQueue_Create(u32 nBufferPages);

u32 queues_IPCQueue_AddReader(struct Queues_IPCQueue_Writer * queue, PID pid, MemoryPage thread, u32 permission);

struct Queues_IPCQueue_Reader * queues_IPCQueue_GetFromWriter(PID writerPID, SharedMemory_SegmentID segment, u32 nBufferPages, u32 queueOffset);

u32 queues_IPCQueue_WriteData(struct Queues_IPCQueue_Writer * queue, void * data, u32 size);

u32 queues_IPCQueue_WriterWakeup(struct Queues_IPCQueue_Writer * queue);

#endif
