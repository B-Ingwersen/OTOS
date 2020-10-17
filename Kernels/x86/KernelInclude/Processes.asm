createProcess:          ; eax = pid
    pusha

    mov edi, eax
    xor edx, edx
    mov dl, al
    lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * edx]
    call openSpinlock

    mov ebx, [PROCESS_TABLE_ADDRESS + 4 * edx]
    cmp ebx, 0
    jne createProcess_PIDAlreadyUsed

    and di, 0xFF00
    mov [PROCESS_PID_TABLE_ADDRESS + 4 * edx], edi

    mov ecx, 3
    createProcess_get3FreePages:
        call getMemoryTablePage
        call zeroPagePagingEnabled     
        push eax
        loop createProcess_get3FreePages
    ;call disablePaging
    ;mov ecx, 3
    ;createProcess_zero3Pages:
    ;    mov eax, [esp + ecx - 4]
    ;    call zeroPage
    ;    loop createProcess_zero3Pages

    ; put entry in process table
    mov eax, [esp]
    or ax, OS_PAGING_FLAG_GENERIC_PROTECTED | PAGING_FLAG_PRESENT
    mov [PROCESS_TABLE_ADDRESS + 4 * edx], eax

    ; insert recursive pde - pte mapping
    mov ecx, edx
    shl ecx, PAGE_SHIFT_RIGHT_BITS
    add ecx, PROCESS_PDES_RECURSIVE_ACCESS
    invlpg [ecx]
    or ax, PAGING_FLAG_USER
    mov [ecx + 1023 * 4], eax

    ; insert 1st pte
    mov eax, [esp + 4]
    or ax, OS_PAGING_FLAG_GENERIC_PROTECTED | PAGING_FLAG_PRESENT | PAGING_FLAG_USER | PAGING_FLAG_WRITE
    mov [ecx], eax

    ; fill first PTE
    mov eax, edx
    shl eax, 22
    add eax, PROCESS_VIRTUAL_MEMORY_MAP_ADDRESS
    invlpg [eax]
    mov ebx, eax
    xor ecx, ecx
    or cx, OS_PAGING_FLAG_KERNEL | PAGING_FLAG_PRESENT
    createProcess_fillFirstPTE:
        mov [eax], ecx
        add eax, 4
        add ecx, PAGE_SIZE
        cmp ecx, (MEMORY_KENREL_MEMORY_MAX_ADDRESS - PAGE_SIZE)
        jb createProcess_fillFirstPTE
    
    ; insert Process Info
    ;and cx, PAGING_LOW_BIT_REVERSE_MASK
    mov eax, [esp + 8]
    or ax, OS_PAGING_FLAG_GENERIC_PROTECTED | PAGING_FLAG_PRESENT | PAGING_FLAG_USER
    mov [ebx + 255 * 4], eax

    ;insert Local APIC mapping
    mov eax, [POINTER_TO_LOCAL_APIC_ADDRESS]
    or ax, PAGING_FLAG_PRESENT | OS_PAGING_FLAG_KERNEL
    mov [ebx + (LOCAL_APIC_PAGED_LOCATION >> 10)], eax

    add esp, 12

    lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * edx]
    call closeSpinlock

    popa
    mov eax, 1
    ret

    createProcess_PIDAlreadyUsed:

    call closeSpinlock
    popa
    mov eax, CREATE_PROCESS_FAILURE_CODE

    ret

checkPIDExists:                         ; edx = PID
    push ecx
    push edx

    cmp edx, dword 0
    je .fail

    xor ecx, ecx
    mov cl, dl
    mov eax, [PROCESS_PID_TABLE_ADDRESS + 4 * ecx]
    mov al, 0
    mov dl, 0
    cmp eax, edx
    jne .fail

    mov eax, [PROCESS_TABLE_ADDRESS + 4 * ecx]
    and ax, PROCESS_DELETE_FLAG
    cmp ax, 0
    jne .fail

    mov eax, PID_EXISTS_CODE
    jmp .end

    .fail:
    mov eax, PID_DOESNT_EXISTS_CODE

    .end:
    pop edx
    pop ecx
    ret

