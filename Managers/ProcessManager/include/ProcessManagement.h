#ifndef PROCESS_MANAGEMENT_H
#define PROCESS_MANAGEMENT_H

#include "OTOSCore/Definitions.h"
#include "OTOSCore/Synchronization.h"

#define MAXIMUM_NUMBER_OF_PROCESSES 256
#define FIRST_REGULAR_PID_INDEX 3

#define PROCESS_PERMISSION_OTHER_PROCESS_MEMORY 0
#define PROCESS_PERMISSION_OTHER_PROCESS_THREAD 4
#define PROCESS_PERMISSION_KERNEL_MEMORY 8
#define PROCESS_PERMISSION_BIOS_CALL 12
#define PROCESS_PERMISSION_PROCESS_MANAGEMENT 16
#define PROCESS_PERMISSION_INTERRUPT_FORWARD 20
#define PROCESS_PERMISSION_IO_PORTS 24

#define ERROR_PID 0

struct ProcessInfo{
    PID pid;
    u8 fullFileName[2048];
    u8 searchableName[256];
    u32 fullFileName_length;
    u32 searchableName_length;
};

extern struct ProcessInfo ** processInfoTable;
extern u32 * usedPIDTable;
extern Synchronization_Spinlock processInfoTable_spinlock;

void waitForProcessToInitialize(PID pid);

void initializeProcessManagement();

PID createProcess(u8 * name, u32 nameLength, u8 * searchableName, u32 searchableNameLength,
    MemoryPage discardableCodeBuffer, u32 codeBufferNPages, u32 * initialPermissions, u32 initialPermissionsLength);

PID createProcessFromDiskFile(u8 * fileName, u32 fileNameLength, u8 * searchableName, u32 searchableNameLength, u32 * initialPermissions, u32 initialPermissionsLength);

PID getProcessPID_SearchableName(u8 * name, u32 nameLength);

#endif