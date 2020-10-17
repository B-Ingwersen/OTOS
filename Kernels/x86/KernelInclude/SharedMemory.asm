
initializeSharedMemory:                     ; ebx=sharedMemoryInfoAddress:   PDE = in process page tables
    pusha

    mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_SHARED_MEMORY_SPINLOCK
    call openSpinlock

    mov eax, [PROCESS_INFO_ADDRESS + PROCESS_INFO_SHARED_MEMORY_POINTER]
    cmp eax, 0
    jne initializeSharedMemory_fail

    mov eax, cr3
    push eax
    mov eax, KERNEL_PDE_ADDRESS
    mov cr3, eax

    and bx, PAGING_LOW_BIT_REVERSE_MASK

    mov eax, MEMORY_SYSTEM_CALL_NEW_MEMORY
    or bx, PAGING_FLAG_USER | OS_PAGING_FLAG_GENERIC_PROTECTED
    mov ecx, 1
    mov edx, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
    call mapNewOrKernelMemoryIntoProcess
    cmp eax, MEMORY_SYSTEM_CALL_SUCCESS
    jne initializeSharedMemory_fail2

    pop eax
    mov cr3, eax

    and bx, PAGING_LOW_BIT_REVERSE_MASK
    mov [PROCESS_INFO_ADDRESS + PROCESS_INFO_SHARED_MEMORY_POINTER], ebx

    .zeroPage:
        xor eax, eax
        mov edi, ebx
        mov ecx, PAGE_SIZE
        rep stosb

    mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_SHARED_MEMORY_SPINLOCK
    call closeSpinlock

    popa
    mov eax, SHARED_MEMORY_SUCCESS
    ret

    initializeSharedMemory_fail2:

    pop eax
    mov cr3, eax

    initializeSharedMemory_fail:

    mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_SHARED_MEMORY_SPINLOCK
    call closeSpinlock

    popa
    mov eax, SHARED_MEMORY_FAIL
    ret

addSharedMemorySegment:                 ; ebx = base address, ecx = # pages, edx = descriptor address, edi = descriptor #
    pusha

    mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_SHARED_MEMORY_SPINLOCK
    call openSpinlock

    ; check if the shared memory info page is allocated
    mov eax, [PROCESS_INFO_ADDRESS + PROCESS_INFO_SHARED_MEMORY_POINTER]
    cmp eax, 0
    je addSharedMemorySegment_fail

    ; open page
    .checkDescriptorNumberNotUsed:
        cmp edi, MAX_SHARED_MEMORY_SEGMENTS
        jae addSharedMemorySegment_fail
        shl edi, 2
        add eax, edi
        cmp [eax], dword 0
        jne addSharedMemorySegment_fail

    and bx, PAGING_LOW_BIT_REVERSE_MASK
    push eax

    .prepForProcessAccess:
        mov eax, cr3
        push eax
        mov eax, KERNEL_PDE_ADDRESS
        mov cr3, eax

        mov eax, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
        and eax, 0xFF
        lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * eax]
        call openSpinlock

    .checkSegment:
        push edx
        mov edx, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
        call checkMemoryRegionMapped
        pop edx
        cmp eax, MEMORY_CHECK_SUCESS_CODE
        jne addSharedMemorySegment_fail2

    .checkSharedMemoryDescriptor:

        push ebx
        push ecx
        push edx
        mov ebx, edx
        mov ecx, 1
        mov edx, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
        call checkMemoryRegionNotMapped
        cmp eax, MEMORY_CHECK_SUCESS_CODE
        jne addSharedMemorySegment_fail3

    .mapSharedMemoryDescriptor:
        call mapPTEsForMemoryRegion
        or ebx, OS_PAGING_FLAG_GENERIC_PROTECTED | PAGING_FLAG_USER
        call mapNewMemoryForRegion
        pop edx
        pop ecx
        pop ebx
    
    .setSegmentAsProtected:
        push edx
        mov edx, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
        or ebx, OS_PAGING_FLAG_GENERIC_PROTECTED | PAGING_FLAG_PRESENT | PAGING_FLAG_USER | PAGING_FLAG_WRITE
        call changeFlagsForMemoryRegion
        pop edx

    mov eax, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
    and eax, 0xFF
    lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * eax]
    call closeSpinlock

    pop eax
    mov cr3, eax

    .fillInDescriptorInformation:
        push ecx
        xor eax, eax
        mov edi, edx
        mov ecx, PAGE_SIZE
        rep stosb
        pop ecx

        pop eax
        and bx, PAGING_LOW_BIT_REVERSE_MASK
        mov [eax], edx
        mov edi, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
        and edi, 0xFF
        shl edi, 3
        mov [edx + edi], ebx
        mov [edx + edi + 4], ecx

    mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_SHARED_MEMORY_SPINLOCK
    call closeSpinlock

    popa
    mov eax, SHARED_MEMORY_SUCCESS
    ret

    addSharedMemorySegment_fail3:

    pop edx
    pop ecx
    pop ebx

    addSharedMemorySegment_fail2:

    mov eax, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
    and eax, 0xFF
    lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * eax]
    call closeSpinlock

    pop eax
    mov cr3, eax

    pop eax

    addSharedMemorySegment_fail:

    mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_SHARED_MEMORY_SPINLOCK
    call closeSpinlock

    popa
    xor eax, eax
    ret

