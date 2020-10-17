#include "Graphics.h"
#include "OTOSCore/Disk.h"
#include "OTOSCore/CStandardLibrary/string.h"

u8 * graphics_characterData = NULL;
u32 graphics_characterDataNPages = 0;

void drawRectangle(struct GraphicsScreen * s, struct WindowSection * window, i32 x, i32 y, i32 w, i32 h, u32 color) {
    u32 * pixels = (u32*)(s -> screen);
	i32 WINDOW_WIDTH = s -> width;

    x += window -> x;
    y += window -> y;
    w += x;
    h += y;

    if (x < window -> X1) {x = window -> X1;}
    if (y < window -> Y1) {x = window -> Y1;}
    if (w > window -> X2) {w = window -> X2;}
    if (h > window -> Y2) {h = window -> Y2;}

    i32 memStart = y * WINDOW_WIDTH + x;
	i32 memWidth = w - x;
    if (memWidth <= 0) {
        return;
    }

    i32 j;
    i32 jMax = h - y;
    for ( j = 0; j < jMax; j++ ) {
		graphics_memSetD( pixels + memStart, color, memWidth );
		memStart += WINDOW_WIDTH;
	}
}

void drawHorizontalShearLine( struct GraphicsScreen * s, struct WindowSection * window, i32 x, i32 y, i32 w, i32 sWidth, i32 sHeight, u32 color ) {
    u32 * pixels = (u32*)(s -> screen);
	i32 WINDOW_WIDTH = s -> width;  

    i32 x1, x2;
    i32 memStart, memWidth;

	i32 j = 0;
	i32 yMax = sHeight;
	if (y < window -> Y1) {j = window ->Y1 - y;}
	if (y + sHeight >= window -> Y2) {yMax = window -> Y2 - y; }

	while ( j < yMax ) {
		x1 = x + (j * sWidth) / sHeight;// + ((j * width) % height)/(height/2);
		x2 = x1 + w;
		if (x1 < window -> X1) {x1 = window -> X1;}
		if (x2 > window -> X2) {x2 = window -> X2;}
		memStart = WINDOW_WIDTH * (y + j) + x1;
        if (x2 > x1) {
            memWidth = x2 - x1;
            graphics_memSetD( pixels + memStart, color, memWidth );
        }
		j++;
	}
}

// draw a basic text character in graphics mode;
void drawBasicCharacter( struct GraphicsScreen * s, struct WindowSection * window, i8 * charInfo, u32 x, u32 y, i32 size, u32 color ) {
	int index = 0;
	while (index < 100) {
		if (!charInfo[index]) {break;}

        i32 X = size * ((i32)charInfo[index + 1]) / 10;
        i32 Y = size * ((i32)charInfo[index + 2]) / 10;
        i32 W = size * ((i32)(charInfo[index + 3] + charInfo[index + 1])) / 10 - X;
        i32 H = size * ((i32)(charInfo[index + 4] + charInfo[index + 2])) / 10 - Y;
        i32 S = size / 10;
        if (size % 10 >= 5) {S++;}
        X += x;
        Y += y;
		if (charInfo[index] == 1) {
            if (charInfo[index + 3] == 1) {
                W = S;
            }
            if (charInfo[index + 4] == 1) {
                H = S;
            }
            drawRectangle(s, window, X, Y, W, H, color);
		}
		else if (charInfo[index] == 2) {
            drawHorizontalShearLine(s, window, X, Y, S, W, H, color);
		}
		index += 5;
	}
}

void graphics_loadCharacterData(u8 * filePath) {
    u64 fileSize;
    graphics_characterData = disk_ReadFullFileToBuffer(filePath, strlen(filePath), 0x400000, &fileSize, &graphics_characterDataNPages);
}

void graphics_memSetD(u32 * start, u32 color, u32 size) {
    int i;
    for (i = 0; i < size; ++i) {
        start[i] = color;
    }
}