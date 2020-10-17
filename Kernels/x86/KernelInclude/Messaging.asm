
initializeMessaging:                                                    ; ebx = intended base
    pusha

    mov edx, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]

    xor eax, eax
    mov al, dl
    lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * eax]
    call openSpinlock
    mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_MESSAGING_SPINLOCK
    call openSpinlock

    mov eax, [PROCESS_INFO_ADDRESS + PROCESS_INFO_MESSAGING_TABLE_BASE_ADDRESS]
    cmp eax, 0
    jne .fail1

    .moveToKernelPDE:
        mov eax, cr3
        push eax
        mov eax, KERNEL_PDE_ADDRESS
        mov cr3, eax

    .getMemMapHandleAndCheckPID:
        call checkPIDExists
        cmp eax, PID_EXISTS_CODE
        jne .fail2

    and bx, PAGING_LOW_BIT_REVERSE_MASK
    mov ecx, MESSAGING_REGION_N_PAGES
    call checkMemoryRegionNotMapped
    cmp eax, MEMORY_CHECK_SUCESS_CODE
    jne .fail2

    call mapPTEsForMemoryRegion
    or bx, OS_PAGING_FLAG_GENERIC_PROTECTED
    call changeFlagsForMemoryRegion

    .success:
        pop eax
        mov cr3, eax

        mov [PROCESS_INFO_ADDRESS + PROCESS_INFO_MESSAGING_TABLE_BASE_ADDRESS], ebx

        mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_MESSAGING_SPINLOCK
        call closeSpinlock
        xor eax, eax
        mov al, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
        lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * eax]
        call closeSpinlock

        popa
        xor eax, eax
        inc eax
        ret

    .fail2:
    .returnFromKernelPTE:
        pop eax
        mov cr3, eax
    
    .fail1:

    mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_MESSAGING_SPINLOCK
    call closeSpinlock
    xor eax, eax
    mov al, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
    lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * eax]
    call closeSpinlock

    popa
    xor eax, eax
    ret

destroyMessaging:
    pusha

    mov edx, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]

    mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_MESSAGING_SPINLOCK
    call openSpinlock

    mov eax, [PROCESS_INFO_ADDRESS + PROCESS_INFO_MESSAGING_TABLE_BASE_ADDRESS]
    cmp eax, 0
    je .end

    mov edx, eax
    shr edx, 10
    and dl, 0xFC
    add edx, RECURSIVE_PTE_MAP_BASE
    mov ecx, MESSAGING_N_SLOTS
    .destroyMessagingLoop:
        mov eax, [edx]
        and al, PAGING_FLAG_PRESENT
        cmp al, 0
        je .destroyMessagingLoop_continue

        .destroyThread:
            ;mov eax, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
            mov ebx, [edx]

            .removeThreadFromScheduler:
                mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_THREAD_MANAGEMENT_SPINLOCK
                call openSpinlock

                or byte [ebx + THREAD_EXECUTION_STATE], THREAD_EXECUTION_STATE_MASK_DESTROY
                
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
                    and al, THREAD_EXECUTION_STATE_MASK_SCHEDULED
                    cmp al, 0
                    jne .waitForRemovalLoop

            mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_THREAD_MANAGEMENT_SPINLOCK
            call openSpinlock
            call closeSpinlock

            .removeAndRescheduleThread:
                mov eax, cr3
                push eax

                mov eax, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
                call messageReturn
                cmp ebx, 0
                je .noMessageThreadSchedule

                call addThreadToScheduler

                pop ebx
                call exitJustProcess

                jmp .removeAndRescheduleThread_done

                .noMessageThreadSchedule:
                pop eax
                mov cr3, eax

                .removeAndRescheduleThread_done:

        .destroyMessagingLoop_continue:
        
        xor eax, eax
        mov [edx], eax
        mov [edx + 4], eax

        add edx, 8
        dec ecx
        cmp ecx, 0
        jg .destroyMessagingLoop
    .end:

    mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_MESSAGING_SPINLOCK
    call closeSpinlock

    popa
    ret


