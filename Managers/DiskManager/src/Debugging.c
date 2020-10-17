
#include "Debugging.h"
#include "OTOSCore/MemoryAllocation.h"
#include "OTOSCore/SystemCalls.h"

MemoryPage debugging_VideoMemory = NULL;
u32 debug_Offset = 0;

void debugging_Initialize() {
    MemoryPage videoMemory = getUnmappedPage();
    systemCall_Memory_MapKernelMemory(0, videoMemory, 0xB8000, 1, true, SYSTEM_CALL_NO_PERMISSION);
    debugging_VideoMemory = videoMemory;
}

void debugging_PrintString(u8 * string) {
    MemoryPage videoMemory = debugging_VideoMemory;
    u8 * screen = (u8*)videoMemory;

    u32 i;
    for (i = 0; string[i] != 0; i++) {
        screen[2 * i] = string[i];
    }
}

void debugging_PrintStringLength(u8 * string, u32 length) {
    MemoryPage videoMemory = debugging_VideoMemory;
    u8 * screen = (u8*)videoMemory;

    u32 i;
    for (i = 0; string[i] != 0 && i < length; i++) {
        screen[2 * i] = string[i];
    }
}

void debugging_Panic(u8 * errorMessage) {
    debugging_PrintString(errorMessage);
    systemCall_ProcessThread_ChangeThreadExecution(0, (MemoryPage)0xFFB00000, SYSTEM_CALL_CHANGE_THREAD_EXECUTION_HALT, SYSTEM_CALL_NO_PERMISSION);
}

void debugging_SaveToRegisters(u32 eax, u32 ebx, u32 ecx, u32 edx) {
    asm volatile("jmp $":: "a"(eax), "b"(ebx), "c"(ecx), "d"(edx) );
}

void debugging_HexDump(void * data, u32 bytes) {
    u8 * d = (u8*)data;
    u8 * screen = (u8*)debugging_VideoMemory + debug_Offset + 1600;

    u32 i;
    for (i = 0; i < bytes; i++) {
        u8 lowChar = (d[i] & 0x0F) + '0';
        u8 highChar = (d[i] >> 4) + '0';

        if (lowChar > '9') {
            lowChar += 'A' - ('0' + 10);
        }
        if (highChar > '9') {
            highChar += 'A' - ('0' + 10);
        }

        screen[4 * i] = highChar;
        screen[4 * i + 2] = lowChar;
    }

    debug_Offset += bytes * 4 + 4;
}