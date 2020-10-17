#include "Messaging.h"
#include "ProcessManagement.h"
#include "OTOSCore/SystemCalls.h"

void messaging_GetProcessPID(PID pid, void * dataPointer);

void messaging_DefaultMessageHandler(PID pid, MemoryPage dataPage) {
    void * data = (void *) ( ((IntegerPointer)dataPage) + SYSTEM_CALL_MESSAGING_DATA_OFFSET );
    u32 operation = *(u32*)data;

    if (operation == PROCESS_MANAGER_MESSAGING_OPERATION_GET_PROCESS_PID) {
        messaging_GetProcessPID(pid, data);
    }

    systemCall_Messaging_MessageReturn();
}

void messaging_GetProcessPID(PID pid, void * dataPointer) {
    struct ProcessManager_Messaging_GetProcessPID * data = (struct ProcessManager_Messaging_GetProcessPID *)dataPointer;

    data -> pid = getProcessPID_SearchableName(data -> name, data -> nameLength);
    
    if (data -> pid == ERROR_PID) {
        data -> result = GENERIC_ERROR_RESULT;
    }
    else {
        data -> result = GENERIC_SUCCESS_RESULT;
    }
}

