#include "Queues.h"

struct Queues_IPCQueue_Writer * queues_IPCQueue_Create_External(u32 nBufferPages, struct _Utilities_ExternalStructure * _Utilities_External) {
    if (nBufferPages < 1) {
        return NULL;
    }

    struct Queues_IPCQueue_Writer * writerQueue = _Utilities_External -> malloc(sizeof(struct Queues_IPCQueue_Writer));
    if (writerQueue == NULL) {
        return NULL;
    }

    struct Queues_IPCQueue * queue = (struct Queues_IPCQueue *) _Utilities_External -> getMappedPages(nBufferPages, true);
    if (queue == NULL) {
        _Utilities_External -> free(writerQueue);
        return NULL;
    }

    queue -> readerPointer = 0;
    queue -> writerPointer = 0;
    queue -> outOfSpace = false;

    writerQueue -> queue = queue;
    writerQueue -> dataLocation = (u8 *)queue + sizeof(struct Queues_IPCQueue);
    writerQueue -> queueSize = nBufferPages * PAGE_SIZE - sizeof(struct Queues_IPCQueue);
    writerQueue -> nBufferPages = nBufferPages;
    writerQueue -> writerPointer = 0;
    synchronization_InitializeSpinlock(&writerQueue -> lock);
    
    SharedMemory_SegmentID segment = _Utilities_External -> sharedMemory_CreateSegment(queue, nBufferPages);
    if (segment == SHARED_MEMORY_ERROR_SEGMENT) {
        _Utilities_External -> returnMappedPages(queue, nBufferPages);
        _Utilities_External -> free(writerQueue);
        return NULL;
    }

    writerQueue -> segment = segment;
    writerQueue -> readerPID = 0;
    writerQueue -> thread = NULL;
    return writerQueue;
}

u32 queues_IPCQueue_AddReader_External(struct Queues_IPCQueue_Writer * queue, PID pid, MemoryPage thread, u32 permission, struct _Utilities_ExternalStructure * _Utilities_External) {
    
    u32 result = _Utilities_External -> sharedMemory_GrantProcessPermission(queue -> segment, pid, true);
    if (result == GENERIC_ERROR_RESULT) {
        return result;
    }

    queue -> readerPID = pid;
    queue -> thread = thread;
    queue -> permission = permission;
    return GENERIC_SUCCESS_RESULT;
}

struct Queues_IPCQueue_Reader * queues_IPCQueue_GetFromWriter_External(PID writerPID, SharedMemory_SegmentID segment, u32 nBufferPages, u32 queueOffset, struct _Utilities_ExternalStructure * _Utilities_External) {
    if (queueOffset >= nBufferPages * PAGE_SIZE) {
        return NULL;
    }

    struct Queues_IPCQueue_Reader * readerQueue = _Utilities_External -> malloc(sizeof(struct Queues_IPCQueue_Reader));
    if (readerQueue == NULL) {
        return NULL;
    }

    MemoryPage buffer = _Utilities_External -> getUnmappedPages(nBufferPages);
    if (buffer == NULL) {
        _Utilities_External -> free(readerQueue);
        return NULL;
    }

    u32 result = _Utilities_External -> systemCall_SharedMemory_MapSegment(writerPID, segment, 0, buffer, nBufferPages, SYSTEM_CALL_NO_PERMISSION);
    if (result == GENERIC_ERROR_RESULT) {
        _Utilities_External -> free(readerQueue);
        _Utilities_External -> returnUnmappedPages(buffer, nBufferPages);
        return NULL;
    }
    struct Queues_IPCQueue * queue = (struct Queues_IPCQueue *)buffer;

    readerQueue -> dataLocation = (u8*)queue + queueOffset;
    readerQueue -> queueSize = nBufferPages * PAGE_SIZE - queueOffset;
    readerQueue -> nBufferPages = nBufferPages;

    readerQueue -> readerPointer = 0;
    synchronization_InitializeSpinlock(&readerQueue -> lock);

    readerQueue -> segment = segment;
    readerQueue -> writerPID = writerPID;

    readerQueue -> queue = queue;

    return readerQueue;
}

