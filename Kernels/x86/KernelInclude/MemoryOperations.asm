
mapNewOrKernelMemoryIntoProcess:                ; ebx=base address (bx include desired flags), ecx = # pages, edx = PID, edi = Kernel Memory Base (if using kernel memory)
                                            ; eax = map new OR kernel
    pusha
    push eax
    
    xor eax, eax
    mov al, dl
    lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * eax]
    call openSpinlock

    call checkPIDExists
    cmp eax, PID_EXISTS_CODE
    jne mapNewOrKernelMemoryIntoProcess_fail

    call checkMemoryRegionNotMapped
    cmp eax, MEMORY_CHECK_SUCESS_CODE
    jne mapNewOrKernelMemoryIntoProcess_fail

    mov eax, [esp]
    cmp eax, MEMORY_SYSTEM_CALL_NEW_MEMORY
    jne mapNewOrKernelMemoryIntoProcess_notNewMem
    call mapPTEsForMemoryRegion
    call mapNewMemoryForRegion
    jmp mapNewOrKernelMemoryIntoProcess_success

    mapNewOrKernelMemoryIntoProcess_notNewMem:

    cmp eax, MEMORY_SYSTEM_CALL_KERNEL_MEMORY
    jne mapNewOrKernelMemoryIntoProcess_fail
    call mapPTEsForMemoryRegion
    call mapKernelMemoryForRegion

    mapNewOrKernelMemoryIntoProcess_success:

    xor eax, eax
    mov al, dl
    lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * eax]
    call closeSpinlock

    pop eax
    popa
    mov eax, MEMORY_SYSTEM_CALL_SUCCESS
    ret

    mapNewOrKernelMemoryIntoProcess_fail:

    xor eax, eax
    mov al, dl
    lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * eax]
    call closeSpinlock

    pop eax
    popa
    mov eax, MEMORY_SYSTEM_CALL_FAILURE
    ret

removeMemoryFromProcess:                ; ebx=base address (bx include desired flags), ecx = # pages, edx = PID
                                            ; eax = map new OR kernel
    pusha
    push eax
    
    xor eax, eax
    mov al, dl
    lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * eax]
    call openSpinlock

    call checkPIDExists
    cmp eax, PID_EXISTS_CODE
    jne removeMemoryFromProcess_fail

    call checkMemoryRegionMapped
    cmp eax, MEMORY_CHECK_SUCESS_CODE
    jne removeMemoryFromProcess_fail

    call unmapMemoryForRegion

    removeMemoryFromProcess_success:

    xor eax, eax
    mov al, dl
    lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * eax]
    call closeSpinlock

    pop eax
    popa
    mov eax, MEMORY_SYSTEM_CALL_SUCCESS
    ret

    removeMemoryFromProcess_fail:

    xor eax, eax
    mov al, dl
    lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * eax]
    call closeSpinlock

    pop eax
    popa
    mov eax, MEMORY_SYSTEM_CALL_FAILURE
    ret

moveOrShareMemoryToProcess:                     ;ebx=base address (bl include desired flags), ecx = # pages, edx = PID, edi=source base address, esi=source PID
                                                ; eax = move or share memory
    push dword MEMORY_SYSTEM_CALL_FAILURE
    pusha
    push eax

    .getDualSpinlockAccess:
        mov eax, MULTIPLE_PROCESS_MEMORY_SPINLOCK_ACCESS
        call openSpinlock
        
        xor eax, eax
        mov al, dl
        lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * eax]
        call openSpinlock

        xor eax, eax
        mov ax, si
        mov ah, 0
        lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * eax]
        call openSpinlock

    call checkPIDExists
    cmp eax, PID_EXISTS_CODE
    jne moveOrShareMemoryToProcess_fail

    push edx
    mov edx, esi
    call checkPIDExists
    pop edx
    cmp eax, PID_EXISTS_CODE
    jne moveOrShareMemoryToProcess_fail

    call checkMemoryRegionNotMapped
    cmp eax, MEMORY_CHECK_SUCESS_CODE
    jne moveOrShareMemoryToProcess_fail

    push ebx
    push edx
    mov ebx, edi
    mov edx, esi
        cmp [esp + 8], dword MEMORY_SYSTEM_CALL_SHARE_MEMORY
        jne .noProtectedPagesAllowed
        or ebx, OS_PAGING_FLAG_GENERIC_PROTECTED
    .noProtectedPagesAllowed:
    call checkMemoryRegionMapped
    pop edx
    pop ebx
    cmp eax, MEMORY_CHECK_SUCESS_CODE
    jne moveOrShareMemoryToProcess_fail

    cmp edx, esi
    je moveOrShareMemoryToProcess_fail

    mov eax, [esp]
    cmp eax, MEMORY_SYSTEM_CALL_MOVE_MEMORY
    jne moveOrShareMemoryToProcess_notMove
    call mapPTEsForMemoryRegion
    call moveProcessMemoryRegion
    jmp moveOrShareMemoryToProcess_success

    moveOrShareMemoryToProcess_notMove:
    cmp eax, MEMORY_SYSTEM_CALL_SHARE_MEMORY
    jne moveOrShareMemoryToProcess_fail
    call mapPTEsForMemoryRegion
    call copyProcessMemoryRegion

    moveOrShareMemoryToProcess_success:

    mov [esp + 36], dword MEMORY_SYSTEM_CALL_SUCCESS

    moveOrShareMemoryToProcess_fail:

    xor eax, eax
    mov ax, si
    mov ah, 0
    lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * eax]
    call closeSpinlock

    xor eax, eax
    mov al, dl
    lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * eax]
    call closeSpinlock

    mov eax, MULTIPLE_PROCESS_MEMORY_SPINLOCK_ACCESS
    call closeSpinlock

    pop eax
    popa
    pop eax
    ret



