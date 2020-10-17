
#ifndef INITIALIZATION_H
#define INITIALIZATION_H

#include "Definitions.h"
#include "OTOSCore/Definitions.h"

struct DiskDescriptor * initializeMBRDisk(struct DiskDriver * driver, u32 diskID);

u32 initializeDiskManager();

#endif