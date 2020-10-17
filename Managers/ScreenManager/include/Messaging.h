#ifndef MESSAGING_H
#define MESSAGING_H

#include "ScreenManager/MessagingDefinitions.h"
#include "OTOSCore/Definitions.h"

void messaging_Test(PID pid, void * dataPointer);

void messaging_GetModeList(PID pid, void * dataPointer);

void messaging_SetMode(PID pid, void * dataPointer);

void messaging_GetFrameBuffer(PID pid, void * dataPointer);

void messaging_DefaultMessageHandler(PID pid, MemoryPage dataPage);

#endif // MESSAGING_H