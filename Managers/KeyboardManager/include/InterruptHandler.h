#ifndef INTERRUPT_HANDLER_H
#define INTERRUPT_HANDLER_H

#include "OTOSCore/Definitions.h"
#include "OTOSCore/SystemCalls.h"

#define SYSTEM_CALL_INTERRUPTS_GET_INTERRUPT_FORWARD 1
#define SYSTEM_CALL_INTERRUPTS_SET_IO_PORT_PERMISSION 2

SystemCallResult systemCall_Interrupt_GetInterruptForward(u32 interruptNumber, void * handler, void * immediateHandler);

void keyboardHardwareInterruptHandler(PID pid, void * data);

void translateKeyCode(u32 keycode);

void keyboardHardwareImmediateHandler();

#endif