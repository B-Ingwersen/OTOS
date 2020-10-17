
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "OTOSCore/Definitions.h"
#include "Utilities/Queues.h"

extern PID KEYBOARD_MANAGER_PID;
extern MemoryPage keyboardThread;
extern struct Queues_IPCQueue_Reader * keyboardQueue;
extern void (*keyboard_handlingFunction)(u32 keycode, bool32 release);

void keyboardThreadFunction();

u32 getKeyboardForward(void (*handlingFunction)(u32 keycode, bool32 release));

#endif