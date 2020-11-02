#include "Output.h"
#include "Screen.h"
#include "Graphics.h"
#include "TrueType.h"
#include "OTOSCore/Threads.h"
#include "OTOSCore/CStandardLibrary/math.h"
#include "OTOSCore/CStandardLibrary/stdlib.h"
#include "OTOSCore/SystemCalls.h"

struct OutputScreen * screen = NULL;
struct OutputScreen * screen_lastDrawn = NULL;
struct GraphicsScreen * graphicsScreen = NULL;
MemoryPage graphics_loopdThread = NULL;
bool32 graphics_loopDataWaiting = false;
Synchronization_Spinlock graphics_loopLock = 0;
Synchronization_Spinlock graphics_drawLock = 0;
u8 output_shellColors = 0x0F;
u8 output_shellCursorColor = 0x10;
u32 output_shellFontSize = 20;

Synchronization_Spinlock output_lock = 0;

trueTypeFontManager trueType_globalFontManager = {NULL};
trueTypeFontDescriptor * trueType_monospaceFont = NULL;
trueTypeFontDescriptor * trueType_monospaceFontBold = NULL;
trueTypeFontRenderer * trueType_defaultFontRenderer = NULL;

u32 output_initialize() {
    // allocate screen and its mirror
    screen = malloc(sizeof(struct OutputScreen));
    screen -> freeableScreen = false;
    screen -> screen = NULL;
    screen_lastDrawn = malloc(sizeof(struct OutputScreen));
    screen_lastDrawn -> screen = NULL;
    screen -> freeableScreen = true;

    // load the default fonts
    graphics_loadCharacterData("/Resources/Fonts/BasicText/textInfo.data");
    trueType_monospaceFont = openTrueTypeFont(&trueType_globalFontManager, "/Resources/Fonts/TrueType/Liberation/Mono-Regular.ttf");
    trueType_monospaceFontBold = openTrueTypeFont(&trueType_globalFontManager, "/Resources/Fonts/TrueType/Liberation/Mono-Bold.ttf");
    trueType_defaultFontRenderer = openTrueTypeRenderer(trueType_monospaceFont, (float)output_shellFontSize);
    
    synchronization_InitializeSpinlock(&graphics_loopLock);
    synchronization_InitializeSpinlock(&graphics_drawLock);

    struct ScreenManager_Messaging_ModeInfo modeInfo;
    //u32 mode = screen_GetHighestResMode(SCREEN_VIDEO_MODE_TYPE_GRAPHICS, 1500, 900, &modeInfo);
    u32 mode = screen_GetHighestResMode(SCREEN_VIDEO_MODE_TYPE_TEXT, 80, 25, &modeInfo);
    screen_SetVideoMode(mode);

    //screen_SetVideoMode(3);

    output_setupVideoMode(&modeInfo);

    // setup loop thread
    MemoryPage threadPages = getUnmappedPages(2);
    systemCall_Memory_MapNewMemory(0, threadPages, 1, true, SYSTEM_CALL_NO_PERMISSION);
    MemoryPage tStack = threadPages + PAGE_SIZE - sizeof(void *);
    graphics_loopdThread = threadPages + PAGE_SIZE;
    systemCall_ProcessThread_CreateThread(0, graphics_loopdThread, output_drawGraphicsLoop, tStack, SYSTEM_CALL_NO_PERMISSION);
    systemCall_ProcessThread_ChangeThreadExecution(0, graphics_loopdThread, SYSTEM_CALL_CHANGE_THREAD_EXECUTION_SCHEDULE, SYSTEM_CALL_NO_PERMISSION);
    graphics_loopDataWaiting = false;
}

void output_setupVideoMode(struct ScreenManager_Messaging_ModeInfo * modeInfo) {
    synchronization_OpenSpinlock(&graphics_drawLock);
    synchronization_OpenSpinlock(&output_lock);

    screen_GetFramebuffer(modeInfo -> frameBufferNPages);
    /*memset(frameBuffer, 80, frameBuffer_nPages * 4096);
    fontCharacterCache * f = trueType_renderCharacter('@', trueType_defaultFontRenderer);
    ScreenData screenData = {frameBuffer, modeInfo -> width, modeInfo -> height};
    trueType_drawCharacter(f, 20, 20, &screenData, 0xFF);
    asm volatile("jmp $" :: "a"(0xBABAB));*/
    
    // initialize screen and free attached resources
    screen -> tabSize = 8;
    screen -> cursorBackgroundSave = output_shellColors;
    screen -> cursorLocation = 0;
    if (screen -> freeableScreen && screen -> screen != NULL) {
        free(screen -> screen);
        screen -> screen = NULL;
    }

    // free resources attached to screen
    if (screen_lastDrawn -> freeableScreen && screen_lastDrawn -> screen != NULL) {
        free(screen_lastDrawn -> screen);
        screen_lastDrawn -> screen = NULL;
    }

    if (modeInfo -> type == SCREEN_VIDEO_MODE_TYPE_GRAPHICS) {
        if (graphicsScreen == NULL) {
            graphicsScreen = malloc(sizeof(struct GraphicsScreen));
        }
        graphicsScreen -> width = modeInfo -> width;
        graphicsScreen -> height = modeInfo -> height;
        graphicsScreen -> colorDepth = modeInfo -> colorDepth;
        graphicsScreen -> screen = frameBuffer;

        output_resizeGraphicsText();
    }
    else {
        screen -> screen = (u8*)frameBuffer;
        screen -> freeableScreen = false;
        screen -> height = modeInfo -> height;
        screen -> width = modeInfo -> width;

        if (graphicsScreen != NULL) {
            free(graphicsScreen);
            graphicsScreen = NULL;
        }

        screen_lastDrawn -> screen = (u8*)malloc(modeInfo -> width * modeInfo -> height * 2);
        screen_lastDrawn -> width = modeInfo -> width;
        screen_lastDrawn -> height = modeInfo -> height;
        int i; for (i = 0; i < modeInfo -> width * modeInfo -> height; ++i) {
            screen_lastDrawn -> screen[2 * i] = 255;
            screen_lastDrawn -> screen[2 * i + 1] = output_shellColors;
        }
    }

    synchronization_CloseSpinlock(&output_lock);
    synchronization_CloseSpinlock(&graphics_drawLock);
}

