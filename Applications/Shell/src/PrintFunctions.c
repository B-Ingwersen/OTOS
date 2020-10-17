#include "PrintFunctions.h"
#include "Output.h"
#include "OTOSCore/Synchronization.h"
#include "OTOSCore/CStandardLibrary/ctype.h"
#include "OTOSCore/CStandardLibrary/string.h"

u32 getLineWidth() {
    synchronization_OpenSpinlock(&output_lock);
    u32 width = screen -> width;
    synchronization_CloseSpinlock(&output_lock);
    return width;
}

u32 setCursor(u32 location) {
    if (screen -> cursorLocation < screen -> width * screen -> height) {
        screen -> screen[2 * screen -> cursorLocation + 1] = screen -> cursorBackgroundSave;
    }

    screen -> cursorLocation = location;
    screen -> cursorBackgroundSave = screen -> screen[2 * location + 1];
    screen -> screen[2 * location + 1] = output_shellCursorColor;
}

u32 clearScreen() {
    synchronization_OpenSpinlock(&output_lock);

    u32 i;
    for (i = 0; i < screen -> width * screen -> height; i++) {
        screen -> screen[2 * i] = 0x0;
        screen -> screen[2 * i + 1] = output_shellColors;
    }
    setCursor(0);

    synchronization_CloseSpinlock(&output_lock);
    output_update();
}

void scrollScreen() {
    memmove(screen -> screen, screen -> screen + screen -> width * 2, screen -> width * (screen -> height - 1) * 2);
    //memset(screen -> screen + screen -> width * (screen -> height - 1) * 2, 0, screen -> width * 2);
    screen -> cursorLocation -= screen -> width;

    int i;
    for (i = 0; i < screen -> width; ++i) {
        screen -> screen[(screen -> width * (screen -> height - 1) + i) * 2] = 0;
        screen -> screen[(screen -> width * (screen -> height - 1) + i) * 2 + 1] = output_shellColors;
    }

    output_update();
}

void print_colorBlocks(u8 * blocks, u32 nBlocks) {
    synchronization_OpenSpinlock(&output_lock);
    
    u32 i;
    u32 cursor = screen -> cursorLocation;
    for (i = 0; i < nBlocks; i++) {
        screen -> screen[2 * cursor] = 219; // block ascii character
        screen -> screen[2 * cursor + 1] = blocks[i];
        cursor++;
    }
    if (nBlocks > 0) {
        screen -> cursorBackgroundSave = blocks[0];
    }
    setCursor(cursor);

    synchronization_CloseSpinlock(&output_lock);
    output_update();
}

void print_char(u8 c) {
    synchronization_OpenSpinlock(&output_lock);

    u32 cursor = screen -> cursorLocation;
    u32 maxSize = screen -> width * screen -> height;

    if (c == '\n') {
        cursor += screen -> width - (cursor % screen -> width);
    }
    else if (c == '\t') {
        u32 linePosition = cursor % screen -> width;
        cursor += screen -> tabSize - (linePosition % screen -> tabSize);
    }
    else {
        screen -> screen[2 * cursor] = c;
        screen -> screen[2 * cursor + 1] = output_shellColors;
        cursor++;
    }
    if (cursor >= maxSize) {
        scrollScreen();
        cursor -= screen -> width;
    }

    screen -> cursorBackgroundSave = output_shellColors;
    setCursor(cursor);

    synchronization_CloseSpinlock(&output_lock);
    output_update();
}

void print_chars(u8 * text, u32 len) {
    synchronization_OpenSpinlock(&output_lock);

    u32 cursor = screen -> cursorLocation;
    u32 maxSize = screen -> width * screen -> height;
    u32 i; for (i = 0; i < len; ++i) {
        u8 c = *text;
        if (c == '\n') {
            cursor += screen -> width - (cursor % screen -> width);
        }
        else if (c == '\t') {
            u32 linePosition = cursor % screen -> width;
            cursor += screen -> tabSize - (linePosition % screen -> tabSize);
        }
        else {
            screen -> screen[2 * cursor] = c;
            screen -> screen[2 * cursor + 1] = output_shellColors;
            cursor++;
        }
        text++;
        if (cursor >= maxSize) {
            scrollScreen();
            cursor -= screen -> width;
        }
    }
    screen -> cursorBackgroundSave = output_shellColors;
    setCursor(cursor);

    synchronization_CloseSpinlock(&output_lock);
    output_update();
}

void print_string(u8 * text) {
    print_chars(text, strlen(text));
}

void print_int(i32 number) {
    if (number < 0) {
        print_char('-');
        number = -number;
    }

    if (number / 10 != 0) {
        print_int(number / 10);
    }
    print_char(number % 10 + '0');

    output_update();
}

u8 * parseNextArgument(u8 * text) {
    while (!isspace(*text)) {
        if (*text == '\0') {
            return NULL;
        }
        ++text;
    }

    while (isspace(*text)) {
        ++text;
    }

    if (*text == '\0') {
        return NULL;
    }

    return text;
}

u8 * parseArgumentEnd(u8 * text) {
    while (!isspace(*text) && *text != '\0') {
        ++text;
    }
    return text;
}

bool8 parseInteger(u8 * text, i32 * returnVal) {
    i32 val = 0;
    while (isdigit(*text)) {
        val *= 10;
        val += (i32)(*text - '0');
        ++text;
    }

    if (!(isspace(*text) || *text == '\0')) {
        return false;
    }

    *returnVal = val;
    return true;
}