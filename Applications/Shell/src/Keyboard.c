
#include "Keyboard.h"
#include "KeyboardManager/MessagingDefinitions.h"
#include "Processes.h"

#include "OTOSCore/Threads.h"

PID KEYBOARD_MANAGER_PID = 0;
MemoryPage keyboardThread = 0;
struct Queues_IPCQueue_Reader * keyboardQueue = NULL;
void (*keyboard_handlingFunction)(u32 keycode, bool32 release) = NULL;

void keyboardThreadFunction() {
    threads_SetupThread();

    bool8 release = false;
    while (true) {
        if (keyboardQueue != NULL) {
            u8 keyboardByteIn;
            if (queues_IPCQueue_ReadData(keyboardQueue, &keyboardByteIn, 1) == GENERIC_SUCCESS_RESULT) {
                if (keyboardByteIn == 0xF0) {
                    release = true;
                }
                else {
                    
                    keyboard_handlingFunction((u32)keyboardByteIn, release);
                    release = false;
                }
            }
            else {
                keyboardQueue -> queue -> readerExecuting = false;
                systemCall_ProcessThread_ChangeThreadExecution(0, keyboardThread, SYSTEM_CALL_CHANGE_THREAD_EXECUTION_HALT, SYSTEM_CALL_NO_PERMISSION);
                keyboardQueue -> queue -> readerExecuting = true;
            }
        }
    }
}

u32 getKeyboardForward(void (*handlingFunction)(u32 keycode, bool32 release)) {

    keyboard_handlingFunction = handlingFunction;

    KEYBOARD_MANAGER_PID = processes_GetProcessPID("KeyboardManager");
    u32 nBufferPages = 4;

    MemoryPage messageDataPage = threads_GetLocalStorage() -> messagingDataPage;
    struct KeyboardManager_Messaging_RequestForwarding * data = (struct KeyboardManager_Messaging_RequestForwarding *)( ((IntegerPointer)messageDataPage) + SYSTEM_CALL_MESSAGING_DATA_OFFSET);

    MemoryPage threadPages = getUnmappedPages(2);
    systemCall_Memory_MapNewMemory(0, threadPages, 1, true, SYSTEM_CALL_NO_PERMISSION);
    MemoryPage tStack = threadPages + PAGE_SIZE - sizeof(void *);
    keyboardThread = threadPages + PAGE_SIZE;

    systemCall_ProcessThread_CreateThread(0, keyboardThread, keyboardThreadFunction, tStack, SYSTEM_CALL_NO_PERMISSION);
    systemCall_ProcessThread_ChangeThreadExecution(0, keyboardThread, SYSTEM_CALL_CHANGE_THREAD_EXECUTION_SCHEDULE, SYSTEM_CALL_NO_PERMISSION);

    data -> operation = KEYBOARD_MANAGER_MESSAGING_OPERATION_REQUEST_FORWARDING;
    data -> nBufferPages = nBufferPages;
    data -> thread = keyboardThread;
    
    u32 result = systemCall_Messaging_MessageProcess(KEYBOARD_MANAGER_PID, messageDataPage);
    if (result == GENERIC_ERROR_RESULT) {
        return result;
    }

    keyboardQueue = queues_IPCQueue_GetFromWriter(KEYBOARD_MANAGER_PID, data -> returnSegment, nBufferPages, data -> returnQueueOffset);
}