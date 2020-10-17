
#include "BuiltInOps/SetShellFont.h"
#include "PrintFunctions.h"
#include "TrueType.h"
#include "Output.h"
#include "OTOSCore/CStandardLibrary/string.h"

void setShellFont_command(u8 * input) {
    i32 fontSize;
    input = parseNextArgument(input);
    if (input == NULL || !parseInteger(input, &fontSize)) {
        print_string("Please specify a fontSize\n");
        return;
    }

    if (fontSize < 12 || fontSize > 50) {
        print_string("Error: valid font sizes are 12 - 50\n");
        return;
    }

    u8 * fontType = parseNextArgument(input);
    trueTypeFontDescriptor * font = trueType_defaultFontRenderer -> descriptor;
    if (fontType != NULL) {
        u32 len = parseArgumentEnd(fontType) - fontType;
        if (strncmp(fontType, "regular", len) == 0) {
            font = trueType_monospaceFont;
        }
        else if (strncmp(fontType, "bold", len) == 0) {
            font = trueType_monospaceFontBold;
        }
        else {
            print_string("Error: unrecognized font type (options are regular and bold)\n");
        }
    }

    synchronization_OpenSpinlock(&graphics_drawLock);
    synchronization_OpenSpinlock(&output_lock);

    if (graphicsScreen == NULL) {
        synchronization_CloseSpinlock(&output_lock);
        synchronization_CloseSpinlock(&graphics_drawLock);
        print_string("Error: cannot sent font size in text mode shell\n");
        return;
    }

    closeTrueTypeRenderer(trueType_defaultFontRenderer);
    output_shellFontSize = fontSize;
    trueType_defaultFontRenderer = openTrueTypeRenderer(font, (float)output_shellFontSize);
    output_resizeGraphicsText();

    //asm volatile("jmp $" :: "a" (0x69), "b" (trueType_monospaceFontBold));

    synchronization_CloseSpinlock(&output_lock);
    synchronization_CloseSpinlock(&graphics_drawLock);

    clearScreen();
    output_update();
}