
#include "Processes.h"
#include "ProcessManager/MessagingDefinitions.h"

#include "OTOSCore/Threads.h"
#include "OTOSCore/SystemCalls.h"
#include "OTOSCore/CStandardLibrary/string.h"

u32 processes_GetProcessPID(u8 * processName) {
    u32 nameLength = strlen(processName);

    MemoryPage messageDataPage = threads_GetLocalStorage() -> messagingDataPage;
    struct ProcessManager_Messaging_GetProcessPID * data = (struct ProcessManager_Messaging_GetProcessPID *)( ((IntegerPointer)messageDataPage) + SYSTEM_CALL_MESSAGING_DATA_OFFSET);

    data -> operation = PROCESS_MANAGER_MESSAGING_OPERATION_GET_PROCESS_PID;
    data -> nameLength = nameLength;
    strncpy(data -> name, processName, nameLength);

    u32 result = systemCall_Messaging_MessageProcess(PROCESS_MANAGER_PID, messageDataPage);
    if (result == GENERIC_ERROR_RESULT) {
        return 0;
    }

    return data -> pid;
}