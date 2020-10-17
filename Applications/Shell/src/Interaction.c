#include "Interaction.h"
#include "Keyboard.h"
#include "PrintFunctions.h"
#include "Output.h"

#include "BuiltInOps/TigerLogo.h"
#include "BuiltInOps/Clear.h"
#include "BuiltInOps/ListVideoModes.h"
#include "BuiltInOps/SetVideoMode.h"
#include "BuiltInOps/SetShellColors.h"
#include "BuiltInOps/SetShellFont.h"
#include "BuiltInOps/Ls.h"
#include "BuiltInOps/Cd.h"
#include "BuiltInOps/ViewFile.h"

#include "OTOSCore/CStandardLibrary/stdlib.h"
#include "OTOSCore/CStandardLibrary/string.h"

u32 inputCursorLoc = 0;
u8 * inputBuffer = NULL;
u32 inputCursor = 0;
u32 inputSize = 0;
u8 * currentDir = NULL;

bool32 shiftDown = false;
bool32 ctrlDown = false;

struct BuiltinList builtinList[10] = {};

u8 keyboardScanCodeTable[] = {
    0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0x09,'`',0,
    0,0,0,0, 0,'q','1',0, 0,0,'z','s', 'a','w','2',0,
    0,'c','x','d', 'e','4','3',0, 0,' ','v','f', 't','r','5',0,
    0,'n','b','h', 'g','y','6',0, 0,0,'m','j', 'u','7','8',0,
    0,',','k','i', 'o','0','9',0, 0,'.','/','l', 0x3B,'p','-',0,
    0,0,0x27,0, 0x5B,'=',0,0, 0,0,0x0A,0x5D, 0,0x2F,0,0,
    0,0,0,0, 0,0,0x08,0, 0,'1',0,'4', '7',0,0,0,
    '0','.','2','5', '6','8',0,0, 0,'+','3','-', '*','9',0,0,
    0,0,0,0
};

u8 capitalsScanodeTable[] = {  //0x14 - 0x5D
    'Q','!',0, 0,0,'Z','S', 'A','W','@',0,
    0,'C','X','D', 'E','$','#',0, 0,' ','V','F', 'T','R','%',0,
    0,'N','B','H', 'G','Y','^',0, 0,0,'M','J', 'U','&','*',0,
    0,'<','K','I', 'O',')','(',0, 0,'>','?','L', ':','P','_',0,
    0,0,0x22,0, 0x7B,'+',0,0, 0,0,0x0D,0x7D, 0,'|'
};

void interaction_initialize() {
    currentDir = malloc(2048);
    inputBuffer = malloc(2048);
    strcpy(currentDir, "/");

    builtinList[0].function = tigerLogo_command;
    builtinList[0].name = "tigerLogo";
    builtinList[1].function = clear_command;
    builtinList[1].name = "clear";
    builtinList[2].function = listVideoModes_command;
    builtinList[2].name = "listVideoModes";
    builtinList[3].function = setVideoMode_command;
    builtinList[3].name = "setVideoMode";
    builtinList[4].function = setShellColors_command;
    builtinList[4].name = "setShellColors";
    builtinList[5].function = setShellFont_command;
    builtinList[5].name = "setShellFont";
    builtinList[6].function = ls_command;
    builtinList[6].name = "ls";
    builtinList[7].function = cd_command;
    builtinList[7].name = "cd";
    builtinList[8].function = viewFile_command;
    builtinList[8].name = "viewFile";

    getKeyboardForward(processKey);

    clearScreen();
    tigerLogo_welcome();
    prompt();
}

void addToInputBuffer(u8 c) {
    if (inputSize >= INPUT_SIZE_MAX - 1) {
        return;
    }

    i32 inputSizeChange = 0;

    if (c == INPUT_LEFT) {
        if (inputCursor > 0) {
            inputCursor--;
        }
    }
    else if (c == INPUT_RIGHT) {
        if (inputCursor < inputSize) {
            inputCursor++;
        }
    }
    else if (c == INPUT_BACKSPACE) {
        if (inputCursor <= 0) {
            return;
        }
        memmove(inputBuffer + inputCursor, inputBuffer + inputCursor - 1, inputSize - inputCursor);
        inputCursor--;
        inputSizeChange = -1;
        inputBuffer[inputSize - 1] = ' ';
    }
    else {
        memmove(inputBuffer + inputCursor + 1, inputBuffer + inputCursor, inputSize - inputCursor);
        inputBuffer[inputCursor] = c;
        inputCursor++;
        inputSize++;
        inputBuffer[inputSize] = 0;
    }

    setCursor(inputCursorLoc);
    print_string(inputBuffer);
    inputCursorLoc = screen -> cursorLocation - inputSize;
    setCursor(inputCursorLoc + inputCursor);
    inputSize += inputSizeChange;
    inputBuffer[inputSize] = 0;
}