enterJustProcess:                               ; eax = PID
    pusha

    mov edx, eax
    xor ebx, ebx
    mov bl, dl
    lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * ebx]
    call openSpinlock

    call checkPIDExists
    cmp eax, PID_DOESNT_EXISTS_CODE
    je .enterProcess_fail

    mov al, [PROCESS_PID_TABLE_ADDRESS + 4 * ebx]
    and al, 10000000b
    cmp al, 0
    jne .enterProcess_fail

    add byte [PROCESS_PID_TABLE_ADDRESS + 4 * ebx], 1

    mov eax, [PROCESS_TABLE_ADDRESS + 4 * ebx]
    and ax, PAGING_LOW_BIT_REVERSE_MASK
    mov cr3, eax

    lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * ebx]
    call closeSpinlock

    popa
    mov eax, PROCESS_ENTER_SUCCESS
    ret

    .enterProcess_fail:

    lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * ebx]
    call closeSpinlock

    popa
    mov eax, PROCESS_ENTER_FAILURE
    ret

addProcessReference:                               ; eax = PID
    pusha

    mov edx, eax
    xor ebx, ebx
    mov bl, dl
    lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * ebx]
    call openSpinlock

    call checkPIDExists
    cmp eax, PID_DOESNT_EXISTS_CODE
    je .fail

    mov al, [PROCESS_PID_TABLE_ADDRESS + 4 * ebx]
    and al, 10000000b
    cmp al, 0
    jne .fail

    add byte [PROCESS_PID_TABLE_ADDRESS + 4 * ebx], 1

    lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * ebx]
    call closeSpinlock

    popa
    mov eax, PROCESS_ENTER_SUCCESS
    ret

    .fail:

    lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * ebx]
    call closeSpinlock

    popa
    mov eax, PROCESS_ENTER_FAILURE
    ret

exitJustProcess:                                 ; eax = PID, ebx = return PDE
    pusha

    mov edx, eax
    mov ecx, ebx
    xor ebx, ebx
    mov bl, dl
    lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * ebx]
    call openSpinlock

    call checkPIDExists
    cmp eax, PID_DOESNT_EXISTS_CODE
    je .exitProcess_quasiFail

    sub byte [PROCESS_PID_TABLE_ADDRESS + 4 * ebx], 1

    .exitProcess_quasiFail:

    mov eax, ecx
    mov cr3, eax

    lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * ebx]
    call closeSpinlock

    popa
    ret

removeProcessReference:                                 ; eax = PID
    pusha

    mov edx, eax
    xor ebx, ebx
    mov bl, dl
    lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * ebx]
    call openSpinlock

    call checkPIDExists
    cmp eax, PID_DOESNT_EXISTS_CODE
    je .quasiFail

    sub byte [PROCESS_PID_TABLE_ADDRESS + 4 * ebx], 1

    .quasiFail:

    lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * ebx]
    call closeSpinlock

    popa
    ret

changeProcessPermission:                   ;edx = PID, ebx = permission, ecx = value
    pusha

    mov eax, cr3
    push eax

    cmp ebx, PROCESS_PERMISSION_MAX
    jae .fail
    and bl, 0xFC

    mov eax, edx
    call enterJustProcess
    cmp eax, PROCESS_ENTER_SUCCESS
    jne .fail

    mov [ebx + PROCESS_INFO_ADDRESS + PROCESS_INFO_PERMISSIONS_BASE], ecx

    pop ebx
    mov eax, edx
    call exitJustProcess

    popa
    mov eax, 1
    ret

    .fail:

    pop eax
    popa
    mov eax, 0
    ret

