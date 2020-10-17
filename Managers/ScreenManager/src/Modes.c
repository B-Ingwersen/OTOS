
#include "Modes.h"
#include "OTOSCore/CStandardLibrary/string.h"
#include "OTOSCore/CStandardLibrary/stdlib.h"

struct VideoModeInfo * videoModes = NULL;
u32 nVideoModes = 0;
struct currentVideoMode currentMode = {};

void initializeModes() {
    videoModes = calloc(MAX_VIDEO_MODES, sizeof(struct VideoModeInfo));
    currentMode.modeIndex = -1;
}

u32 setVideoMode(u32 mode) {
    u32 i = mode;
    if (i >= nVideoModes) {
        return GENERIC_ERROR_RESULT;
    }

    videoModes[i].setMode(&videoModes[i]);

    currentMode.modeIndex = i;

    IntegerPointer frameBuffer = (IntegerPointer)(videoModes[i].framebuffer);
    IntegerPointer frameBufferOffset = frameBuffer & 0xFFF;
    IntegerPointer frameBufferBase = frameBuffer - frameBufferOffset;
    IntegerPointer nPages = (frameBufferOffset + (videoModes[i].width * videoModes[i].height * videoModes[i].colorDepth) + PAGE_SIZE - 1) / PAGE_SIZE;

    currentMode.frameBuffer = (void*)frameBuffer;
    currentMode.frameBufferBase = (MemoryPage)frameBufferBase;
    currentMode.frameBufferOffset = (u32)frameBufferOffset;
    currentMode.frameBufferNPages = (u32)nPages;

    return GENERIC_SUCCESS_RESULT;
}

u32 mapFramebufferIntoProcess(PID pid, IntegerPointer loadLocation, u32 nAllocatedPages) {
    if (currentMode.frameBuffer == NULL) {
        return GENERIC_ERROR_RESULT;
    }
    if (nAllocatedPages != currentMode.frameBufferNPages) {
        return GENERIC_ERROR_RESULT;
    }

    return systemCall_Memory_MapKernelMemory(pid, (MemoryPage)loadLocation, (IntegerPointer)currentMode.frameBufferBase, currentMode.frameBufferNPages, true, SYSTEM_CALL_PROCESS_PERMISSION);
}

u32 getVideoMode(u32 type, u32 width, u32 height) {
    u32 i;
    for (i = 0; i < nVideoModes; i++) {
        if (videoModes[i].type == type && videoModes[i].width == width && videoModes[i].height == height) {
            return i;
        }
    }
    return MAX_VIDEO_MODES;
}

u32 getHighestResMode(u32 type, u32 maxWidth, u32 maxHeight) {
    u32 mode = MAX_VIDEO_MODES;

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
        }
    }
    return mode;
}