#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "OTOSCore/Definitions.h"

struct GraphicsScreen {
    i32 width;
    i32 height;
    u8 colorDepth;
    void * screen;
};

struct WindowSection {
    i32 X1;
    i32 X2;
    i32 Y1;
    i32 Y2;
    i32 x;      // true origin location, can be out of bounds
    i32 y;      // true origin location, can be out of bounds
};

extern u8 * graphics_characterData;
extern u32 graphics_characterDataNPages;

void drawRectangle(struct GraphicsScreen * s, struct WindowSection * window, i32 x, i32 y, i32 w, i32 h, u32 color);

void drawHorizontalShearLine( struct GraphicsScreen * s, struct WindowSection * window, i32 x, i32 y, i32 w, i32 sWidth, i32 sHeight, u32 color);

void drawBasicCharacter( struct GraphicsScreen * s, struct WindowSection * window, i8 * charInfo, u32 x, u32 y, i32 size, u32 color);

void graphics_loadCharacterData(u8 * filePath);

void graphics_memSetD(u32 * start, u32 color, u32 size);

#endif