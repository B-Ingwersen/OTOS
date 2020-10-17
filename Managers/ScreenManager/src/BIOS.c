
#include "BIOS.h"
#include "Modes.h"
#include "OTOSCore/SystemCalls.h"

u32 bios_setMode(struct VideoModeInfo * mode) {
    struct SystemCall_BiosCall_Variables biosVars;

    biosVars.eax = mode -> modeID & 0xFF;
    biosVars.interruptNumber = 0x10;

    return systemCall_BiosCall(&biosVars, SYSTEM_CALL_BIOS_CALL_RW_NONE, NULL, 0, NULL, 0);
}

u32 bios_addStandardVideoModes() {
    if (nVideoModes < MAX_VIDEO_MODES) {
        struct VideoModeInfo * newMode = &(videoModes[nVideoModes]);

        newMode -> type = VIDEO_MODE_INFO_TYPE_TEXT;
        newMode -> colorDepth = 2;
        newMode -> framebuffer = (void *)0xB8000;

        newMode -> width = 80;
        newMode -> height = 25;
        newMode -> modeID = 3;
        newMode -> setMode = bios_setMode;

        nVideoModes += 1;
    }
}