
#include "BuiltInOps/TigerLogo.h"
#include "PrintFunctions.h"
#include "Screen.h"
#include "OTOSCore/CStandardLibrary/string.h"

u8 tigerLogo_data[] = {
    0x08,0x88,0x88, 0x00,0x00,0x00,
	0x08,0xff,0xf8, 0x00,0x66,0x66,
	0x08,0xff,0xf8, 0x66,0x60,0x00,

	0x06,0x66,0x66, 0x60,0x06,0x66,
	0x66,0x6f,0xff, 0xff,0x66,0x00,
	0x60,0x66,0xee, 0xef,0xf0,0x66,

	0x60,0x66,0xee, 0x0e,0x04,0x66,
	0x70,0x66,0x66, 0x66,0x04,0x66,
	0x77,0x00,0x66, 0x66,0x46,0x66,

	0x07,0x77,0x00, 0x64,0x66,0x66,
	0x00,0x77,0x77, 0x74,0x66,0x66,
	0x00,0x00,0x00, 0xff,0xcc,0xcc,

	0x00,0x00,0x00, 0xff,0xf8,0x8c,
	0x00,0x00,0x00, 0x0f,0xff,0x8c
};

void tigerLogo() {
    i32 preceedingWhiteSpace = (getLineWidth() - 24) / 2;
    if (preceedingWhiteSpace < 0) {
        preceedingWhiteSpace = 0;
    }

    u32 row;
    for (row = 0; row < 14; row++) {
        u8 rowData[24];
        u32 i;
        for (i = 0; i < 6; i++) {
            u8 color1 = tigerLogo_data[6 * row + i];
            u8 color2 = color1 & 0xF;
            color1 >>= 4;

            rowData[2 * i] = color1;
            rowData[2 * i + 1] = color2;
            rowData[23 - (2 * i + 1)] = color2;
            rowData[23 - (2 * i)] = color1;
        }

        for (i = 0; i < preceedingWhiteSpace; i++) {
            print_string(" ");
        }

        print_colorBlocks(rowData, 24);
        print_string("\n");
    }
}

void tigerLogo_centeredText(u8 * text) {
    i32 preceedingWhiteSpace = (getLineWidth() - strlen(text)) / 2;
    if (preceedingWhiteSpace < 0) {
        preceedingWhiteSpace = 0;
    }

    u32 i;
    for (i = 0; i < preceedingWhiteSpace; i++) {
        print_string(" ");
    }
    print_string(text);
}

void tigerLogo_welcome() {
    clearScreen();
    print_string("\n\n");
    tigerLogo_centeredText("Welcome to OrangeTiger OS!");
    print_string("\n\n\n");
    tigerLogo();
    print_string("\n\n\n");
    tigerLogo_centeredText("Version 0.2.0");
    print_string("\n\n");
}

void tigerLogo_command(u8 * input) {
    tigerLogo();
}


