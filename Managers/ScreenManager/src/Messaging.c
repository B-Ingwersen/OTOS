
#include "OTOSCore/SystemCalls.h"
#include "ScreenManager/MessagingDefinitions.h"
#include "Messaging.h"
#include "Modes.h"

void messaging_DefaultMessageHandler(PID pid, MemoryPage dataPage) {
    void * data = (void *) ( ((IntegerPointer)dataPage) + SYSTEM_CALL_MESSAGING_DATA_OFFSET );
    u32 operation = *(u32*)data;

    if (operation == SCREEN_MANAGER_MESSAGING_OPERATION_TEST) {
        messaging_Test(pid, data);
    }
    else if (operation == SCREEN_MANAGER_MESSAGING_OPERATION_GET_MODE_LIST) {
        messaging_GetModeList(pid, data);
    }
    else if (operation == SCREEN_MANAGER_MESSAGING_OPERATION_SET_MODE) {
        messaging_SetMode(pid, data);
    }
    else if (operation == SCREEN_MANAGER_MESSAGING_OPERATION_GET_FRAMEBUFFER) {
        messaging_GetFrameBuffer(pid, data);
    }

    systemCall_Messaging_MessageReturn();
}

void messaging_Test(PID pid, void * dataPointer) {
    struct ScreenManager_Messaging_Test * data = (struct ScreenManager_Messaging_Test *)dataPointer;
    data -> result = GENERIC_SUCCESS_RESULT;
}

void messaging_GetModeList(PID pid, void * dataPointer) {
    struct ScreenManager_Messaging_GetModeList * data = (struct ScreenManager_Messaging_GetModeList *)dataPointer;
    
    data -> nModes = nVideoModes;
    u32 i;
    for (i = 0; i < nVideoModes; i++) {
        data -> modeList[i].type = videoModes[i].type;
        data -> modeList[i].width = videoModes[i].width;
        data -> modeList[i].height = videoModes[i].height;
        data -> modeList[i].colorDepth = videoModes[i].colorDepth;

        IntegerPointer frameBuffer = (IntegerPointer)(videoModes[i].framebuffer);
        IntegerPointer frameBufferOffset = frameBuffer & 0xFFF;
        IntegerPointer frameBufferBase = frameBuffer - frameBufferOffset;
        IntegerPointer nPages = (frameBufferOffset + (videoModes[i].width * videoModes[i].height * videoModes[i].colorDepth) + PAGE_SIZE - 1) / PAGE_SIZE;

        data -> modeList[i].frameBufferNPages = nPages;
    }

    data -> result = GENERIC_SUCCESS_RESULT;
}

void messaging_SetMode(PID pid, void * dataPointer) {
    struct ScreenManager_Messaging_SetMode * data = (struct ScreenManager_Messaging_SetMode *)dataPointer;

    data -> result = setVideoMode(data -> mode);
}

void messaging_GetFrameBuffer(PID pid, void * dataPointer) {
    struct ScreenManager_Messaging_GetFramebuffer * data = (struct ScreenManager_Messaging_GetFramebuffer *)dataPointer;

    data -> result = mapFramebufferIntoProcess(pid, data -> loadLocation, data -> nAllocatedPages);
}