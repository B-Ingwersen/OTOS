createThread:                           ; edx = PID, ecx = threadContextLocation (in process), edi=entryPoint, esi=stackLocation
    pusha

    ;open process spinlock for access to modify 
    xor ebx, ebx
    mov bl, dl
    lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * ebx]
    call openSpinlock

    call checkPIDExists
    cmp eax, PID_DOESNT_EXISTS_CODE
    je createThread_failure

    mov ebx, ecx
    and cx, PAGING_LOW_BIT_REVERSE_MASK
    mov ecx, 1
    call checkMemoryRegionNotMapped
    cmp eax, MEMORY_CHECK_FAILURE_CODE
    je createThread_failure

    call mapPTEsForMemoryRegion
    or bx, OS_PAGING_FLAG_THREAD_CONTEXT | PAGING_FLAG_USER
    call mapNewMemoryForRegion

    ; zero threadContext page
    shr ebx, 10
    and bl, 0xFC
    xor eax, eax
    mov al, dl
    shl eax, 22
    mov eax, [eax + ebx + PROCESS_VIRTUAL_MEMORY_MAP_ADDRESS]
    ;call zeroPagePagingEnabled

    createThread_setInitialState: ; eax = page address, ebx = 
        push edi
        push esi

        push eax
        push ecx
        push edi

        mov ebx, eax
        mov eax, ZERO_PAGE_SPINLOCK
        call openSpinlock

        or bx, OS_PAGING_FLAG_GENERIC_PROTECTED | PAGING_FLAG_PRESENT | PAGING_FLAG_USER
        mov [KERNEL_PDE_ADDRESS + 12], ebx
        mov edi, KERNEL_PDE_RECURSIVE_ACCESS + 3 * PAGE_SIZE
        invlpg [edi]

        mov ecx, PAGE_SIZE / 4
        xor eax, eax
        rep stosd

        mov edi, KERNEL_PDE_RECURSIVE_ACCESS + 3 * PAGE_SIZE
        lea eax, [edi + THREAD_SPINLOCK]
        call openSpinlock

        mov [edi + THREAD_OWNER_PROCESS], edx
        mov [edi + THREAD_CURRENT_PROCESS], edx

        mov eax, [esp + 16]                             ; get entry point
        mov [edi + THREAD_INTERRUPT_STACK_SAVE + 7 * 4], eax
        mov eax, [esp + 12]                             ; get stack pointer
        mov [edi + THREAD_INTERRUPT_STACK_SAVE + 10 * 4], eax
        mov [edi + THREAD_INTERRUPT_STACK_SAVE + 0 * 4], eax
        mov [edi + THREAD_INTERRUPT_STACK_SAVE + 8 * 4], dword 0x1B
        mov [edi + THREAD_INTERRUPT_STACK_SAVE + 11 * 4], dword 0x23
        mov [edi + THREAD_INTERRUPT_STACK_SAVE + 9 * 4], dword 0x202

        fsave [edi + THREAD_INTERRUPT_FP_SAVE]

        lea eax, [edi + THREAD_SPINLOCK]
        call closeSpinlock

        ;mov edi, PROCESS_VIRTUAL_MEMORY_MAP_ENRTY_SIZE * 2 + 3 * PAGE_SIZE
        mov [edi + THREAD_SPINLOCK], dword 0
        mov [KERNEL_PDE_ADDRESS + 12], dword 0
        invlpg [edi]

        mov eax, ZERO_PAGE_SPINLOCK
        call closeSpinlock

        pop edi
        pop ecx
        pop eax
        
        add esp, 8

    xor ebx, ebx
    mov bl, dl
    lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * ebx]
    call closeSpinlock

    popa
    mov eax, CREATE_THREAD_SUCESS_CODE
    ret

    createThread_failure:

    xor ebx, ebx
    mov bl, dl
    lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * ebx]
    call closeSpinlock

    popa
    mov eax, CREATE_THREAD_FAILURE_CODE
    ret

