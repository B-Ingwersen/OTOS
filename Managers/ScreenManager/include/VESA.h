
#ifndef VESA_H
#define VESA_H

#include "OTOSCore/Definitions.h"
#include "Modes.h"

struct Vesa_ModeInfo {
    u16 modeAttributes;
    u8 windowA_Attributes;
    u8 windowB_Attributes;
    u16 windowGranularity_KB;
    u16 windowSize_KB;
    u16 windowA_StartSegment;
    u16 windowB_StartSegment;
    u32 farWindowPositioningFunction;
    u16 bytesPerScanLine;

    u16 width;
    u16 height;
    u8 charWidth;
    u8 charHeight;
    u8 nMemoryPlanes;
    u8 bitsPerPixel;
    u8 nBanks;
    u8 memoryModelType;
    u8 bnakSize_KB;
    u8 nImagePanes;
    u8 reserved;

    u8 redMaskSize;
    u8 redFieldPosition;
    u8 greenMaskSize;
    u8 greenFieldPosition;
    u8 blueMaskSize;
    u8 blueFieldPosition;
    u8 reservedMaskSize;
    u8 reservedMaskPosition;
    u8 directColorModeInfo;

    u32 linearVideoBufferAddress;
    u32 pointerToOffscreenMemory;
    u16 offscreenMemorySize_KB;
} ExactBinaryStructure;

u32 vesa_BIOSCall_GetVideoModes(MemoryPage loadTo);

u32 vesa_BIOSCall_GetModeInfo(MemoryPage loadTo, u32 mode);

u32 vesa_BIOSCall_SetMode(u32 mode);

u32 vesa_SetVideoMode(struct VideoModeInfo * mode);

u32 vesa_GetVideoModes();

#endif // VESA_H