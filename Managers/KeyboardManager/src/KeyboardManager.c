asm ("jmp _start");

#include "OTOSCore/Definitions.h"
#include "OTOSCore/SystemCalls.h"
#include "OTOSCore/Threads.h"
#include "OTOSCore/SharedMemory.h"
#include "Utilities/Definitions.h"
#include "Debugging.h"
#include "PS2.h"
#include "IPC.h"
#include "Messaging.h"
#include "InterruptHandler.h"

void _start() {
    OTOSCore_Load((MemoryPage)0x7FF0000, (MemoryPage)0x8000000, 0x00010003);
    memoryAllocation_DefaultInitialization(0x10000000, 0xF0000000, (MemoryPage)0xF0000000);
    threads_SetupThread();
    systemCall_Messaging_Initialize((MemoryPage)0xFF000000);
    sharedMemory_Initialize();
    debugging_Initialize();
    debugging_PrintString("Keyboard Driver START!");

    ps2_Initialize();
    utilities_initialize();
    ipc_Initialize();

    u32 result = systemCall_Interrupt_GetInterruptForward(0x21, keyboardHardwareInterruptHandler, keyboardHardwareImmediateHandler);

    systemCall_Messaging_SetMessageHandler(messaging_DefaultMessageHandler);

    debugging_PrintString("Keyboard Driver Done Initializing");

    systemCall_ProcessThread_ChangeThreadExecution(0, (MemoryPage)0xFFB01000, SYSTEM_CALL_CHANGE_THREAD_EXECUTION_HALT, SYSTEM_CALL_NO_PERMISSION);

    asm volatile (
        "mov eax, 0xC0DE0005\n\tjmp $"
    );
}