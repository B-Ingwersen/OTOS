#ifndef PS2_H
#define PS2_H

#include "OTOSCore/Definitions.h"
#include "OTOSCore/Synchronization.h"

extern Synchronization_Spinlock ps2Spinlock;

extern u32 ps2_keycodeSet;

void ps2_WaitForCommandResponse();

void ps2_WaitForWriteReady();

bool32 sendByteToKeyboard(u8 byte);

void ps2_Initialize();

u8 ps2_ReadKeyCode();

#endif