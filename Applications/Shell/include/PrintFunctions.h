#ifndef PRINT_FUNCTIONS_H
#define PRINT_FUNCTIONS_H

#include "OTOSCore/Definitions.h"

u32 getLineWidth();
u32 setCursor(u32 location);
u32 clearScreen();
void scrollScreen();
void print_colorBlocks(u8 * blocks, u32 nBlocks);
void print_char(u8 c);
void print_chars(u8 * text, u32 len);
void print_string(u8 * text);
void print_int(i32 number);

u8 * parseNextArgument(u8 * text);
u8 * parseArgumentEnd(u8 * text);
bool8 parseInteger(u8 * text, i32 * returnVal);

#endif // _PRINT_FUNCTIONS___H_