void prompt() {
    print_string(currentDir);
    print_string(">");

    inputBuffer[0] = 0;
    inputCursor = 0;
    inputSize = 0;
    inputCursorLoc = screen -> cursorLocation;
}

void processCommand(u8 * input) {
    u32 i, n;
    for (n = 0; input[n] != ' ' && input[n] != 0; n++) {}
    if (n == 0) {
        return;
    }
    
    for (i = 0; builtinList[i].function != NULL; i++) {
        if (strncmp(builtinList[i].name, input, n) == 0) {
            builtinList[i].function(input);
            return;
        }
    }
    print_string("Error: command not found\n");
}

void processKey(u32 keycode, bool32 release) {
    if ( (keycode == 0x12) || (keycode == 0x59) ) {
        if (release) {shiftDown = false;}
        else {shiftDown = true;}
    }


    if (release) {

    }
    else {
        if (keycode == 0x5A) {
            setCursor(inputCursorLoc + inputCursor);
            print_string("\n");
            processCommand(inputBuffer);
            prompt();
            return;
        }
        else if (keycode == 0x0D) {
            return;
        }


        u8 c = 0;
        if (keycode == 0x66) {
            c = INPUT_BACKSPACE;
        }
        else if (keycode == 0x66) {
            c = INPUT_BACKSPACE;
        }
        else if (shiftDown && (keycode >= 0x15) && (keycode <= 0x5D) ) {
            c = capitalsScanodeTable[keycode - 0x15];
        }
        else if (keycode < 132) {
            c = keyboardScanCodeTable[keycode];
        }

        if (c != 0) {
            u8 string[2] = {c, 0};
            //print_string(string);
            addToInputBuffer(c);
        }
    }
}

/* process a path into a simplified and absolute format:
    -convert relative paths against "currentDir" into absolute paths
    -simplify "." and ".." in file name
the processed path is built in "pathOut"; pathOutSize should contain the size of
the pathOut buffer, and it will be overwritten with the space actually used by
the processed path; true is returned if the operation is successful, false if
an error occurs (malformed path, buffer size inadequate)
*/
bool8 processPath(u8 * path, u32 pathLen, u8 * pathOut, u32 * pathOutSize) {
    u32 pathOutLen;
    
    // set up absolute paths
    if (path[0] == '/') {
        if (*pathOutSize <= 1) {
            return false;
        }

        pathOut[0] = '/';
        pathOutLen = 1;
        ++path, --pathLen;
    }

    // copy the current directory into path out for relative paths
    else {
        u32 currentDirLen = strlen(currentDir);
        if (currentDirLen + 1 >= *pathOutSize) {
            return false;
        }

        strcpy(pathOut, currentDir);
        pathOutLen = currentDirLen;
    }

    while (true) {
        if (pathLen == 0) {
            break;
        }

        // add a trailing slash if needed
        if (pathOut[pathOutLen - 1] != '/') {
            if (*pathOutSize <= pathOutLen + 1) {
                return false;
            }
            pathOut[pathOutLen] = '/'; ++pathOutLen;
        }

        // get name length
        u32 i = 0;
        while (path[i] != '/' && i < pathLen) {++i;}

        if (i == 1 && path[0] == '.') {}
        else if (i == 2 && path[0] == '.' && path[1] == '.') {
            if (pathOutLen > 0 && pathOut[pathOutLen - 1] == '/') {
                pathOutLen--;
            }

            while (pathOutLen > 0 && pathOut[pathOutLen - 1] != '/') {
                pathOutLen--;
            }

            if (pathOutLen == 0) {
                return false;
            }

            if (pathOutLen > 1 && pathOut[pathOutLen - 1] == '/') {
                pathOutLen--;
            }
        }
        else {
            if (*pathOutSize <= pathOutLen + i + 1) {
                return false;
            }
            memcpy(pathOut + pathOutLen, path, i);
            pathOutLen += i;
        }

        // advance position in path, moving past next slash if needed
        path += i; pathLen -= i;
        if (pathLen > 0 && pathOut[0] == '/') {
            ++path; --pathLen;
        }
    }

    pathOut[pathOutLen] = 0;
    *pathOutSize = pathOutLen;
    return true;
}