
#include "InterruptHandler.h"
#include "IPC.h"
#include "PS2.h"
#include "Translation.h"
#include "Debugging.h"
#include "OTOSCore/SystemCalls.h"

SystemCallResult systemCall_Interrupt_GetInterruptForward(u32 interruptNumber, void * handler, void * immediateHandler) {
    SystemCallResult result;
    asm volatile (
        "int 0x46"
        : "=a" (result)
        : "b" (interruptNumber), "c" (handler), "d" (immediateHandler), "a" (SYSTEM_CALL_INTERRUPTS_GET_INTERRUPT_FORWARD)
        : "cc", "memory"
    );
    return result;
}

void keyboardHardwareInterruptHandler(PID pid, void * data) {

    forwardByte_Wakeup();

    systemCall_Messaging_MessageReturn();
}

void translateKeyCode(u32 keycode) {
    if (keycode > 0 && keycode <= 0x58) {
        forwardByte_WriteData(translationTable[keycode]);
    }
    else if (keycode > 0x80 && keycode <= 0xD8) {
        forwardByte_WriteData(0xF0);
        forwardByte_WriteData(translationTable[keycode - 0x80]);
    }
}

void keyboardHardwareImmediateHandler() {
    //debugging_HexDump("a", 1);
    u32 keycode = (u32)ps2_ReadKeyCode();
    if (keycode == 0x02) {
        ps2_keycodeSet = 1;
    }

    if (ps2_keycodeSet == 1) {
        translateKeyCode(keycode);
    }
    else {
        forwardByte_WriteData((u8)keycode);
    }
}