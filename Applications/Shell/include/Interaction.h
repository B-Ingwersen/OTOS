#ifndef INTERACTION_H
#define INTERACTION_H

#include "OTOSCore/Definitions.h"

#define INPUT_SIZE_MAX 2048
#define INPUT_BACKSPACE 1
#define INPUT_LEFT 2
#define INPUT_RIGHT 3

extern u32 inputCursorLoc;
extern u8 * inputBuffer;
extern u32 inputCursor;
extern u32 inputSize ;
extern u8 * currentDir;

extern bool32 shiftDown;
extern bool32 ctrlDown;

extern  u8 keyboardScanCodeTable[];

extern u8 capitalsScanodeTable[];

struct BuiltinList {
    u8 * name;
    void (*function)(u8 * input);
};

extern struct BuiltinList builtinList[];

void interaction_initialize();
void addToInputBuffer(u8 c);
void prompt();
void processCommand(u8 * input);
void processKey(u32 keycode, bool32 release);
bool8 processPath(u8 * path, u32 pathLen, u8 * pathOut, u32 * pathOutSize);

#endif