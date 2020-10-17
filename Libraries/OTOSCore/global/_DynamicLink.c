#include "Definitions.h"
#include "SystemCalls.h"
#include "LibraryManager/MessagingDefinitions.h"

void * _OTOSCore_LoadLocation = 0; //To be filled in at run time

#define MEMORY_SYSTEM_CALL_TYPE_MAP_MEMORY 0x0
#define SYSTEM_CALL_MESSAGING_MESSAGE_PROCESS 2
#define SYSTEM_CALL_SHARED_MEMORY_MAP_SEGMENT 3

static inline SystemCallResult OTOSCore_Initialization_MapPage(PID pid, MemoryPage basePage, SystemCallPermission permission) {
    SystemCallResult result;
    asm volatile (
        "mov eax, %2    \n\t"\
        "push eax       \n\t"\
        "mov eax, %1    \n\t"\
        "int 0x40       \n\t"\
        "add esp, 4"
        : "=a" (result)
        : "g" (MEMORY_SYSTEM_CALL_TYPE_MAP_MEMORY), "g" (permission), "b" ((u32)basePage | 1), "c" (1), "d" (pid)
        : "cc", "memory"
    );
    return result;
}

static inline SystemCallResult OTOSCore_Initialization_MessageProcess(PID pid, MemoryPage dataPage) {
    SystemCallResult result;
    asm volatile (
        "push ebx   \n\t"
        "push ecx   \n\t"
        "push edx   \n\t"
        "push esi   \n\t"
        "push edi   \n\t"
        "push ebp   \n\t"
        "pushf      \n\t"
        "int 0x43   \n\t"
        "popf       \n\t"
        "pop ebp    \n\t"
        "pop edi    \n\t"
        "pop esi    \n\t"
        "pop edx    \n\t"
        "pop ecx    \n\t"
        "pop ebx        "
        : "=a" (result)
        : "a" (SYSTEM_CALL_MESSAGING_MESSAGE_PROCESS), "b" (dataPage), "d" (pid)
        : "cc", "memory"
    );
    return result;
}

static inline SystemCallResult OTOSCore_Initialization_MapSegment(PID sourcePID, u32 segmentID, MemoryPage destinationBaseAddress, u32 nPages) {
    SystemCallResult result;
    asm volatile (
        "push %1    \n\t"\
        "int 0x44   \n\t"\
        "add esp, 4"
        : "=a" (result)
        : "g" (SYSTEM_CALL_NO_PERMISSION), "a" (SYSTEM_CALL_SHARED_MEMORY_MAP_SEGMENT), "b" (destinationBaseAddress), "c" (nPages), "d" (0), "D" (segmentID), "S" (sourcePID)
        : "cc", "memory"
    );
    return result;
}

void OTOSCore_Load(MemoryPage messagePage, MemoryPage loadLocation, PID libraryManagerPID) {

    OTOSCore_Initialization_MapPage(0x00010003, messagePage, SYSTEM_CALL_NO_PERMISSION);
    struct LibraryManager_Messaging_GetLibrary * messageData = (void*)( (IntegerPointer)messagePage + SYSTEM_CALL_MESSAGING_DATA_OFFSET );
    messageData -> operation = LIBRARY_MANAGER_MESSAGING_OPERATION_GET_LIBRARY;
    u32 i;
    u8 libraryName[] = "OTOSCore";
    for (i = 0; i < 8; i++) {
        messageData -> libraryName[i] = libraryName[i];
    }
    messageData -> libraryNameLength = 8;
    OTOSCore_Initialization_MessageProcess(0x00010003, messagePage);
    OTOSCore_Initialization_MapSegment(0x00010003, messageData -> segment, loadLocation, messageData -> bufferNPages);

    _OTOSCore_LoadLocation = (void *)loadLocation;
}