u32 queues_IPCQueue_WriteData_External(struct Queues_IPCQueue_Writer * queue, void * data, u32 size, struct _Utilities_ExternalStructure * _Utilities_External) {
    u32 freeSpace;
    u32 readerPointer = queue -> queue -> readerPointer;
    if (readerPointer >= queue -> queueSize) {
        return GENERIC_ERROR_RESULT;
    }

    synchronization_OpenSpinlock(&queue -> lock);

    if (readerPointer > queue -> writerPointer) {
        freeSpace = readerPointer - queue -> writerPointer;
    }
    else {
        freeSpace = readerPointer + queue -> queueSize - queue -> writerPointer;
    }

    if (size > freeSpace) {
        queue -> queue -> outOfSpace = true;
        if (!(queue -> queue -> readerExecuting) ) {
            _Utilities_External -> systemCall_ProcessThread_ChangeThreadExecution(queue -> readerPID, queue -> thread, SYSTEM_CALL_CHANGE_THREAD_EXECUTION_SCHEDULE, queue -> permission);
        }
        synchronization_CloseSpinlock(&queue -> lock);

        return GENERIC_ERROR_RESULT;
    }

    if (size < queue -> queueSize - queue -> writerPointer) {
        u32 i;
        for (i = 0; i < size; i++) {
            queue -> dataLocation[queue -> writerPointer + i] = ((u8*)data)[i];
        }
        queue -> writerPointer += size;
    }
    else {
        u32 i;
        u32 endSize = queue -> queueSize - queue -> writerPointer;
        u32 startSize = size - endSize;

        for (i = 0; i < endSize; i++) {
            queue -> dataLocation[queue -> writerPointer + i] = ((u8*)data)[i];
        }
        for (i = 0; i < startSize; i++) {
            queue -> dataLocation[i] = ((u8*)data)[endSize + i];
        }

        queue -> writerPointer = startSize;
    }

    queue -> queue -> writerPointer = queue -> writerPointer;
    if (!(queue -> queue -> readerExecuting) ) {
        _Utilities_External -> systemCall_ProcessThread_ChangeThreadExecution(queue -> readerPID, queue -> thread, SYSTEM_CALL_CHANGE_THREAD_EXECUTION_SCHEDULE, queue -> permission);
    }
    synchronization_CloseSpinlock(&queue -> lock);
    return GENERIC_SUCCESS_RESULT;
}

u32 queues_IPCQueue_WriteDataNoWakeup(struct Queues_IPCQueue_Writer * queue, void * data, u32 size) {
    u32 freeSpace;
    u32 readerPointer = queue -> queue -> readerPointer;
    if (readerPointer >= queue -> queueSize) {
        return GENERIC_ERROR_RESULT;
    }

    synchronization_OpenSpinlock(&queue -> lock);

    if (readerPointer > queue -> writerPointer) {
        freeSpace = readerPointer - queue -> writerPointer;
    }
    else {
        freeSpace = readerPointer + queue -> queueSize - queue -> writerPointer;
    }

    if (size > freeSpace) {
        queue -> queue -> outOfSpace = true;
        synchronization_CloseSpinlock(&queue -> lock);

        return GENERIC_ERROR_RESULT;
    }

    if (size < queue -> queueSize - queue -> writerPointer) {
        u32 i;
        for (i = 0; i < size; i++) {
            queue -> dataLocation[queue -> writerPointer + i] = ((u8*)data)[i];
        }
        queue -> writerPointer += size;
    }
    else {
        u32 i;
        u32 endSize = queue -> queueSize - queue -> writerPointer;
        u32 startSize = size - endSize;

        for (i = 0; i < endSize; i++) {
            queue -> dataLocation[queue -> writerPointer + i] = ((u8*)data)[i];
        }
        for (i = 0; i < startSize; i++) {
            queue -> dataLocation[i] = ((u8*)data)[endSize + i];
        }

        queue -> writerPointer = startSize;
    }

    queue -> queue -> writerPointer = queue -> writerPointer;

    synchronization_CloseSpinlock(&queue -> lock);
    return GENERIC_SUCCESS_RESULT;
}

u32 queues_IPCQueue_WriterWakeup_External(struct Queues_IPCQueue_Writer * queue, struct _Utilities_ExternalStructure * _Utilities_External) {
    if (!(queue -> queue -> readerExecuting) ) {
        _Utilities_External -> systemCall_ProcessThread_ChangeThreadExecution(queue -> readerPID, queue -> thread, SYSTEM_CALL_CHANGE_THREAD_EXECUTION_SCHEDULE, queue -> permission);
    }
    return GENERIC_SUCCESS_RESULT;
}

u32 queues_IPCQueue_ReadData(struct Queues_IPCQueue_Reader * queue, void * data, u32 size) {
    u32 writerPointer = queue -> queue -> writerPointer;
    if (writerPointer >= queue -> queueSize) {
        return GENERIC_ERROR_RESULT;
    }

    synchronization_OpenSpinlock(&queue -> lock);
    
    u32 unreadDataSize;
    if (writerPointer >= queue -> readerPointer) {
        unreadDataSize = writerPointer - queue -> readerPointer;
    }
    else {
        unreadDataSize = writerPointer + queue -> queueSize - queue -> readerPointer;
    }

    if (size > unreadDataSize) {
        synchronization_CloseSpinlock(&queue -> lock);
        return GENERIC_ERROR_RESULT;
    }

    if (size < queue -> queueSize - queue -> readerPointer) {
        u32 i;
        for (i = 0; i < size; i++) {
            ((u8*)data)[i] = queue -> dataLocation[queue -> readerPointer + i];
        }
        queue -> readerPointer += size;
    }
    else {
        u32 i;
        u32 endSize = queue -> queueSize - queue -> readerPointer;
        u32 startSize = size - endSize;

        for (i = 0; i < endSize; i++) {
            ((u8*)data)[i] = queue -> dataLocation[queue -> readerPointer + i];
        }
        for (i = 0; i < startSize; i++) {
            ((u8*)data)[endSize + i] = queue -> dataLocation[i];
        }

        queue -> readerPointer = startSize;
    }
    queue -> queue -> readerPointer = queue -> readerPointer;

    synchronization_CloseSpinlock(&queue -> lock);
    return GENERIC_SUCCESS_RESULT;
}

u32 queues_IPCQueue_DeleteWriter(struct Queues_IPCQueue_Writer * queue) {
    //TODO -- implement!!!!
}
