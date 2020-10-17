
#ifndef DEBUGGING_H
#define DEBUGGING_H

#include "OTOSCore/Definitions.h"

extern MemoryPage debugging_VideoMemory;
extern u32 debug_Offset;

void debugging_Initialize();

void debugging_PrintString(u8 * string);

void debugging_PrintStringLength(u8 * string, u32 length);

void debugging_Panic(u8 * errorMessage);

void debugging_SaveToRegisters(u32 eax, u32 ebx, u32 ecx, u32 edx);

void debugging_HexDump(void * data, u32 bytes);

#endif