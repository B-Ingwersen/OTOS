
#include "IPC.h"
#include "Utilities/Queues.h"
#include "OTOSCore/CStandardLibrary/stdlib.h"

struct ProcessPermission * processPermissions = NULL;
PID currentForwardProcess = 0;

void ipc_Initialize() {
    processPermissions = malloc(sizeof(struct ProcessPermission) * MAX_NUMBER_OF_PROCESSES);

    u32 i;
    for (i = 0; i < MAX_NUMBER_OF_PROCESSES; i++) {
        processPermissions[i].pid = 0;
    }
}

u32 ipc_AddProcess(PID pid, u32 nBufferPages, MemoryPage thread, SharedMemory_SegmentID * returnSegment, u32 * returnQueueOffset) {
    
    if (nBufferPages > MAX_BUFFER_PAGES) {
        return GENERIC_ERROR_RESULT;
    }

    u32 process = pid & 0xFF;

    if (processPermissions[process].pid == pid) {
        return GENERIC_ERROR_RESULT;
    }

    if (processPermissions[process].queue != NULL) {
        queues_IPCQueue_DeleteWriter(processPermissions[process].queue);
        processPermissions[process].queue = NULL;
    }

    struct Queues_IPCQueue_Writer * queue = queues_IPCQueue_Create(nBufferPages);
    if (queue == NULL) {
        return GENERIC_ERROR_RESULT;
    }

    if (!queues_IPCQueue_AddReader(queue, pid, thread, SYSTEM_CALL_PROCESS_PERMISSION)) {
        queues_IPCQueue_DeleteWriter(queue);
        return GENERIC_ERROR_RESULT;
    }

    processPermissions[process].pid = pid;
    processPermissions[process].queue = queue;
    *returnSegment = queue -> segment;
    *returnQueueOffset = sizeof(struct Queues_IPCQueue);
    currentForwardProcess = pid;
    return GENERIC_SUCCESS_RESULT;
}

u32 forwardByte_WriteData(u8 byte) {
    if (currentForwardProcess == 0) {
        return GENERIC_SUCCESS_RESULT;
    }

    u32 process = currentForwardProcess & 0xFF;
    if (processPermissions[process].pid != currentForwardProcess) {
        return GENERIC_ERROR_RESULT;
    }

    return queues_IPCQueue_WriteDataNoWakeup(processPermissions[process].queue, &byte, 1);
}

u32 forwardByte_Wakeup() {
    u32 process = currentForwardProcess & 0xFF;
    if (processPermissions[process].pid != 0) {
        return queues_IPCQueue_WriterWakeup(processPermissions[process].queue);
    }
    else {
        return GENERIC_SUCCESS_RESULT;
    }
}