setMessagingHandlerFunction:
    push eax

    mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_MESSAGING_SPINLOCK
    call openSpinlock

    mov eax, [PROCESS_INFO_ADDRESS + PROCESS_INFO_MESSAGING_TABLE_BASE_ADDRESS]
    cmp eax, 0
    je .fail

    mov [PROCESS_INFO_ADDRESS + PROCESS_INFO_MESSAGING_TABLE_HANDLER_ADDRESS], ebx
    xor eax, eax
    inc eax

    .fail:
    push eax
    mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_MESSAGING_SPINLOCK
    call closeSpinlock
    pop eax

    pop eax
    ret

messageProcess:                                                     ; ebx = messagePage, edx = PID
    pusha

    and bx, PAGING_LOW_BIT_REVERSE_MASK

    xor eax, eax
    mov al, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
    lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * eax]
    call openSpinlock
    mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_THREAD_MANAGEMENT_SPINLOCK
    call openSpinlock

    .checkDataPageMapped:
        mov eax, ebx
        shr eax, 20
        and al, 0xFC
        mov al, [RECURSIVE_PDE_MAP_BASE + eax]
        and al, 1
        cmp al, 0
        je .fail1

        mov eax, ebx
        shr eax, 10
        and al, 0xFC
        mov ax, [RECURSIVE_PTE_MAP_BASE + eax]
        and ax, PAGING_FLAG_PRESENT | PAGING_FLAG_USER | OS_PAGING_FLAGS_MASK
        cmp ax, PAGING_FLAG_PRESENT | PAGING_FLAG_USER
        jne .fail1
    
    .checkThreadDestroyHaltFlags:
        mov eax, [ebp + PROCESSOR_CONTEXT_CURRENT_THREAD_PAGED_ADDRESS]
        mov eax, [eax + THREAD_EXECUTION_STATE]
        and al, (THREAD_EXECUTION_STATE_MASK_HALT_REQUESTED | THREAD_EXECUTION_STATE_MASK_DESTROY)
        cmp al, 0
        jne .fail1
    
    .saveInMessageStack:
        mov eax, [ebp + PROCESSOR_CONTEXT_CURRENT_THREAD_PAGED_ADDRESS]
        mov ecx, [eax + THREAD_MESSAGE_STACK_POINTER]
        cmp ecx, THREAD_MESSAGE_STACK_MAX_SIZE
        jae .fail1

        add [eax + THREAD_MESSAGE_STACK_POINTER], dword MESSAGE_STACK_ENTRY_SIZE
        lea ecx, [eax + ecx + THREAD_MESSAGE_STACK_BASE]
        mov eax, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
        mov [ecx], eax
        mov eax, [ebp + PROCESSOR_CONTEXT_CURRENT_THREAD_PAGED_ADDRESS]
        mov [ecx + 4], eax
        mov [ecx + 8], ebx
        mov [ecx + 16], esi                                                             ; return eip
        mov [ecx + 20], edi                                                             ; return esp
    
    .removeThreadPagesFromCallProcess:
        mov eax, ebx
        shr eax, 10
        and al, 0xFC
        add eax, RECURSIVE_PTE_MAP_BASE
        mov esi, [eax]                                                              ; esi = dataPage physical address
        mov [eax], dword OS_PAGING_FLAG_GENERIC_PROTECTED

        mov eax, [ebp + PROCESSOR_CONTEXT_CURRENT_THREAD_PAGED_ADDRESS]
        shr eax, 10
        and al, 0xFC
        add eax, RECURSIVE_PTE_MAP_BASE
        mov edi, [eax]                                                              ; edi = threadContext physical address
        mov [eax], dword OS_PAGING_FLAG_THREAD_CONTEXT

    .saveDataPagePhysicalAddress:
        mov [ecx + 12], esi
    
    .tryEnterMessageProcess:
        mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_THREAD_MANAGEMENT_SPINLOCK
        call closeSpinlock
        xor eax, eax
        mov al, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
        lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * eax]
        call closeSpinlock

        mov eax, cr3
        push eax                                                                    ; STACK + 4 !!!!
        mov eax, edx
        call enterJustProcess
        cmp eax, PROCESS_ENTER_SUCCESS
        jne .fail2

        xor eax, eax
        mov al, dl
        lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * eax]
        call openSpinlock

        mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_MESSAGING_SPINLOCK
        call openSpinlock

    .checkForMessageHandler:
        cmp [PROCESS_INFO_ADDRESS + PROCESS_INFO_MESSAGING_TABLE_HANDLER_ADDRESS], dword 0
        je .fail3

    .getOpenSlotInTable:
        push ebx

        mov ecx, [PROCESS_INFO_ADDRESS + PROCESS_INFO_MESSAGING_TABLE_BASE_ADDRESS]
        shr ecx, 10
        and cl, 0xFC
        add ecx, RECURSIVE_PTE_MAP_BASE
        mov ebx, 0
        .lookForOpenSlot:
            mov eax, [ecx]
            and al, PAGING_FLAG_PRESENT
            cmp al, 0
            je .slotFound

            add ecx, 8
            inc ebx
            cmp ebx, MESSAGING_N_SLOTS
            jb .lookForOpenSlot
            jmp .fail4

        .slotFound:
        pop ebx
    
    .insertPages:
        or di, OS_PAGING_FLAG_THREAD_CONTEXT
        or si, OS_PAGING_FLAG_GENERIC_PROTECTED
        ;mov [ecx], edi
        ;mov [ecx + 4], esi
        mov [ecx], esi                              ; data page BELOW
        mov [ecx + 4], edi                          ; thread context ABOVE
        shl ecx, 10
        invlpg [ecx]
        invlpg [ecx + PAGE_SIZE]
    
    .recordChanges:
        mov [ecx + THREAD_CURRENT_PROCESS], edx
        mov eax, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
        mov ebx, cr3
        call exitJustProcess

        add ecx, PAGE_SIZE
        mov [ebp + PROCESSOR_CONTEXT_CURRENT_THREAD_PAGED_ADDRESS], ecx
        sub ecx, PAGE_SIZE
        mov eax, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
        mov [ecx + 2048 - 8], eax                                           ; first is the PID
        mov [ecx + 2048 - 4], ecx                                           ; second is the base address
        mov [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS], edx

    .success:
        push dword 0x23
        lea eax, [ecx + 2048 - 12]
        push eax
        push dword 0x0202       ; default flags
        push dword 0x1B
        push dword [PROCESS_INFO_ADDRESS + PROCESS_INFO_MESSAGING_TABLE_HANDLER_ADDRESS]

        mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_MESSAGING_SPINLOCK
        call closeSpinlock

        xor eax, eax
        mov al, dl
        lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * eax]
        call closeSpinlock

        lea ebp, [ecx + 2048]
        xor eax, eax
        mov ebx, eax
        mov ecx, eax
        mov edx, eax
        mov edi, eax
        mov esi, eax

        jmp systemCallEnd
    
    .fail4:
        pop ebx

    .fail3:
        mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_MESSAGING_SPINLOCK
        call closeSpinlock

        xor eax, eax
        mov al, dl
        lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * eax]
        call closeSpinlock

        push ebx
        mov ebx, [esp + 4]
        mov eax, edx
        call exitJustProcess
        pop ebx

    .fail2:
        add esp, 4

        xor eax, eax
        mov al, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
        lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * eax]
        call openSpinlock
        mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_THREAD_MANAGEMENT_SPINLOCK
        call openSpinlock

        mov eax, ebx
        shr eax, 10
        and al, 0xFC
        add eax, RECURSIVE_PTE_MAP_BASE
        mov [eax], esi

        mov eax, [ebp + PROCESSOR_CONTEXT_CURRENT_THREAD_PAGED_ADDRESS]
        shr eax, 10
        and al, 0xFC
        add eax, RECURSIVE_PTE_MAP_BASE
        mov [eax], edi

        mov eax, [ebp + PROCESSOR_CONTEXT_CURRENT_THREAD_PAGED_ADDRESS]
        sub [eax + THREAD_MESSAGE_STACK_POINTER], dword MESSAGE_STACK_ENTRY_SIZE
    
    .fail1:
        mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_THREAD_MANAGEMENT_SPINLOCK
        call closeSpinlock
        xor eax, eax
        mov al, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
        lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * eax]
        call closeSpinlock

        popa
        xor eax, eax
        ret

