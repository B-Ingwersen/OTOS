asm ("jmp _start");

#include "OTOSCore/Definitions.h"
#include "OTOSCore/MemoryAllocation.h"
#include "OTOSCore/SystemCalls.h"
#include "OTOSCore/Threads.h"
#include "OTOSCore/Disk.h"
#include "Debugging.h"
#include "ProcessManagement.h"
#include "Messaging.h"

void _start() {
    memoryAllocation_DefaultInitialization(0x40000000, 0xC0000000, (MemoryPage)0xC0000000);
    debugging_Initialize();
    threads_SetupThread();
    initializeProcessManagement();

    systemCall_Messaging_Initialize((MemoryPage)0xFF000000);
    systemCall_Messaging_SetMessageHandler(messaging_DefaultMessageHandler);

    debugging_PrintString("INIT DONE");

    waitForProcessToInitialize(DISK_MANAGER_PID);
    debugging_PrintString("DISK DONE");

    PID LibraryManagerPID = createProcessFromDiskFile("/Managers/LibraryManager", 24, "LibraryManager", 14, NULL, 0);
    debugging_PrintString("LIBR LOAD");
    waitForProcessToInitialize(LibraryManagerPID);
    debugging_PrintString("LIBR DONE");

    u32 screenManagerPermissions[3] = {PROCESS_PERMISSION_KERNEL_MEMORY, PROCESS_PERMISSION_OTHER_PROCESS_MEMORY, PROCESS_PERMISSION_BIOS_CALL};
    PID ScreenManagerPID = createProcessFromDiskFile("/Managers/ScreenManager", 23, "ScreenManager", 13, screenManagerPermissions, 3);
    waitForProcessToInitialize(ScreenManagerPID);

    u32 keyboardManagerPermissions[4] = {PROCESS_PERMISSION_INTERRUPT_FORWARD, PROCESS_PERMISSION_IO_PORTS, PROCESS_PERMISSION_KERNEL_MEMORY, PROCESS_PERMISSION_OTHER_PROCESS_THREAD};
    PID KeyboardManagerPID = createProcessFromDiskFile("/Managers/KeyboardManager", 25, "KeyboardManager", 15, keyboardManagerPermissions, 4);
    debugging_PrintString("KEYB LOAD");
    waitForProcessToInitialize(KeyboardManagerPID);

    PID ShellPID = createProcessFromDiskFile("/Applications/Shell", 19, "Shell", 5, NULL, 0);

    systemCall_ProcessThread_ChangeThreadExecution(0, (MemoryPage)0xFFB01000, SYSTEM_CALL_CHANGE_THREAD_EXECUTION_HALT, SYSTEM_CALL_NO_PERMISSION);

    asm volatile (
        "mov eax, 0xC0DE0007\n\tjmp $" ::
    );
}