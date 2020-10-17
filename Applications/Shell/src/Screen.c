
#include "Screen.h"
#include "Processes.h"

#include "OTOSCore/Threads.h"
#include "OTOSCore/SystemCalls.h"

PID SCREEN_MANAGER_PID = 0;
void * frameBuffer = NULL;
u32 frameBuffer_nPages = 0;

void screen_Initialize() {
    SCREEN_MANAGER_PID = processes_GetProcessPID("ScreenManager");
}

struct ScreenManager_Messaging_ModeInfo * screen_GetModeList(u32 * nModes) {

    MemoryPage messageDataPage = threads_GetLocalStorage() -> messagingDataPage;
    struct ScreenManager_Messaging_GetModeList * data = (struct ScreenManager_Messaging_GetModeList *)( ((IntegerPointer)messageDataPage) + SYSTEM_CALL_MESSAGING_DATA_OFFSET);

    data -> operation = SCREEN_MANAGER_MESSAGING_OPERATION_GET_MODE_LIST;
    u32 result = systemCall_Messaging_MessageProcess(SCREEN_MANAGER_PID, messageDataPage);
    if (result == GENERIC_ERROR_RESULT) {
        return NULL;
    }
    else {
        *nModes = data -> nModes;
        return data -> modeList;
    }
}

u32 screen_GetVideoMode(u32 type, u32 width, u32 height, struct ScreenManager_Messaging_ModeInfo * returnInfo) {
    u32 nVideoModes;
    struct ScreenManager_Messaging_ModeInfo * videoModes = screen_GetModeList(&nVideoModes);
    if (videoModes == NULL) {
        return SCREEN_VIDEO_MODE_ERROR;
    }

    u32 i;
    for (i = 0; i < nVideoModes; i++) {
        if (videoModes[i].type == type && videoModes[i].width == width && videoModes[i].height == height) {
            if (returnInfo != NULL) {
                returnInfo -> type = videoModes[i].type;
                returnInfo -> width = videoModes[i].width;
                returnInfo -> height = videoModes[i].height;
                returnInfo -> colorDepth = videoModes[i].colorDepth;
                returnInfo -> frameBufferNPages = videoModes[i].frameBufferNPages;
            }

            return i;
        }
    }
    return SCREEN_VIDEO_MODE_ERROR;
}

u32 screen_GetHighestResMode(u32 type, u32 maxWidth, u32 maxHeight, struct ScreenManager_Messaging_ModeInfo * returnInfo) {
    u32 nVideoModes;
    struct ScreenManager_Messaging_ModeInfo * videoModes = screen_GetModeList(&nVideoModes);
    if (videoModes == NULL) {
        return SCREEN_VIDEO_MODE_ERROR;
    }

    u32 mode = SCREEN_VIDEO_MODE_ERROR;

    u32 width = 0;
    u32 height = 0;

    u32 i;
    for (i = 0; i < nVideoModes; i++) {
        if (videoModes[i].type == type && (videoModes[i].width * videoModes[i].height) > height * width) {
            if (maxWidth != 0 && videoModes[i].width > maxWidth) {
                continue;
            }
            if (maxHeight != 0 && videoModes[i].height > maxHeight) {
                continue;
            }
            mode = i;
            width = videoModes[i].width;
            height = videoModes[i].height;

            if (returnInfo != NULL) {
                (returnInfo) -> type = videoModes[i].type;
                (returnInfo) -> width = videoModes[i].width;
                (returnInfo) -> height = videoModes[i].height;
                (returnInfo) -> colorDepth = videoModes[i].colorDepth;
                (returnInfo) -> frameBufferNPages = videoModes[i].frameBufferNPages;
            }
        }
    }
    return mode;
}

bool8 screen_getModeInfo(u32 mode, struct ScreenManager_Messaging_ModeInfo * returnInfo) {
    u32 nVideoModes;
    struct ScreenManager_Messaging_ModeInfo * videoModes = screen_GetModeList(&nVideoModes);
    if (videoModes == NULL) {
        return false;
    }

    if (mode >= nVideoModes) {
        return false;
    }

    returnInfo -> type = videoModes[mode].type;
    returnInfo -> width = videoModes[mode].width;
    returnInfo -> height = videoModes[mode].height;
    returnInfo -> colorDepth = videoModes[mode].colorDepth;
    returnInfo -> frameBufferNPages = videoModes[mode].frameBufferNPages;

    return true;
}

u32 screen_SetVideoMode(u32 mode) {
    MemoryPage messageDataPage = threads_GetLocalStorage() -> messagingDataPage;
    struct ScreenManager_Messaging_SetMode * data = (struct ScreenManager_Messaging_SetMode *)( ((IntegerPointer)messageDataPage) + SYSTEM_CALL_MESSAGING_DATA_OFFSET);

    data -> operation = SCREEN_MANAGER_MESSAGING_OPERATION_SET_MODE;
    data -> mode = mode;
    u32 result = systemCall_Messaging_MessageProcess(SCREEN_MANAGER_PID, messageDataPage);
    if (result == GENERIC_ERROR_RESULT) {
        return result;
    }
    return data -> result;
}

void screen_GetFramebuffer(u32 nFramebufferPages) {
    // free old framebuffer
    if (frameBuffer != NULL) {
        u32 result = systemCall_Memory_UnmapMemory(0, frameBuffer, frameBuffer_nPages, true, SYSTEM_CALL_NO_PERMISSION);
        returnUnmappedPages(frameBuffer, frameBuffer_nPages);
    }

    MemoryPage messageDataPage = threads_GetLocalStorage() -> messagingDataPage;
    struct ScreenManager_Messaging_GetFramebuffer * data = (struct ScreenManager_Messaging_GetFramebuffer *)( ((IntegerPointer)messageDataPage) + SYSTEM_CALL_MESSAGING_DATA_OFFSET);

    void * loadLocation = getUnmappedPages(nFramebufferPages);
    if (loadLocation == NULL) {
        returnUnmappedPages(loadLocation, nFramebufferPages);
        frameBuffer = NULL;
        return;
    }

    data -> operation = SCREEN_MANAGER_MESSAGING_OPERATION_GET_FRAMEBUFFER;
    data -> loadLocation = (IntegerPointer)loadLocation;
    data -> nAllocatedPages = nFramebufferPages;
    u32 result = systemCall_Messaging_MessageProcess(SCREEN_MANAGER_PID, messageDataPage);
    if (result == GENERIC_ERROR_RESULT || data -> result == GENERIC_ERROR_RESULT) {
        returnUnmappedPages(loadLocation, nFramebufferPages);
        frameBuffer = NULL;
        return;
    }

    frameBuffer = loadLocation;
    frameBuffer_nPages = nFramebufferPages;
    return;
}