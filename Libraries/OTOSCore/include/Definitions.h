#ifndef OTOS_CORE___DEFINITIONS_H
#define OTOS_CORE___DEFINITIONS_H

//TYPE DEFITINTIONS
    typedef unsigned char u8;
    typedef char i8;
    typedef unsigned short u16;
    typedef short i16;
    typedef unsigned int u32;
    typedef int i32;
    typedef long long unsigned int u64;
    typedef long long int i64;

    typedef u8 bool8;
    typedef u16 bool16;
    typedef u32 bool32;
    typedef u64 bool64;

    #define GlobalType const
    #define GlobalPointer(type) const IntegerPointer
    #define GlobalAccess(variable, type) *(type volatile *)&variable
    #define GlobalPointerAccess(pointer, type) (type volatile *)pointer

    #define GlobalVariable(type, name) type name = (type)0;

    #define true 1
    #define false 0
    #define GENERIC_ERROR_RESULT 0
    #define GENERIC_SUCCESS_RESULT 1
    #define ExactBinaryStructure __attribute__((packed))

    #define NULL ((void *)0)
    #define PAGE_SIZE 4096
    #define PAGING_ISOLATE_PAGE_ADDRESS_MASK 0xFFFFF000
    #define PAGING_RECURSIVE_MEMORY_MAP_LOCATION 0xFFC00000
    #define PAGING_OS_FLAG_THREAD_CONTEXT 0x0A00

    typedef u32 PID;
    typedef void* MemoryPage;
    typedef u32 IntegerPointer;

//TYPE DEFINITIONS END

void OTOSCore_Load(MemoryPage messagePage, MemoryPage loadLocation, PID libraryManagerPID);

#endif
