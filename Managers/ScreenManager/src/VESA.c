
#include "VESA.h"
#include "OTOSCore/SystemCalls.h"

u32 vesa_BIOSCall_GetVideoModes(MemoryPage loadTo) {
    struct SystemCall_BiosCall_Variables biosVars;
    biosVars.eax = 0x4F00;
    biosVars.edi = 0;
    biosVars.interruptNumber = 0x10;
    
    u32 result = systemCall_BiosCall(&biosVars, SYSTEM_CALL_BIOS_CALL_READ, loadTo, PAGE_SIZE, NULL, 0);
    if (result == GENERIC_ERROR_RESULT || (biosVars.eax & 0xFFFF) != 0x004F) {
        return GENERIC_ERROR_RESULT;
    }

    return GENERIC_SUCCESS_RESULT;
}

u32 vesa_BIOSCall_GetModeInfo(MemoryPage loadTo, u32 mode) {
    struct SystemCall_BiosCall_Variables biosVars;
    biosVars.eax = 0x4F01;
    biosVars.ecx = mode;
    biosVars.edi = 0;
    biosVars.interruptNumber = 0x10;

    u32 result = systemCall_BiosCall(&biosVars, SYSTEM_CALL_BIOS_CALL_READ, loadTo, 256, NULL, 0);
    if (result == GENERIC_ERROR_RESULT || (biosVars.eax & 0xFFFF) != 0x004F) {
        return GENERIC_ERROR_RESULT;
    }

    return GENERIC_SUCCESS_RESULT;
}

u32 vesa_BIOSCall_SetMode(u32 mode) {
    struct SystemCall_BiosCall_Variables biosVars;
    biosVars.eax = 0x4F02;
    biosVars.ebx = mode;
    biosVars.edi = 0;
    biosVars.interruptNumber = 0x10;

    u32 result = systemCall_BiosCall(&biosVars, SYSTEM_CALL_BIOS_CALL_READ, NULL, 0, NULL, 0);
    if (result == GENERIC_ERROR_RESULT || (biosVars.eax & 0xFFFF) != 0x004F) {
        return GENERIC_ERROR_RESULT;
    }

    return GENERIC_SUCCESS_RESULT;
}

u32 vesa_SetVideoMode(struct VideoModeInfo * mode) {
    u32 result = vesa_BIOSCall_SetMode(mode -> modeID);
    return result;
}

u32 vesa_GetVideoModes() {

    MemoryPage loadPage = getMappedPage(true);
    u32 result = vesa_BIOSCall_GetVideoModes(loadPage);
    if (result == GENERIC_ERROR_RESULT) {
        return GENERIC_ERROR_RESULT;
    }

    // get and copy modes!
        IntegerPointer vesaModesBuffer = (IntegerPointer)( *(u32*)((IntegerPointer)loadPage + 0x0E) );
        vesaModesBuffer = ( (vesaModesBuffer & 0xFFFF0000) >> 12 ) + (vesaModesBuffer & 0xFFFF);
        IntegerPointer vesaModesBuffer_Offset = vesaModesBuffer & 0xFFF;
        vesaModesBuffer -= vesaModesBuffer_Offset;

        MemoryPage vesaModes_LoadBuffer = getUnmappedPages(2);
        systemCall_Memory_MapKernelMemory(0, vesaModes_LoadBuffer, vesaModesBuffer, 2, true, SYSTEM_CALL_NO_PERMISSION);

        u16 * vesaSupportedModes = (u16*)getMappedPage(true);
        u16 * vesaSupportedModes_CopyFrom = (u16*)((IntegerPointer)vesaModes_LoadBuffer + vesaModesBuffer_Offset);
        u32 i;
        for (i = 0; i < 256; i++) {
            if (vesaSupportedModes_CopyFrom[i] == 0xFFFF) {
                break;
            }
            vesaSupportedModes[i] = vesaSupportedModes_CopyFrom[i];
        }

        u32 nVesaSupportedModes = i;

        systemCall_Memory_UnmapMemory(0, vesaModes_LoadBuffer, 2, true, SYSTEM_CALL_NO_PERMISSION);
        returnUnmappedPages(vesaModes_LoadBuffer, 2);

    int j = 0;
    for (i = nVideoModes; i < MAX_VIDEO_MODES && j < nVesaSupportedModes; j++) {
        result = vesa_BIOSCall_GetModeInfo(loadPage, vesaSupportedModes[j]);
        if (result == GENERIC_ERROR_RESULT) {
            continue;
        }
        
        struct Vesa_ModeInfo * vesaModeInfo = (struct Vesa_ModeInfo *)loadPage;
        u32 attributes = (u32)vesaModeInfo -> modeAttributes;
        if ( (attributes & 1) == 0) {  // mode not supported by current hardware
            continue;
        }

        if ( (attributes & (1 << 4)) == 0) {
            //asm volatile("jmp $"::"a"(attributes), "b"(vesaModeInfo -> windowA_StartSegment));
            IntegerPointer framebuffer = (IntegerPointer)(vesaModeInfo -> windowA_StartSegment) * 16;
            if (framebuffer == 0) {
                continue;
            }

            videoModes[i].type = VIDEO_MODE_INFO_TYPE_TEXT;
            videoModes[i].colorDepth = 2;
            videoModes[i].framebuffer = (void*)framebuffer;
        }
        else {
            if ( !(attributes & (1<<7)) ) { // no linear framebuffer
                continue;
            }
            if (vesaModeInfo -> bitsPerPixel != 32) { //need 32 bit color format
                continue;
            }

            videoModes[i].type = VIDEO_MODE_INFO_TYPE_GRAPHICS;
            videoModes[i].colorDepth = 4;
            videoModes[i].framebuffer = (void*)(vesaModeInfo -> linearVideoBufferAddress);
        }

        videoModes[i].width = (u32)vesaModeInfo -> width;
        videoModes[i].height = (u32)vesaModeInfo -> height;
        videoModes[i].modeID = (u32)vesaSupportedModes[j];
        videoModes[i].setMode = vesa_SetVideoMode;

        i++;
    }

    returnMappedPage(loadPage);
    returnMappedPage((MemoryPage)vesaSupportedModes);

    nVideoModes = i;
    return GENERIC_SUCCESS_RESULT;
}