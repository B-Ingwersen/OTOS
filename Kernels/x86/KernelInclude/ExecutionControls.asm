
timeSliceInterrupt:
    sti

    push eax
    push ebx
    push ecx
    push edx
    push edi
    push esi
    push ebp

    mov eax, [TIMER_POINTER_TO_EOI_FUNCTION]
    call eax

    ; reset stack and check for halt state
    mov ebp, esp
    and bp, PROCESSOR_CONTEXT_BASE_BIT_MASK
    ;mov eax, [ebp + PROCESSOR_CONTEXT_EXECUTION_STATE]
    ;cmp eax, PROCESSOR_EXECUTION_STATE_HALTED
    ;je timeSliceInterrupt_haltInterrupt

    ; check if coming from user mode
    mov dl, [esp + THREAD_STACK_CODE_SEG_LOCATION]
    and dl, 0xF8
    cmp dl, 0x8
    jne timeSliceInterrupt_userModeInterrupt

    ; check if coming from Ring 3 enter zone
    mov eax, [esp + THREAD_STACK_EIP_LOCATION]
    cmp eax, RING3_ENTER_ZONE_START
    jb timeSliceInterrupt_kernelModeInterrupt
    cmp eax, RING3_ENTER_ZONE_END
    jae timeSliceInterrupt_kernelModeInterrupt

    timeSliceInterrupt_ring3ZoneInterrupt:
        jmp enterScheduler_Reschedule

    timeSliceInterrupt_kernelModeInterrupt:
        mov [ebp + PROCESSOR_CONTEXT_THREAD_TIME], dword 0
        pop ebp
        pop esi
        pop edi
        pop edx
        pop ecx
        pop ebx
        pop eax
        iretd

    timeSliceInterrupt_userModeInterrupt:               ; save the
        mov eax, [ebp + PROCESSOR_CONTEXT_CURRENT_THREAD_PAGED_ADDRESS]

        mov ecx, THREAD_STACK_SAVE_SIZE / 4
        mov esi, esp
        lea edi, [eax + THREAD_INTERRUPT_STACK_SAVE]
        rep movsd
        add esp, THREAD_STACK_SAVE_SIZE
        
        fsave [eax + THREAD_INTERRUPT_FP_SAVE]

        jmp enterScheduler_Reschedule

    timeSliceInterrupt_haltInterrupt:
        lea esp, [ebp + PROCESSOR_CONTEXT_STACK_BASE]
        jmp enterScheduler_HaltInterrupt

enterThreadExecution:                   ; eax is thread context (in current paging paradigm), ebp points to Processor Context
    ; save stack; ad8b
    .checkIfOkayToEnter:
        mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_THREAD_MANAGEMENT_SPINLOCK
        call openSpinlock

        mov ebx, [ebp + PROCESSOR_CONTEXT_CURRENT_THREAD_PAGED_ADDRESS]
        mov ecx, [ebx + THREAD_EXECUTION_STATE]
        mov al, cl
        and al, (THREAD_EXECUTION_STATE_MASK_HALT_REQUESTED | THREAD_EXECUTION_STATE_MASK_DESTROY)
        cmp al, 0
        je .okayToEnter
            ;and cl, (THREAD_EXECUTION_STATE_BYTE_REVERSE_MASK_SCHEDULED & THREAD_EXECUTION_STATE_BYTE_REVERSE_MASK_EXECUTING & THREAD_EXECUTION_STATE_BYTE_REVERSE_MASK_HALT_REQUESTED)
            ;mov [ebx + THREAD_EXECUTION_STATE], ecx
            mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_THREAD_MANAGEMENT_SPINLOCK
            call closeSpinlock

            jmp enterScheduler_exitProcess
        .okayToEnter:

        and cl, (THREAD_EXECUTION_STATE_BYTE_REVERSE_MASK_SCHEDULED & THREAD_EXECUTION_STATE_BYTE_REVERSE_MASK_HALT_REQUESTED)
        mov [ebx + THREAD_EXECUTION_STATE], ecx

        mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_THREAD_MANAGEMENT_SPINLOCK
        call closeSpinlock
    mov eax, [ebp + PROCESSOR_CONTEXT_CURRENT_THREAD_PAGED_ADDRESS]
    
    .setIOPrivilageLevel:
        cmp [PROCESS_INFO_ADDRESS + PROCESS_INFO_PERMISSIONS_BASE + PROCESS_PERMISSION_IO_PORTS], dword 1
        je .setIOPrivilageLevel_Ring3
        and [eax + THREAD_INTERRUPT_STACK_SAVE + 9 * 4], word 0xCFFF
        jmp .setIOPrivilageLevel_End
        .setIOPrivilageLevel_Ring3:
        or [eax + THREAD_INTERRUPT_STACK_SAVE + 9 * 4], word 0x3000
        .setIOPrivilageLevel_End:

    ; copy general purpose registers and iretd structure back to the stack
    lea esi, [eax + THREAD_INTERRUPT_STACK_SAVE]
    lea edi, [esp - THREAD_STACK_SAVE_SIZE]
    mov ecx, THREAD_STACK_SAVE_SIZE / 4
    sub esp, THREAD_STACK_SAVE_SIZE
    rep movsd

    ; restore the floating point registers
    frstor [eax + THREAD_INTERRUPT_FP_SAVE]

    RING3_ENTER_ZONE_START:
    cmp [ebp + PROCESSOR_CONTEXT_THREAD_TIME], dword 0
    je enterScheduler_Reschedule

    mov cx, 0x23
    mov ds, cx
    mov es, cx
    mov fs, cx
    mov gs, cx

    pop ebp
    pop esi
    pop edi
    pop edx
    pop ecx
    pop ebx
    pop eax

    or [esp + 8], word EFLAGS_INTERRUPT_ENABLE_MASK
    sti
    iretd

    RING3_ENTER_ZONE_END:
    jmp $

