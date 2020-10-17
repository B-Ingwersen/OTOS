
#include "PS2.h"

Synchronization_Spinlock ps2Spinlock = 0;

u32 ps2_keycodeSet = 2;

static inline void IOPort_ByteOut(u16 port, u8 val) {
    asm volatile ( "outb %1, %0" :: "a"(val), "Nd"(port) );
}

static inline u8 IOPort_ByteIn(u16 port) {
    u8 returnValue;
    asm volatile ( "inb %0, %1" : "=a"(returnValue) : "Nd"(port) );
    return returnValue;
}

void ps2_WaitForCommandResponse() {
    u8 pollResult = 0;
    while ( (pollResult & 1) != 1) {
        pollResult = IOPort_ByteIn(0x64);
    }
}

void ps2_WaitForWriteReady() {
    u8 pollResult = 2;
    while ( (pollResult & 2) != 0) {
        pollResult = IOPort_ByteIn(0x64);
    }
}

bool32 sendByteToKeyboard(u8 byte) {
    u32 attempt = 0;
    while (attempt < 3) {
        ps2_WaitForWriteReady();
        IOPort_ByteOut(0x60, byte);
        ps2_WaitForCommandResponse();
        u8 response = IOPort_ByteIn(0x60);

        if (response == 0xFA) {
            return true;
        }
        ++attempt;
    }
    return false;
}

void ps2_Initialize() {

    while ( true ) {
        u8 inByte = IOPort_ByteIn(0x64);
        if ( (inByte & 1) != 0) {
            IOPort_ByteIn(0x60);
        }
        else {
            break;
        }
    }

    // read controller configuration byte
    ps2_WaitForWriteReady();
    IOPort_ByteOut(0x64, 0x20);
    ps2_WaitForCommandResponse();
    u8 configByte = IOPort_ByteIn(0x60);
    
    // write configuration byte, disabling keyboard translation
    ps2_WaitForWriteReady();
    IOPort_ByteOut(0x64, 0x60);
    ps2_WaitForWriteReady();
    IOPort_ByteOut(0x60, configByte & 0xBF);

    // set scan code set 2
    //sendByteToKeyboard(0xF0);
    //sendByteToKeyboard(0x02);

    /*ps2_WaitForWriteReady();
    IOPort_ByteOut(0x60, 0xF0);
    ps2_WaitForCommandResponse();
    u8 byte = IOPort_ByteIn(0x60);
    IOPort_ByteOut(0x60, 0x02);
    ps2_WaitForCommandResponse();
    byte = IOPort_ByteIn(0x60);*/

    // set repeat rate
    /*ps2_WaitForWriteReady();
    IOPort_ByteOut(0x60, 0xF3);
    ps2_WaitForCommandResponse();
    byte = IOPort_ByteIn(0x60);
    IOPort_ByteOut(0x60, 0x0);
    ps2_WaitForCommandResponse();
    byte = IOPort_ByteIn(0x60);
    ps2_WaitForWriteReady();*/

    /*sendByteToKeyboard(0xF0);
    sendByteToKeyboard(0x00);
    ps2_WaitForCommandResponse();
    ps2_keycodeSet = (u32)IOPort_ByteIn(0x60);
    asm volatile("mov eax, 0\n\tdiv edx" :: "b" (0xDEADB), "c" (ps2_keycodeSet));
    ps2_WaitForWriteReady();*/

    synchronization_InitializeSpinlock(&ps2Spinlock);
}

u8 ps2_ReadKeyCode() {
    return IOPort_ByteIn(0x60);
}