changeThreadExecution:                              ; edx = PID, ebx = threadContextLocation, eax = operation
    pusha
    mov edi, eax                ; save eax in edi
    mov eax, cr3                ; save cr3
    push eax

    mov eax, edx                ; move to process's page table
    call enterJustProcess
    cmp eax, PROCESS_ENTER_SUCCESS
    jne .fail

    .threadChanges:
        mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_THREAD_MANAGEMENT_SPINLOCK ;open's target process's spinlock
        call openSpinlock

        .checkThreadContextExists:  ; self-explanitory
            mov eax, ebx
            shr eax, 20
            and al, 0xFC
            mov al, [RECURSIVE_PDE_MAP_BASE + eax]
            and al, 1
            cmp al, 0
            je .fail2

            mov eax, ecx
            shr eax, 10
            and al, 0xFC
            mov al, [RECURSIVE_PTE_MAP_BASE + eax]
            and al, 1
            cmp al, 0
            je .fail2

        mov eax, [ebx + THREAD_EXECUTION_STATE]     ; check for destroy flag; if true, abort
        and al, THREAD_EXECUTION_STATE_MASK_DESTROY
        cmp al, 0
        jne .fail2

        cmp edi, CHANGE_THREAD_EXECUTION_SCHEDULE   ; identify operation
        je .operationSchedule
        cmp edi, CHANGE_THREAD_EXECUTION_HALT
        je .operationHalt

        jmp .fail2

        .operationSchedule:
            mov ecx, [ebx + THREAD_EXECUTION_STATE] ; get flags
            and cl, THREAD_EXECUTION_STATE_BYTE_REVERSE_MASK_HALT_REQUESTED ; remove halt flag
            mov [ebx + THREAD_EXECUTION_STATE], ecx ; save modified flags
            mov al, cl
            and al, (THREAD_EXECUTION_STATE_MASK_EXECUTING | THREAD_EXECUTION_STATE_MASK_SCHEDULED) ; if currently executing/scheduled, no work to do
            cmp al, 0
            je .operationSchedule_addToScheduler

            or cl, THREAD_EXECUTION_STATE_MASK_RESCHEDULE_REQUESTED ; mark that we tried to reschedule 
            mov [ebx + THREAD_EXECUTION_STATE], ecx ; save modified flags
            jmp .success
            
            .operationSchedule_addToScheduler:
            mov eax, edx                            ; eax = pid
            call addThreadToScheduler
            or cl, THREAD_EXECUTION_STATE_MASK_SCHEDULED    ; mark that it was scheduled -- I forgot this initially, and it was a really annoying bug
            mov [ebx + THREAD_EXECUTION_STATE], ecx ; save modified flags

            jmp .success

        .operationHalt:
            mov ecx, [ebx + THREAD_EXECUTION_STATE]
            mov al, cl
            and cl, THREAD_EXECUTION_STATE_BYTE_REVERSE_MASK_RESCHEDULE_REQUESTED
            mov [ebx + THREAD_EXECUTION_STATE], ecx

            and al, THREAD_EXECUTION_STATE_MASK_RESCHEDULE_REQUESTED
            cmp al, 0
            jne .success

            or cl, THREAD_EXECUTION_STATE_MASK_HALT_REQUESTED
            mov [ebx + THREAD_EXECUTION_STATE], ecx

            jmp .success

        .success:

        mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_THREAD_MANAGEMENT_SPINLOCK
        call closeSpinlock

    pop ebx
    mov eax, edx
    call exitJustProcess

    popa
    xor eax, eax
    inc eax             ; eax = 1 = success code
    ret

    .fail2:
        mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_THREAD_MANAGEMENT_SPINLOCK
        call closeSpinlock

        mov ebx, [esp]
        mov eax, edx
        call exitJustProcess

    .fail:
        pop eax
        popa
        xor eax, eax    ; failure code

        ret

