#ifndef PROCESSES_H
#define PROCESSES_H

#include "OTOSCore/Definitions.h"

#define PROCESS_MANAGER_PID 0x00010001

u32 processes_GetProcessPID(u8 * processName);

#endif