messageReturn:
    push eax                        ; curent process
    push ebx                        ; current thread
    pusha                                                                                   ; STACK + 32

    xor esi, esi
    .openPMEM2_1:
        xor eax, eax
        mov al, [esp + 32 + 4]; current process
        lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * eax]
        call openSpinlock
    .openTM2_1:
        mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_THREAD_MANAGEMENT_SPINLOCK
        call openSpinlock

    .getMessageStackInfo:
        mov eax, [esp + 32]; current thread
        mov ecx, [eax + THREAD_MESSAGE_STACK_POINTER]
        cmp ecx, 0
        jle .messageStackEmpty

        sub ecx, MESSAGE_STACK_ENTRY_SIZE
        mov [eax + THREAD_MESSAGE_STACK_POINTER], ecx

        lea eax, [eax + ecx + THREAD_MESSAGE_STACK_BASE]
        mov edi, [eax + 12]                                                                 ; edi = dataPage physical address
        mov ebx, [eax + 8]                                                                  ; ebx = dataPage paged location
        mov ecx, [eax + 4]                                                                  ; ecx = threadContext paged location
        mov edx, [eax]                                                                      ; edx = PID

    .closeTM2_1:
        mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_THREAD_MANAGEMENT_SPINLOCK
        call closeSpinlock
    .closePMEM2_1:
        xor eax, eax
        mov al, [esp + 32 + 4]; current process
        lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * eax]
        call closeSpinlock
    
    .tryAddingProccess1Reference:
        mov eax, edx
        call addProcessReference
        cmp eax, PROCESS_ENTER_SUCCESS
        je .enterProcessSuccess

            mov ebx, [esp + 32]
            shr ebx, 10
            and bl, 0xFC
            add ebx, RECURSIVE_PTE_MAP_BASE
            lea esi, [ebx - 4]                      ; esi = location to unmap page

            mov eax, cr3
            push eax
            mov eax, KERNEL_PDE_ADDRESS
            mov cr3, eax

            mov eax, edi
            call freeMemoryTablePage
            
            pop eax
            mov cr3, eax

            jmp .openPMEM2_1

    .enterProcessSuccess:
    .openPMEM2_2:
        xor eax, eax
        mov al, [esp + 32 + 4]; current process
        lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * eax]
        call openSpinlock
    .openTM2_2:
        mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_THREAD_MANAGEMENT_SPINLOCK
        call openSpinlock

    .unmapFromMessageProcess:
        push ebx
        mov ebx, [esp + 36]; current thread
        shr ebx, 10
        and bl, 0xFC
        add ebx, RECURSIVE_PTE_MAP_BASE
        mov esi, [ebx]                                                                          ; esi = threadContext physical page
        mov eax, OS_PAGING_FLAG_GENERIC_PROTECTED
        mov [ebx], eax
        mov [ebx - 4], eax
        pop ebx
    
    .closeTM2_2:
        mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_THREAD_MANAGEMENT_SPINLOCK
        call closeSpinlock
    .closePMEM2_2:
        xor eax, eax
        mov al, [esp + 32 + 4]; current process
        lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * eax]
        call closeSpinlock
    
    .exitProcess2:
        push ebx
        mov eax, [esp + 36 + 4]
        xor ebx, ebx
        mov bl, dl
        mov ebx, [PROCESS_TABLE_ADDRESS + 4 * ebx]
        and bx, PAGING_LOW_BIT_REVERSE_MASK
        call exitJustProcess 
        pop ebx                                                                          ; STACK - 4
    
    .openPMEM1:
        xor eax, eax
        mov al, dl
        lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * eax]
        call openSpinlock
    .openTM1:
        mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_THREAD_MANAGEMENT_SPINLOCK
        call openSpinlock
 
    .mapDataPageAndThreadContext:
        mov eax, ecx
        shr eax, 10
        and al, 0xFC
        add eax, RECURSIVE_PTE_MAP_BASE
        mov [eax], esi
        invlpg [ecx]

        mov eax, ebx
        shr eax, 10
        and al, 0xFC
        add eax, RECURSIVE_PTE_MAP_BASE
        mov [eax], edi
        invlpg [ebx]

        mov eax, [ecx + THREAD_MESSAGE_STACK_POINTER]
        lea eax, [ecx + THREAD_MESSAGE_STACK_BASE + eax]
        mov esi, [eax + 16]
        mov edi, [eax + 20]
    
    .recordChanges:
        mov [ecx + THREAD_CURRENT_PROCESS], edx
        mov [esp + 32 + 4], edx; current thread
        mov [esp + 32], ecx; current process

    ; STOPPED HERE!
    
    .resetExecutionVariables:
        mov [ecx + THREAD_INTERRUPT_STACK_SAVE + 7 * 4], esi
        mov [ecx + THREAD_INTERRUPT_STACK_SAVE + 8 * 4], dword 0x1B
        mov [ecx + THREAD_INTERRUPT_STACK_SAVE + 9 * 4], dword 0x202
        mov [ecx + THREAD_INTERRUPT_STACK_SAVE + 10 * 4], edi
        mov [ecx + THREAD_INTERRUPT_STACK_SAVE + 11 * 4], dword 0x23

        mov [ecx + THREAD_INTERRUPT_STACK_SAVE + 6 * 4], dword 1                ; put success result (1) in eax;

    .closeTM1:
        mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_THREAD_MANAGEMENT_SPINLOCK
        call closeSpinlock
    .closePMEM1:
        xor eax, eax
        mov al, dl
        lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * eax]
        call closeSpinlock

    .resetStack:
        popa
        pop ebx
        pop eax

        ret
    
    .messageStackEmpty:
        cmp esi, 0
        je .messageStackEmpty_NoDataPage
            mov [esi], dword OS_PAGING_FLAG_GENERIC_PROTECTED
            .messageStackEmpty_NoDataPage:

        .messageStackEmpty_GetThreadContextPhysicalPage:
            mov ebx, [esp + 32]; current thread
            shr ebx, 10
            and bl, 0xFC
            add ebx, RECURSIVE_PTE_MAP_BASE
            mov edx, [ebx]
            mov [ebx], dword 0                      ; remove from process
        
        .removeSpinlocks:
            mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_THREAD_MANAGEMENT_SPINLOCK
            call closeSpinlock

            xor eax, eax
            mov al, [esp + 32 + 4]; current process
            lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * eax]
            call closeSpinlock

        .messageStackEmpty_ToKernelPDE:
            mov eax, KERNEL_PDE_ADDRESS
            mov cr3, eax
        .messageStackEmpty_FreeThreadContextPage:
            mov eax, edx
            call freeMemoryTablePage
        .messageStackEmpty_RemovePREF2:
            mov eax, [esp + 32 + 4]; process
            call removeProcessReference

        mov ebp, esp
        and bp, PROCESSOR_CONTEXT_BASE_BIT_MASK
        mov edx, 0xDEAD0002
        jmp enterSchedulerNoReschedule