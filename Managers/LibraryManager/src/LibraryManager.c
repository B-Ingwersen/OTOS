asm ("jmp _start");

#include "OTOSCore/MemoryAllocation.h"
#include "OTOSCore/Threads.h"
#include "OTOSCore/SharedMemory.h"
#include "OTOSCore/SystemCalls.h"

#include "LibraryManagement.h"
#include "Messaging.h"

void _start() {
    memoryAllocation_DefaultInitialization(0x10000000, 0xF0000000, (MemoryPage)0xF0000000);
    threads_SetupThread();
    sharedMemory_Initialize();
    setupLibraryManagement();    

    systemCall_Messaging_Initialize((MemoryPage)0xFF000000);
    systemCall_Messaging_SetMessageHandler(messaging_DefaultMessageHandler);

    systemCall_ProcessThread_ChangeThreadExecution(0, (MemoryPage)0xFFB01000, SYSTEM_CALL_CHANGE_THREAD_EXECUTION_HALT, SYSTEM_CALL_NO_PERMISSION);
    asm volatile("mov eax, 0xC0DE0003\n\tjmp $");
}