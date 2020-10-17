
#ifndef MESSAGING_H
#define MESSAGING_H

#include "DiskManager/MessagingDefinitions.h"
#include "OTOSCore/Definitions.h"

void messaging_DefaultMessageHandler(PID pid, MemoryPage dataPage);

u32 messaging_InitializeMessaging();

void messaging_Test(PID pid, void * dataPointer);

void messaging_OpenFileHandle(PID pid, void * dataPointer);

void messaging_ReadFileHandle(PID pid, void * dataPointer);

void messaging_CloseFileHandle(PID pid, void * dataPointer);

void messaging_ReadDirectoryEntries(PID pid, void * dataPointer);

#endif