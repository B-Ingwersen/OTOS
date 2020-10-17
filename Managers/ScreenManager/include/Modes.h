
#ifndef MODES_H
#define MODES_H

#include "OTOSCore/Definitions.h"
#include "OTOSCore/SystemCalls.h"
#include "OTOSCore/Synchronization.h"
#include "OTOSCore/MemoryAllocation.h"
#include "OTOSCore/SharedMemory.h"

#define MAX_VIDEO_MODES 64
#define VIDEO_MODE_INFO_TYPE_TEXT 1
#define VIDEO_MODE_INFO_TYPE_GRAPHICS 2

struct VideoModeInfo {
    u32 type;
    u32 width;
    u32 height;
    u32 colorDepth;

    void * framebuffer;

    u32 (*setMode)(struct VideoModeInfo * mode);
    u32 modeID;
};

struct currentVideoMode {
    u32 modeIndex;

    void * frameBuffer;
    u32 frameBufferOffset;
    MemoryPage frameBufferBase;
    u32 frameBufferNPages;

    SharedMemory_SegmentID segment;
};


extern struct VideoModeInfo * videoModes;
extern u32 nVideoModes;
extern struct currentVideoMode currentMode;

void initializeModes();

u32 setVideoMode(u32 mode);

u32 mapFramebufferIntoProcess(PID pid, IntegerPointer loadLocation, u32 nAllocatedPages);

u32 getVideoMode(u32 type, u32 width, u32 height);

u32 getHighestResMode(u32 type, u32 maxWidth, u32 maxHeight);

#endif // MODES_H