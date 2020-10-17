
#ifndef MESSAGING_H
#define MESSAGING_H

#include "LibraryManager/MessagingDefinitions.h"

void messaging_DefaultMessageHandler(PID pid, MemoryPage dataPage);

void messaging_Test(PID pid, void * dataPointer);

void messaging_GetLibrary(PID pid, void * dataPointer);

#endif