#ifndef SCREEN_H
#define SCREEN_H

#include "OTOSCore/Definitions.h"
#include "ScreenManager/MessagingDefinitions.h"

#define SCREEN_VIDEO_MODE_ERROR 0xFFFFFFFF

extern PID SCREEN_MANAGER_PID;
extern void * frameBuffer;
extern u32 frameBuffer_nPages;

void screen_Initialize();

struct ScreenManager_Messaging_ModeInfo * screen_GetModeList(u32 * nModes);

u32 screen_GetVideoMode(u32 type, u32 width, u32 height, struct ScreenManager_Messaging_ModeInfo * returnInfo);

u32 screen_GetHighestResMode(u32 type, u32 maxWidth, u32 maxHeight, struct ScreenManager_Messaging_ModeInfo * returnInfo);

bool8 screen_getModeInfo(u32 mode, struct ScreenManager_Messaging_ModeInfo * returnInfo);

u32 screen_SetVideoMode(u32 mode);

void screen_GetFramebuffer(u32 nFramebufferPages);

#endif