modifySharedMemSegment:                     ; ebx = base address OR new permissions, ecx = # pages, edx = PID, edi = descriptor #, esi = source PID
                                                ; eax = (add segment, remove segment)
    push SHARED_MEMORY_FAIL
    push eax
    pusha

    ;delete operations bypass spinlock
        cmp eax, 0x10000
        jae .noSpinlock1

        mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_SHARED_MEMORY_SPINLOCK
        call openSpinlock

    .noSpinlock1:

    ;check if trying to access the same process -- would be bad
    cmp esi, edx
    je modifySharedMemSegment_fail

    ; check if the shared memory info page is allocated
    mov eax, [PROCESS_INFO_ADDRESS + PROCESS_INFO_SHARED_MEMORY_POINTER]
    cmp eax, 0
    je modifySharedMemSegment_fail

    ; open page
    .checkDescriptor:
        cmp edi, MAX_SHARED_MEMORY_SEGMENTS
        jae modifySharedMemSegment_fail
        lea eax, [eax + 4 * edi]                                            ; location in shm info
        mov eax, [eax]                                                      ; eax = descriptor address
        cmp eax, 0
        je modifySharedMemSegment_fail
    
    mov edi, edx
    and edi, 0xFF    
    lea edi, [eax + 8 * edi]                                                ; edi = location in the descriptor                                                
    push edi                                                                ; #STACK + 4

    .getBaseAddressAndLength:
        push esi
        and esi, 0xFF
        mov ecx, [eax + 8 * esi + 4]
        ;cmp ecx, edi
        ;jne modifySharedMemSegment_fail2                                    ; must map # of pages exactly
        ;mov ecx, edx                                                        ; ecx = # of pages
        mov edi, [eax + 8 * esi]                                            ; edi = source base address
        pop esi
    
    .determineCall:
        mov eax, [esp + 32 + 4]                                             ; retrieve operation code
        cmp ax, SHARED_MEMORY_MODIFY_ADD_SEGMENT
        je modifySharedMemSegment_addNewMemorySegment
        cmp ax, SHARED_MEMORY_MODIFY_REMOVE_SEGMENT
        je modifySharedMemSegment_removeSegment
        cmp ax, SHARED_MEMORY_MODIFY_CHANGE_PERMISSIONS
        je modifySharedMemSegment_changePermissions
        jmp modifySharedMemSegment_fail2

    modifySharedMemSegment_addNewMemorySegment:
        mov eax, [esp]
        cmp [eax], dword 0
        jne modifySharedMemSegment_fail2                     ; segment already mapped

        mov eax, [eax + 4]
        and al, MEMORY_SEGMENT_MARKER_ACCESS_ALLOWED
        cmp al, 0
        je modifySharedMemSegment_fail2                     ; access not granted

        mov al, dl
        cmp eax, edx
        jne modifySharedMemSegment_fail2                    ; PID incorrect

        mov eax, [esp]
        mov eax, [eax + 4]
        and ax, MEMORY_SEGMENT_MARKER_WRITE_ALLOWED
        or ax, PAGING_FLAG_USER | OS_PAGING_FLAG_SHARED
        and bx, PAGING_LOW_BIT_REVERSE_MASK
        or bx, ax

        .storePDE:
            mov eax, cr3
            push eax                                                        ; #STACK + 4

        .prepForProcessAccess:
            mov eax, KERNEL_PDE_ADDRESS
            mov cr3, eax
        
        mov eax, MEMORY_SYSTEM_CALL_SHARE_MEMORY
        call moveOrShareMemoryToProcess
        cmp eax, MEMORY_SYSTEM_CALL_FAILURE
        je modifySharedMemSegment_fail3

        pop eax
        mov cr3, eax

        pop edi
        and bx, PAGING_LOW_BIT_REVERSE_MASK
        mov [edi], ebx

        mov [esp + 9 * 4], dword SHARED_MEMORY_SUCCESS
        jmp modifySharedMemSegment_fail

    modifySharedMemSegment_removeSegment:
        mov eax, [esp]
        mov ebx, [eax]                                      ; ebx = segment start address
        cmp ebx, dword 0
        je .alreadyRemoved                                  ; segment unmapped

        .storePDE:
            mov eax, cr3
            push eax                                                        ; #STACK + 4

        .prepForProcessAccess:
            mov eax, KERNEL_PDE_ADDRESS
            mov cr3, eax

        or ebx, OS_PAGING_FLAG_SHARED
        call removeMemoryFromProcess
        ;cmp eax, MEMORY_SYSTEM_CALL_FAILURE
        ;je modifySharedMemSegment_fail3

        pop eax
        mov cr3, eax

        .alreadyRemoved:

        pop edi
        mov [edi], dword 0

        mov [esp + 9 * 4], dword SHARED_MEMORY_SUCCESS
        jmp modifySharedMemSegment_fail
    
    modifySharedMemSegment_changePermissions:                                     ; ebx = flags, edx = PID
        pop edi
        cmp [edi], dword 0
        jne modifySharedMemSegment_fail

        mov dl, bl                                          ; PID in bits 8-31, flags in bits 0-7
        mov [edi + 4], edx
        mov [esp + 9 * 4], dword SHARED_MEMORY_SUCCESS
        jmp modifySharedMemSegment_fail

    modifySharedMemSegment_fail3:

    .restoreSavedPDE:
        pop eax
        mov cr3, eax
    
    modifySharedMemSegment_fail2:

    pop edi                                     ; edi is the location of the descriptor page

    modifySharedMemSegment_fail:
    
    mov eax, [esp + 8 * 4]
    cmp eax, 0x10000
    jae .noSpnilock2

    mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_SHARED_MEMORY_SPINLOCK
    call closeSpinlock

    .noSpnilock2:

    popa
    pop eax
    pop eax
    ret
    
