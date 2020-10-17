
#include "Messaging.h"
#include "LibraryManagement.h"
#include "OTOSCore/Threads.h"
#include "OTOSCore/SharedMemory.h"
#include "OTOSCore/SystemCalls.h"

void messaging_DefaultMessageHandler(PID pid, MemoryPage dataPage) {
    void * data = (void *) ( ((IntegerPointer)dataPage) + SYSTEM_CALL_MESSAGING_DATA_OFFSET );
    u32 operation = *(u32*)data;

    if (operation == LIBRARY_MANAGER_MESSAGING_OPERATION_TEST) {
        messaging_Test(pid, data);
    }
    else if (operation == LIBRARY_MANAGER_MESSAGING_OPERATION_GET_LIBRARY) {
        messaging_GetLibrary(pid, data);
    }

    systemCall_Messaging_MessageReturn();
}

void messaging_Test(PID pid, void * dataPointer) {
    struct LibraryManager_Messaging_Test * data = (struct LibraryManager_Messaging_Test *)dataPointer;
    data -> result = GENERIC_SUCCESS_RESULT;
}

void messaging_GetLibrary(PID pid, void * dataPointer) {
    threads_SetupThread();

    struct LibraryManager_Messaging_GetLibrary * data = (struct LibraryManager_Messaging_GetLibrary *)dataPointer;
    data -> result = GENERIC_ERROR_RESULT;

    SharedMemory_SegmentID segment = getLibraryFromSearchableName(data -> libraryName, data -> libraryNameLength);
    if (segment != SHARED_MEMORY_ERROR_SEGMENT) {
        sharedMemory_GrantProcessPermission(segment, pid, false);
        data -> result = GENERIC_SUCCESS_RESULT;
    }
    data -> segment = segment;

    threads_CleanupThread();
}