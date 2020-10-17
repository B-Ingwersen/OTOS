
#include "BuiltInOps/SetVideoMode.h"
#include "PrintFunctions.h"
#include "Output.h"
#include "Screen.h"

void setVideoMode_command(u8 * input) {
    i32 modeNumber;
    input = parseNextArgument(input);
    if (input == NULL || !parseInteger(input, &modeNumber)) {
        print_string("Please specify a mode number\n");
        return;
    }
    
    struct ScreenManager_Messaging_ModeInfo modeInfo;
    if (!(screen_getModeInfo(modeNumber, &modeInfo))) {
        print_string("Error: invalid mode number\n");
        return;
    }
    screen_SetVideoMode(modeNumber);
    output_setupVideoMode(&modeInfo);
}