deleteProcess:                                                  ; edx = PID; NOTE: THIS WILL CHANGE CR3 = KERNEL_PDE_ADDRESS; SAVE CR3 before calling!!!!!
    pusha

    .checkIfDeletingCallingProcess:
        cmp edx, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
        je .deletingCallingProcess

        mov eax, edx
        call enterJustProcess
        cmp eax, PROCESS_ENTER_SUCCESS
        jne .fail1

        .deletingCallingProcess:
    
    .checkAndSetDeleteFlag:
        xor ebx, ebx
        mov bl, dl
        lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * ebx]
        call openSpinlock

        mov eax, [PROCESS_TABLE_ADDRESS + 4 * ebx]
        and ax, PROCESS_DELETE_FLAG
        cmp ax, 0
        jne .fail2

        or [PROCESS_TABLE_ADDRESS + 4 * ebx], dword PROCESS_DELETE_FLAG
    
    lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * ebx]
    call closeSpinlock

    .waitForDereferences:
        nop
        nop
        nop
        nop

        mov eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * ebx]
        cmp al, 1
        ja .waitForDereferences

    call deleteAllSharedMemory
    mov eax, SYSTEM_CALL_CAPABILITIES_DELETE
    call processCapabilitiesOperations
    call destroyMessaging

    mov ebx, RECURSIVE_PTE_MAP_BASE
    ;shl ebx, PROCESS_VIRTUAL_MEMORY_MAP_ENRTY_SIZE_SHL_BITS
    ;add ebx, PROCESS_VIRTUAL_MEMORY_MAP_ADDRESS
    ;lea ecx, [ebx + PAGE_SIZE]
    ;lea esi, [ebx + PROCESS_VIRTUAL_MEMORY_MAP_ENRTY_SIZE - PAGE_SIZE]
    ;lea edi, [ebx + PROCESS_VIRTUAL_MEMORY_MAP_ENRTY_SIZE - 4]
    ;mov ebx, RECURSIVE_PTE_MAP_BASE
    .removeMemoryPassStart:

        lea ecx, [ebx + PAGE_SIZE]
        lea esi, [ebx + PROCESS_VIRTUAL_MEMORY_MAP_ENRTY_SIZE - PAGE_SIZE]
        lea edi, [ebx + PROCESS_VIRTUAL_MEMORY_MAP_ENRTY_SIZE - 4]
        .removeMemoryMapPDELoop:
            mov eax, [esi]
            cmp eax, 0
            je .noPTEMapped

            .removeMemoryMapPTELoop:
                mov eax, [ebx]
                and ax, OS_PAGING_FLAGS_MASK | PAGING_FLAG_PRESENT
                cmp al, PAGING_FLAG_PRESENT
                jne .skipPage

                cmp ah, (OS_PAGING_FLAG_THREAD_CONTEXT >> 8)
                je .removeMemory_threadContext

                cmp edx, 0
                jne .skipPage

                cmp ah, (OS_PAGING_FLAG_SHARED >> 8)
                je .skipPage
                cmp ah, (OS_PAGING_FLAG_KERNEL >> 8)
                je .skipPage
                
                .removeMemory_genericFreePage:
                    mov eax, [ebx]
                    call freeMemoryTablePage
                    jmp .skipPage

                .removeMemory_threadContext:
                    push ebx
                    shl ebx, 10
                    and bx, PAGING_LOW_BIT_REVERSE_MASK
                    call deleteThread
                    pop ebx
                    jmp .skipPage

                .skipPage:
                add ebx, 4
                cmp ebx, ecx
                jb .removeMemoryMapPTELoop

            .noPTEMapped:
            mov ebx, ecx
            add ecx, PAGE_SIZE
            cmp esi, edi
            jae .removeMemoryMapPDELoop_done
            add esi, 4
            jmp .removeMemoryMapPDELoop
        
        .removeMemoryMapPDELoop_done:

        cmp edx, 0
        je .removeMemoryDone

        .setupForPass2:
            xor ebx, ebx
            mov bl, dl
            shl ebx, PROCESS_VIRTUAL_MEMORY_MAP_ENRTY_SIZE_SHL_BITS
            add ebx, PROCESS_VIRTUAL_MEMORY_MAP_ADDRESS

            push edx
            mov edx, 0
            mov eax, KERNEL_PDE_ADDRESS
            mov cr3, eax

            jmp .removeMemoryPassStart

    .removeMemoryDone:

    .clearProcessTables:
        pop edx
        xor ebx, ebx
        mov bl, dl

        lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * ebx]
        call openSpinlock

        xor edx, edx
        mov [PROCESS_TABLE_ADDRESS + 4 * ebx], edx
        mov [PROCESS_PID_TABLE_ADDRESS + 4 * ebx], edx

        call closeSpinlock   

    .success:

        popa
        xor eax, eax
        inc eax
        ret

    .fail2:
        xor ebx, ebx
        mov bl, dl
        lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * ebx]
        call closeSpinlock

    .fail1:

        popa
        xor eax, eax
        ret