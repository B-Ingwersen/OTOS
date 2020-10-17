#ifndef MESSAGING_H
#define MESSAGING_H

#include "OTOSCore/Definitions.h"

void messaging_DefaultMessageHandler(PID pid, MemoryPage dataPage);

void messaging_Test(PID pid, void * dataPointer);

void messaging_RequestForwarding(PID pid, void * dataPointer);

#endif