enterScheduler:
    enterScheduler_Reschedule:
    enterScheduler_exitProcess:
        lea esp, [ebp + PROCESSOR_CONTEXT_STACK_BASE]

        mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_THREAD_MANAGEMENT_SPINLOCK
        call openSpinlock

        mov ebx, [ebp + PROCESSOR_CONTEXT_CURRENT_THREAD_PAGED_ADDRESS]
        mov ecx, [ebx + THREAD_EXECUTION_STATE]
        and cl, THREAD_EXECUTION_STATE_BYTE_REVERSE_MASK_SCHEDULED
        mov al, cl
        and al, (THREAD_EXECUTION_STATE_MASK_HALT_REQUESTED | THREAD_EXECUTION_STATE_MASK_DESTROY)
        cmp al, 0
        jne .noReschedule

        mov eax, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
        mov ebx, [ebp + PROCESSOR_CONTEXT_CURRENT_THREAD_PAGED_ADDRESS]
        call addThreadToScheduler

        or cl, THREAD_EXECUTION_STATE_MASK_SCHEDULED

        .noReschedule:

        and cl, (THREAD_EXECUTION_STATE_BYTE_REVERSE_MASK_HALT_REQUESTED & THREAD_EXECUTION_STATE_BYTE_REVERSE_MASK_EXECUTING)
        mov [ebx + THREAD_EXECUTION_STATE], ecx

        mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_THREAD_MANAGEMENT_SPINLOCK
        call closeSpinlock

        mov eax, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
        mov ebx, KERNEL_PDE_ADDRESS
        call exitJustProcess

    enterScheduler_HaltInterrupt:

