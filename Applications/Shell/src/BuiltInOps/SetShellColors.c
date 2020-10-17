
#include "BuiltInOps/SetShellColors.h"
#include "PrintFunctions.h"
#include "Output.h"
#include "Screen.h"
#include "OTOSCore/CStandardLibrary/string.h"
#include "OTOSCore/CStandardLibrary/stdlib.h"

u8 ** setShellColors_colorList = NULL;

i32 setShellColors_getColorCode(u8 * name) {
    u32 len = parseArgumentEnd(name) - name;

    int i;
    for (i = 0; i < 16; ++i) {
        if (strncmp(name, setShellColors_colorList[i], len) == 0) {
            return i;
        }
    }
    return -1;
}

void setShellColors_printColorList() {
    print_string("Allowed shell colors:\t");

    int i;
    for (i = 0; i < 16; ++i) {
        print_string(setShellColors_colorList[i]), print_char('\t');
    }
    print_char('\n');
}

void setShellColors_command(u8 * input) {
    if (setShellColors_colorList == NULL) {
        setShellColors_colorList = malloc(16 * sizeof(u8 *));
        setShellColors_colorList[0] = "black";
        setShellColors_colorList[1] = "blue";
        setShellColors_colorList[2] = "green";
        setShellColors_colorList[3] = "cyan";
        setShellColors_colorList[4] = "red";
        setShellColors_colorList[5] = "magenta";
        setShellColors_colorList[6] = "orange";
        setShellColors_colorList[7] = "lightGray";
        setShellColors_colorList[8] = "darkGray";
        setShellColors_colorList[9] = "lightBlue";
        setShellColors_colorList[10] = "lightGreen";
        setShellColors_colorList[11] = "lightCyan";
        setShellColors_colorList[12] = "lightRed";
        setShellColors_colorList[13] = "lightMagenta";
        setShellColors_colorList[14] = "yellow";
        setShellColors_colorList[15] = "white";
    }

    u8 * fgColor = parseNextArgument(input);
    if (fgColor == NULL) {
        print_string("Usage: setShellColors [foreground color] [background color] [optional: cursor color]\n");
        setShellColors_printColorList();
        return;
    }

    u8 * bgColor = parseNextArgument(fgColor);
    if (bgColor == NULL) {
        print_string("Usage: setShellColors [foreground color] [background color] [optional: cursor color]\n");
        setShellColors_printColorList();
        return;
    }

    i32 fgColorCode = setShellColors_getColorCode(fgColor);
    if (fgColorCode == -1) {
        print_string("Error: unrecognized foreground color\n");
        setShellColors_printColorList();
        return;
    }

    i32 bgColorCode = setShellColors_getColorCode(bgColor);
    if (bgColorCode == -1) {
        print_string("Error: unrecognized background color\n");
        setShellColors_printColorList();
        return;
    }

    if (bgColorCode == fgColorCode) {
        print_string("Error: foreground and background colors must be different\n");
        setShellColors_printColorList();
        return;
    }

    u8 * cursorColor = parseNextArgument(bgColor);
    if (cursorColor == NULL) {
        if (bgColorCode == 1 || bgColorCode == 9) {
            output_shellCursorColor = 0xF0;
        }
        else {
            output_shellCursorColor = 0x10;
        }
    }
    else {
        i32 cursorColorCode = setShellColors_getColorCode(cursorColor);
        if (cursorColorCode == -1) {
            print_string("Error: unrecognized cursor color\n");
            setShellColors_printColorList();
            return;
        }
        if (cursorColorCode == bgColorCode) {
            print_string("Error: cursor and background colors must be different\n");
            setShellColors_printColorList();
            return;
        }

        output_shellCursorColor = cursorColorCode << 4;
    }

    u8 newColors = (bgColorCode << 4) + fgColorCode;
    output_shellColors = newColors;
    screen -> cursorBackgroundSave = newColors;

    clearScreen();
    output_update();
}