void output_resizeGraphicsText() {
    i32 charWidth = (int)ceil(trueType_defaultFontRenderer -> cache[' '] -> advanceWidth);
    i32 textWidth = graphicsScreen -> width / charWidth;
    i32 textHeight = graphicsScreen -> height / output_shellFontSize;

    // initialize blank screens
    screen -> screen = (u8*)malloc(textWidth * textHeight * 2);
    screen -> freeableScreen = true;
    screen_lastDrawn -> screen = (u8*)malloc(textWidth * textHeight * 2);
    int i; for (i = 0; i < textHeight * textWidth; ++i) {
        screen -> screen[2 * i] = 0;
        screen -> screen[2 * i + 1] = output_shellColors;
        screen_lastDrawn -> screen[2 * i] = 255;
        screen_lastDrawn -> screen[2 * i + 1] = output_shellColors;
    }

    screen -> width = textWidth;
    screen -> height = textHeight;
    screen_lastDrawn -> width = textWidth;
    screen_lastDrawn -> height = textHeight;
}

void drawGraphicsScreen() {
    u32 * charTable = (u32*)graphics_characterData;
    struct WindowSection windowSection = {
        0, graphicsScreen -> width, 0, graphicsScreen -> height, 0, 0
    };
    ScreenData screenData = {graphicsScreen -> screen, graphicsScreen -> width, graphicsScreen -> height};

    i32 charWidth = graphicsScreen -> width / screen -> width;
    i32 charHeight = graphicsScreen -> height / screen -> height;

    u8 * textScreenPtr = screen -> screen;
    u8 * lastDrawnPtr = screen_lastDrawn -> screen;
    const u32 emulatedTextScreenColors[] = {
        0x00, 0xaa, 0xaa00, 0xaaaa, 0xaa0000, 0xaa55aa, 0xFF7000, 0xaaaaaa,
        0x555555, 0x5555ff, 0x55ff55, 0x55ffff, 0xff5555, 0xff55ff, 0xffff55, 0xFFFFFF
    };

    i32 i,j;
    for (j = 0; j < screen -> height; ++j) {
        for (i = 0; i < screen -> width; ++i) {
            i32 x = charWidth * i;
            i32 y = charHeight * j;

            u8 c = textScreenPtr[0];
            u8 color = textScreenPtr[1];
            if (c != lastDrawnPtr[0] || color != lastDrawnPtr[1]) {
                u32 textColor = emulatedTextScreenColors[color & 0xF];
                u32 backgroundColor = emulatedTextScreenColors[color / 16];

                if (c == 0xDB) {
                    backgroundColor = textColor;
                }
                drawRectangle(graphicsScreen, &windowSection, x, y, charWidth, charHeight, backgroundColor);

                if (c > 32 && c < 127) {
                    fontCharacterCache * f = trueType_renderCharacter(c, trueType_defaultFontRenderer);
                    trueType_drawCharacter(f, x, y, &screenData, textColor);
                }
                
                lastDrawnPtr[0] = c;
                lastDrawnPtr[1] = color;
            }

            textScreenPtr += 2;
            lastDrawnPtr += 2;
        }
    }

    //drawRectangle(graphicsScreen, &windowSection, 30, 40, 200, 10, 0xFF);

    //drawBasicCharacter(graphicsScreen, &windowSection, textInfo, 20, 20, 16, 0xFFFFFFFF);
}

u32 output_update() {
    if (graphicsScreen != NULL) {
        synchronization_OpenSpinlock(&graphics_loopLock);
        if (!graphics_loopDataWaiting) {
            systemCall_ProcessThread_ChangeThreadExecution(0, graphics_loopdThread, SYSTEM_CALL_CHANGE_THREAD_EXECUTION_SCHEDULE, SYSTEM_CALL_NO_PERMISSION);
        }
        graphics_loopDataWaiting = true;
        synchronization_CloseSpinlock(&graphics_loopLock);
    }
}

void output_drawGraphicsLoop() {
    while (true) {
        u32 threadOperation = SYSTEM_CALL_CHANGE_THREAD_EXECUTION_HALT;

        graphics_loopDataWaiting = false;
        if (graphicsScreen != NULL) {
            synchronization_OpenSpinlock(&graphics_drawLock);
            drawGraphicsScreen();
            synchronization_CloseSpinlock(&graphics_drawLock);

            synchronization_OpenSpinlock(&graphics_loopLock);
            if (graphics_loopDataWaiting) {
                threadOperation = SYSTEM_CALL_CHANGE_THREAD_EXECUTION_YIELD_TIME_SLICE;
            }
            synchronization_CloseSpinlock(&graphics_loopLock);
        }

        systemCall_ProcessThread_ChangeThreadExecution(0, graphics_loopdThread, threadOperation, SYSTEM_CALL_NO_PERMISSION);
    }
}