enterSchedulerNoReschedule:
    mov eax, cr3
    cmp eax, 0x71000
    jne THE_FAIL

    enterScheduler_interruptCheck:                                          ; TODO!!!!!

        cmp [ebp + PROCESSOR_CONTEXT_INTS_PROCESSED], dword 16
        jae enterScheduler_getNextSchedulerEntry

        call getNextScheduledInterrupt
        cmp eax, SCHEDULER_EMPTY_CODE
        je enterScheduler_getNextSchedulerEntry

        inc dword [ebp + PROCESSOR_CONTEXT_INTS_PROCESSED]
        call IDT_ProcessScheduledInterrupt
        jmp enterScheduler_interruptCheck

    enterScheduler_getNextSchedulerEntry:
        mov [ebp + PROCESSOR_CONTEXT_INTS_PROCESSED], dword 0

        call getNextScheduledThread                                         ; puts PID in eax, threadContext in ebc
        cmp eax, SCHEDULER_EMPTY_CODE
        je setHaltMode
        cmp eax, SCHEDULER_ENTRY_REMOVED_CODE
        je enterScheduler_getNextSchedulerEntry
    
    enterScheduler_enterProcess:
        mov ecx, eax                                                        ; save process in ecx

        .enterProcessChecks:
            xor edx, edx
            mov dl, cl

            lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * edx]
            call openSpinlock

            mov eax, [PROCESS_PID_TABLE_ADDRESS + 4 * edx]
            mov al, 0
            mov edx, ecx
            mov dl, al
            cmp eax, edx
            jne enterScheduler_PIDDoesntExist
                ; TODO: should this just be a Kernel Panic? (Can a deleted process ever be allowed to have entries in the scheduler queue?)

            xor edx, edx
            mov dl, cl
            mov eax, [PROCESS_TABLE_ADDRESS + 4 * edx]
            and ax, PROCESS_DELETE_FLAG
            cmp ax, 0
            jne enterScheduler_handleDestroyFlag

            ; check destroy flag, jump to enterScheduler_handleDestroyFlag
            xor edx, edx
            mov dl, cl
            add byte [PROCESS_PID_TABLE_ADDRESS + 4 * edx], 1

            mov eax, [PROCESS_TABLE_ADDRESS + 4 * edx]
            and ax, PAGING_LOW_BIT_REVERSE_MASK
            mov cr3, eax

            lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * edx]
            call closeSpinlock

        ;cmp eax, PROCESS_ENTER_SUCCESS
        ;jne enterScheduler_getNextSchedulerEntry                            ; TODO!!!! -- BIG ONE -- IF THE PROCESS IS BEING DESTROYED, MAKE SURE WE CAN STILL GET TO THE THREAD TO SET IT AS NOT-SCHEDULED FIRST!!!!

        mov [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS], ecx
        mov [ebp + PROCESSOR_CONTEXT_CURRENT_THREAD_PAGED_ADDRESS], ebx

    enterScheduler_InterruptMessageEnter:

        mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_THREAD_MANAGEMENT_SPINLOCK
        call openSpinlock

        mov ecx, [ebx + THREAD_EXECUTION_STATE]
        and cl, THREAD_EXECUTION_STATE_BYTE_REVERSE_MASK_SCHEDULED
        or cl, THREAD_EXECUTION_STATE_MASK_EXECUTING
        mov [ebx + THREAD_EXECUTION_STATE], ecx

        call closeSpinlock
    enterScheduler_setTimerSlice:
        mov eax, [TIMER_POINTER_TO_RELOAD_TIME]
        mov [ebp + PROCESSOR_CONTEXT_THREAD_TIME], eax

        mov eax, [TIMER_POINTER_TO_RELOAD_FUNCTION]
        call eax

    mov [ebp + PROCESSOR_CONTEXT_EXECUTION_STATE], dword PROCESSOR_EXECUTION_STATE_EXECUTING

    enterScheduler_enterThread:
        jmp enterThreadExecution

;FAILURE STATES
    enterScheduler_PIDDoesntExist:
        lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * edx]
        call openSpinlock
        jmp enterScheduler_getNextSchedulerEntry

    enterScheduler_handleDestroyFlag:
        mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_THREAD_MANAGEMENT_SPINLOCK
        call openSpinlock
        or byte [ebx + THREAD_EXECUTION_STATE], THREAD_EXECUTION_STATE_BYTE_REVERSE_MASK_SCHEDULED      ; gets rid of the scheduled flag in the Thread Context
        call closeSpinlock

        mov eax, KERNEL_PDE_ADDRESS                             ; leave the process page table before releasing spinlock
        mov cr3, eax
        jmp enterScheduler_PIDDoesntExist
    
setHaltMode:
    mov eax, PROCESSOR_EXECUTION_STATE_HALTED
    mov [ebp + PROCESSOR_CONTEXT_EXECUTION_STATE], eax

    mov eax, [TIMER_POINTER_TO_RELOAD_TIME]
    mov [ebp + PROCESSOR_CONTEXT_THREAD_TIME], eax
    mov eax, [TIMER_POINTER_TO_RELOAD_FUNCTION]
    call eax

    .loop:
        mov eax, 0x1234678
        hlt

        jmp timeSliceInterrupt_haltInterrupt
        jmp .loop

THE_FAIL:
    mov ecx, [esp + 28]
    mov edx, [esp + 32]
    mov edx, [esp + 40]
    mov ebx, 0x899
    jmp $

THE_FAIL2:
    mov ebx, 0x90
    jmp $