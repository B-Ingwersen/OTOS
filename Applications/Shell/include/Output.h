
#ifndef _OUTPUT___H_
#define _OUTPUT___H_

#include "TrueType.h"
#include "ScreenManager/MessagingDefinitions.h"
#include "OTOSCore/Synchronization.h"

struct OutputScreen {
    u8 * screen; //color & text byte
    u32 height;
    u32 width;
    u32 cursorLocation;
    u32 tabSize;
    u8 cursorBackgroundSave;
    bool8 freeableScreen;
};

extern struct OutputScreen * screen;
extern struct GraphicsScreen * graphicsScreen;

extern u8 output_shellColors;
extern u8 output_shellCursorColor;
extern u32 output_shellFontSize;
extern MemoryPage graphics_loopdThread;
extern bool32 graphics_loopDataWaiting;
extern Synchronization_Spinlock graphics_loopLock;
extern Synchronization_Spinlock graphics_drawLock;
extern Synchronization_Spinlock output_lock;

extern trueTypeFontManager trueType_globalFontManager;
extern trueTypeFontDescriptor * trueType_monospaceFont;
extern trueTypeFontDescriptor * trueType_monospaceFontBold;
extern trueTypeFontRenderer * trueType_defaultFontRenderer;

u32 output_initialize();
u32 output_update();
void output_setupVideoMode(struct ScreenManager_Messaging_ModeInfo * modeInfo);
void drawGraphicsScreen();
void output_drawGraphicsLoop();
void output_resizeGraphicsText();

#endif // _OUTPUT___H_