
#include "Messaging.h"
#include "IPC.h"
#include "KeyboardManager/MessagingDefinitions.h"
#include "OTOSCore/SystemCalls.h"

#include "Debugging.h"

void messaging_DefaultMessageHandler(PID pid, MemoryPage dataPage) {

    void * data = (void *) ( ((IntegerPointer)dataPage) + SYSTEM_CALL_MESSAGING_DATA_OFFSET );
    u32 operation = *(u32*)data;

    if (operation == KEYBOARD_MANAGER_MESSAGING_OPERATION_TEST) {
        messaging_Test(pid, data);
    }

    else if (operation == KEYBOARD_MANAGER_MESSAGING_OPERATION_REQUEST_FORWARDING) {
        messaging_RequestForwarding(pid, data);
    }

    systemCall_Messaging_MessageReturn();
}

void messaging_Test(PID pid, void * dataPointer) {
    struct KeyboardManager_Messaging_Test * data = (struct KeyboardManager_Messaging_Test *)dataPointer;
    data -> result = GENERIC_SUCCESS_RESULT;
}

void messaging_RequestForwarding(PID pid, void * dataPointer) {
    struct KeyboardManager_Messaging_RequestForwarding * data = (struct KeyboardManager_Messaging_RequestForwarding *)dataPointer;
    //data -> result = GENERIC_SUCCESS_RESULT;

    data -> result = ipc_AddProcess(pid, data -> nBufferPages, data -> thread, &(data -> returnSegment), &(data -> returnQueueOffset));
}