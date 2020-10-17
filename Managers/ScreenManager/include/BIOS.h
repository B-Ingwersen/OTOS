
#ifndef BIOS_H
#define BIOS_H

#include "OTOSCore/Definitions.h"
#include "Modes.h"

u32 bios_setMode(struct VideoModeInfo * mode);

u32 bios_addStandardVideoModes();

#endif // BIOS_H