deleteSharedMemorySegment:                      ; edi = descriptor #, esi = pid
    push SHARED_MEMORY_FAIL
    push eax
    pusha

    ;mov esi, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]

    cmp eax, 0x10000
    jae .skipSpinlock1

    mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_SHARED_MEMORY_SPINLOCK
    call openSpinlock

    .skipSpinlock1:

    ; check if the shared memory info page is allocated
    mov eax, [PROCESS_INFO_ADDRESS + PROCESS_INFO_SHARED_MEMORY_POINTER]
    cmp eax, 0
    je .fail

    .checkDescriptor:
        cmp edi, MAX_SHARED_MEMORY_SEGMENTS
        jae .fail
        lea eax, [eax + 4 * edi]                                            ; location in shm info
        mov eax, [eax]                                                      ; eax = descriptor address
        cmp eax, 0
        je .fail

    push eax
    xor edx, edx
    xor ebx, ebx
    mov bx, si
    xor bh, bh
    mov ecx, 512
    .deleteDescriptorsLoop:
        cmp [eax], dword 0
        je .skipEntry
        cmp edx, ebx
        je .skipEntry

        push eax
        push edx
        mov edx, [PROCESS_PID_TABLE_ADDRESS + 4 * edx]
        mov dl, [esp]
        mov eax, SHARED_MEMORY_MODIFY_REMOVE_SEGMENT_NO_SPINLOCK
        call modifySharedMemSegment
        pop edx
        pop eax
        
        .skipEntry:

        inc edx
        add eax, 8
        loop .deleteDescriptorsLoop

    pop eax

    .removeProtectedFlagFromSegment:
        xor edx, edx
        mov dx, si
        mov dh, 0
        shl edx, 3
        mov ebx, [eax + edx]
        mov ecx, [eax + edx + 4]

    mov edx, cr3
    push edx
    mov edx, KERNEL_PDE_ADDRESS
    mov cr3, edx

        mov edx, esi
        and bx, PAGING_LOW_BIT_REVERSE_MASK
        or bl, PAGING_FLAG_USER | PAGING_FLAG_PRESENT | PAGING_FLAG_WRITE
        call changeFlagsForMemoryRegion
    .unmapDescriptor:
        mov ebx, eax
        mov ecx, 1
        call unmapMemoryForRegion
    
    pop edx
    mov cr3, edx

    mov [esp + 9 * 4], dword SHARED_MEMORY_SUCCESS

    mov eax, [esp + 8 * 4]
    cmp eax, 0x10000
    jae .noSpnilock2

    mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_SHARED_MEMORY_SPINLOCK
    call closeSpinlock

    .noSpnilock2:

    .fail:

    popa
    pop eax
    pop eax
    
    ret

deleteAllSharedMemory:                                      ; esi = pid
    push SHARED_MEMORY_FAIL
    pusha

    ;mov esi, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]

    mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_SHARED_MEMORY_SPINLOCK
    call openSpinlock

    ; check if the shared memory info page is allocated
    mov eax, [PROCESS_INFO_ADDRESS + PROCESS_INFO_SHARED_MEMORY_POINTER]
    cmp eax, 0
    je .fail

    mov ecx, 1024
    xor edi, edi
    .deleteAllLoop:
        cmp [eax], dword 0
        je .skipEntry

        mov eax, DELETE_SHARED_MEMORY_SEGMENT_NO_SPINLOCK
        call deleteSharedMemorySegment

        .skipEntry:
        inc edi
        add eax, 4

        loop .deleteAllLoop
    
    .deleteInfoPage:
        mov ebx, [PROCESS_INFO_ADDRESS + PROCESS_INFO_SHARED_MEMORY_POINTER]

        mov eax, cr3
        push eax
        mov eax, KERNEL_PDE_ADDRESS
        mov cr3, eax

        mov ecx, 1
        mov edx, esi
        call unmapMemoryForRegion

        pop eax
        mov cr3, eax

    mov [esp + 8 * 4], dword SHARED_MEMORY_SUCCESS

    .fail:

    popa
    pop eax
    ret