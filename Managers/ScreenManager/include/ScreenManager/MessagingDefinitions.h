
#ifndef SCREEN_MANAGER___MESSAGING_DEFINITIONS_H
#define SCREEN_MANAGER___MESSAGING_DEFINITIONS_H

#include "OTOSCore/Definitions.h"

#define SCREEN_MANAGER_MESSAGING_OPERATION_TEST 0
#define SCREEN_MANAGER_MESSAGING_OPERATION_GET_MODE_LIST 1
#define SCREEN_MANAGER_MESSAGING_OPERATION_SET_MODE 2
#define SCREEN_MANAGER_MESSAGING_OPERATION_GET_FRAMEBUFFER 3

#define SCREEN_VIDEO_MODE_TYPE_TEXT 1
#define SCREEN_VIDEO_MODE_TYPE_GRAPHICS 2

struct ScreenManager_Messaging_Test {
    u32 operation;
    u32 result;
} ExactBinaryStructure;

struct ScreenManager_Messaging_ModeInfo {
    u32 type;
    u32 width;
    u32 height;
    u32 colorDepth;
    u32 frameBufferNPages;
} ExactBinaryStructure;

struct ScreenManager_Messaging_GetModeList {
    u32 operation;
    u32 result;
    u32 nModes;

    struct ScreenManager_Messaging_ModeInfo modeList[64];
} ExactBinaryStructure;

struct ScreenManager_Messaging_SetMode {
    u32 operation;
    u32 result;

    u32 mode;
} ExactBinaryStructure;

struct ScreenManager_Messaging_GetFramebuffer {
    u32 operation;
    u32 result;

    IntegerPointer loadLocation;
    u32 nAllocatedPages;
} ExactBinaryStructure;

#endif // SCREEN_MANAGER___MESSAGING_DEFINITIONS_H