#include "Queues.h"

struct Queues_IPCQueue_Writer * queues_IPCQueue_Create(u32 nBufferPages) {
    return queues_IPCQueue_Create_External(nBufferPages, &_Utilities_External);
}

u32 queues_IPCQueue_AddReader(struct Queues_IPCQueue_Writer * queue, PID pid, MemoryPage thread, u32 permission) {
    return queues_IPCQueue_AddReader_External(queue, pid, thread, permission, &_Utilities_External);
}

struct Queues_IPCQueue_Reader * queues_IPCQueue_GetFromWriter(PID writerPID, SharedMemory_SegmentID segment, u32 nBufferPages, u32 queueOffset) {
    return queues_IPCQueue_GetFromWriter_External(writerPID, segment, nBufferPages, queueOffset, &_Utilities_External);
}

u32 queues_IPCQueue_WriteData(struct Queues_IPCQueue_Writer * queue, void * data, u32 size) {
    return queues_IPCQueue_WriteData_External(queue, data, size, &_Utilities_External);
}

u32 queues_IPCQueue_WriterWakeup(struct Queues_IPCQueue_Writer * queue) {
    return queues_IPCQueue_WriterWakeup_External(queue, &_Utilities_External);
}
