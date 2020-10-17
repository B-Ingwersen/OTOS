#ifndef KEYBOARD_MANAGER___MESSAGING_DEFINTIONS_H
#define KEYBOARD_MANAGER___MESSAGING_DEFINTIONS_H

#include "OTOSCore/Definitions.h"

#define KEYBOARD_MANAGER_MESSAGING_OPERATION_TEST 0
#define KEYBOARD_MANAGER_MESSAGING_OPERATION_REQUEST_FORWARDING 1

struct KeyboardManager_Messaging_Test {
    u32 operation;
    u32 result;
} ExactBinaryStructure;

struct KeyboardManager_Messaging_RequestForwarding {
    u32 operation;
    u32 result;
    u32 nBufferPages;
    MemoryPage thread;
    SharedMemory_SegmentID returnSegment;
    u32 returnQueueOffset;
} ExactBinaryStructure;

#endif