deleteThread:                                   ; ebx = threadContext, edx = PID, cr3 = already entered process
    push dword 0
    pusha

    xor eax, eax
    mov al, dl
    lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * eax]
    call openSpinlock

    mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_THREAD_MANAGEMENT_SPINLOCK
    call openSpinlock

    .checkThreadExists:
        and bx, PAGING_LOW_BIT_REVERSE_MASK
        mov eax, ebx
        shr eax, 20
        and al, 0xFC
        add eax, RECURSIVE_PDE_MAP_BASE
        cmp [eax], dword 0
        je .fail1

        mov eax, ebx
        shr eax, 10
        and al, 0xFC
        add eax, RECURSIVE_PTE_MAP_BASE
        mov eax, [eax]
        and ax, OS_PAGING_FLAG_THREAD_CONTEXT | PAGING_FLAG_PRESENT
        cmp ax, OS_PAGING_FLAG_THREAD_CONTEXT | PAGING_FLAG_PRESENT
        jne .fail1

    .checkNotMessageProcess:
        cmp [ebx + THREAD_MESSAGE_STACK_POINTER], dword 0
        jne .fail1
    .checkFlags:
        mov eax, [ebx + THREAD_EXECUTION_STATE]
        and al, THREAD_EXECUTION_STATE_MASK_DESTROY
        cmp al, 0
        jne .fail1

    .checkForScheduling:
        or byte [ebx + THREAD_EXECUTION_STATE], THREAD_EXECUTION_STATE_MASK_DESTROY
        mov eax, [ebx + THREAD_EXECUTION_STATE]
        and al, THREAD_EXECUTION_STATE_MASK_SCHEDULED
        cmp al, 0
        je .notScheduled

        mov eax, edx
        call removeFromScheduler
        cmp al, 0
        je .removeFromSchedulerFail
        and byte [ebx + THREAD_EXECUTION_STATE], THREAD_EXECUTION_STATE_BYTE_REVERSE_MASK_SCHEDULED
        .removeFromSchedulerFail:

        .notScheduled:

    .checkForExecution:
        cmp [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS], edx
        jne .notCurrentThread
        cmp [ebp + PROCESSOR_CONTEXT_CURRENT_THREAD_PAGED_ADDRESS], ebx
        jne .notCurrentThread

        and byte [ebx + THREAD_EXECUTION_STATE], THREAD_EXECUTION_STATE_BYTE_REVERSE_MASK_EXECUTING

        .notCurrentThread:
    
    .waitForRemoval:
        mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_THREAD_MANAGEMENT_SPINLOCK
        call closeSpinlock

        xor eax, eax
        mov al, dl
        lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * eax]
        call closeSpinlock

        .waitForRemovalLoop:
            nop
            nop
            nop
            nop

            mov eax, [ebx + THREAD_EXECUTION_STATE]
            and al, THREAD_EXECUTION_STATE_MASK_EXECUTING | THREAD_EXECUTION_STATE_MASK_SCHEDULED
            cmp al, 0
            jne .waitForRemovalLoop
    
    .removeThread:
        xor eax, eax
        mov al, dl
        lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * eax]
        call openSpinlock

        mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_THREAD_MANAGEMENT_SPINLOCK
        call openSpinlock

        mov ecx, ebx
        shr ecx, 10
        add ecx, RECURSIVE_PTE_MAP_BASE
        mov eax, [ecx]
        mov [ecx], dword 0
    
        mov ebx, cr3
        push ebx
        mov ebx, KERNEL_PDE_ADDRESS
        mov cr3, ebx

        call freeMemoryTablePage
    
        pop eax
        mov cr3, eax

    .success:
        mov [esp + 8 * 4], dword 1

    .fail1:

    mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_THREAD_MANAGEMENT_SPINLOCK
    call closeSpinlock

    xor eax, eax
    mov al, dl
    lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * eax]
    call closeSpinlock

    popa
    pop eax
    ret