asm ("jmp _start");

#include "OTOSCore/Definitions.h"
#include "OTOSCore/SystemCalls.h"
#include "OTOSCore/MemoryAllocation.h"
#include "OTOSCore/Threads.h"

#include "Modes.h"
#include "VESA.h"
#include "BIOS.h"
#include "Messaging.h"

void _start() {
    OTOSCore_Load((MemoryPage)0x7FF0000, (MemoryPage)0x8000000, 0x00010003);

    memoryAllocation_DefaultInitialization(0x10000000, 0xF0000000, (MemoryPage)0xF0000000);
    threads_SetupThread();

    initializeModes();

    bios_addStandardVideoModes();
    vesa_GetVideoModes();

    systemCall_Messaging_Initialize((MemoryPage)0xFF000000);
    systemCall_Messaging_SetMessageHandler(messaging_DefaultMessageHandler);

    systemCall_ProcessThread_ChangeThreadExecution(0, (MemoryPage)0xFFB01000, SYSTEM_CALL_CHANGE_THREAD_EXECUTION_HALT, SYSTEM_CALL_NO_PERMISSION);

    asm volatile (
        "mov eax, 0xC0DE0004\n\tjmp $" :: "b" (nVideoModes)
    );
}