__asm__ ("jmp _start");

#include "Initialization.h"
#include "Debugging.h"

#include "OTOSCore/MemoryAllocation.h"
#include "OTOSCore/SharedMemory.h"
#include "OTOSCore/SystemCalls.h"

void _start() {
    memoryAllocation_DefaultInitialization(0x40000000, 0xC0000000, (MemoryPage)0xC0000000);
    sharedMemory_Initialize();
    debugging_Initialize();

    initializeDiskManager();

    systemCall_ProcessThread_ChangeThreadExecution(0, (MemoryPage)0xFFB01000, SYSTEM_CALL_CHANGE_THREAD_EXECUTION_HALT, SYSTEM_CALL_NO_PERMISSION);
    asm volatile (
        "mov eax, 0xC0DE0002\n\tjmp $"
    );
}