
#ifndef MESSAGING_H
#define MESSAGING_H

#include "OTOSCore/Definitions.h"
#include "ProcessManager/MessagingDefinitions.h"

void messaging_GetProcessPID(PID pid, void * dataPointer);

void messaging_DefaultMessageHandler(PID pid, MemoryPage dataPage);

void messaging_GetProcessPID(PID pid, void * dataPointer);

#endif
