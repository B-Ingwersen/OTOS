
#include "BuiltInOps/ListVideoModes.h"
#include "PrintFunctions.h"
#include "Screen.h"

void listVideoModes_command(u8 * input) {
    u32 nVideoModes;
    struct ScreenManager_Messaging_ModeInfo * videoModes = screen_GetModeList(&nVideoModes);
    if (videoModes == NULL) {
        print_string("An error occured retrieving video modes\n");
        return;
    }

    print_string("Mode #\tType\t\tWidth\tHeight\tColor Depth\n");
    u32 i;
    for (i = 0; i < nVideoModes; i++) {
        print_int(i), print_char('\t');

        if (videoModes[i].type == SCREEN_VIDEO_MODE_TYPE_TEXT) {
            print_string("Text\t\t");
        }
        else {
            print_string("Graphics\t");
        }

        print_int(videoModes[i].width), print_char('\t');
        print_int(videoModes[i].height), print_char('\t');
        print_int(videoModes[i].colorDepth), print_char('\n');
    }
}