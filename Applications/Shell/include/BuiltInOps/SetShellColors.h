#ifndef BUILT_IN_OPS___SET_SHELL_COLORS_H
#define BUILT_IN_OPS___SET_SHELL_COLORS_H

#include "OTOSCore/Definitions.h"

extern u8 ** setShellColors_colorList;

i32 setShellColors_getColorCode(u8 * name);

void setShellColors_printColorList();

void setShellColors_command(u8 * input);

#endif
