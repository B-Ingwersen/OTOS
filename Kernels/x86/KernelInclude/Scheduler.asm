getNextScheduledThread:                 ; return threadContext in eax, process in ebx
    push edx
    push ecx

    mov eax, SCHEDULER_TABLE_SPINLOCK
    call openSpinlock

    mov ebx, [SCHEDULER_TABLE_ENQUEUE]
    cmp ebx, [SCHEDULER_TABLE_DEQUEUE]
    je getNextScheduledThread_schedulerEmpty
    mov edx, [ebx]
    mov ecx, [ebx + 4]
    add ebx, 8

    cmp ebx, SCHEDULER_TABLES_END_ADDRESS
    jb getNextScheduledThread_noEnqueueWrap
    mov ebx, SCHEDULER_TABLE_BASE_ADDRESS
    getNextScheduledThread_noEnqueueWrap:

    mov [SCHEDULER_TABLE_ENQUEUE], ebx

    call closeSpinlock
    mov eax, edx
    mov ebx, ecx

    pop ecx
    pop edx

    ret

    getNextScheduledThread_schedulerEmpty:
    mov edx, SCHEDULER_EMPTY_CODE
    jmp getNextScheduledThread_noEnqueueWrap

addThreadToScheduler:                 ; threadContext in ebx, process in eax
    push edx
    push ecx

    mov edx, eax
    mov ecx, ebx
    mov eax, SCHEDULER_TABLE_SPINLOCK
    call openSpinlock

    mov ebx, [SCHEDULER_TABLE_DEQUEUE]
    mov [ebx], edx
    mov [ebx + 4], ecx
    add ebx, 8

    cmp ebx, SCHEDULER_TABLES_END_ADDRESS
    jb scheduleThread_noDequeueWrap
    mov ebx, SCHEDULER_TABLE_BASE_ADDRESS
    scheduleThread_noDequeueWrap:

    cmp ebx, [SCHEDULER_TABLE_ENQUEUE]
    je schedulerTableFail
    
    mov [SCHEDULER_TABLE_DEQUEUE], ebx

    call closeSpinlock
    mov ebx, ecx
    mov eax, edx

    pop ecx
    pop edx

    ret

removeFromScheduler:                ; threadContext in ebx, process in eax
    pusha

    mov edx, eax
    mov ecx, ebx

    mov eax, SCHEDULER_TABLE_SPINLOCK
    call openSpinlock

    mov eax, [SCHEDULER_TABLE_ENQUEUE]

    .searchThroughScheduler:
        cmp [eax], edx
        jne .continueLoop
        cmp [eax + 4], ecx
        je .entryFound

        .continueLoop:
        add eax, 8
        cmp eax, SCHEDULER_TABLES_END_ADDRESS
        jb .noWrap

        mov eax, SCHEDULER_TABLE_BASE_ADDRESS

        .noWrap:
        cmp eax, [SCHEDULER_TABLE_DEQUEUE]
        je .notFound

        jmp .searchThroughScheduler
    
    .entryFound:
        mov [eax], dword SCHEDULER_ENTRY_REMOVED_CODE

        mov eax, SCHEDULER_TABLE_SPINLOCK
        call closeSpinlock

        popa
        xor eax, eax
        inc eax
        ret

    .notFound:

        mov eax, SCHEDULER_TABLE_SPINLOCK
        call closeSpinlock

        popa
        xor eax, eax
        ret

schedulerTableFail:
    mov eax, 'SCHD'
    mov ebx, 'FAIL'
    xor ecx, ecx
    xor edx, edx
    mov [0xB8000], byte 'S'
    jmp $

getNextScheduledInterrupt:                 ; return interrupt # in eax
    push edx
    push ebx

    mov eax, INTERRUPT_SCHEDULER_TABLE_ENQUEUE_SPINLOCK
    call openSpinlock

    mov ebx, [INTERRUPT_SCHEDULER_TABLE_ENQUEUE]
    cmp ebx, [INTERRUPT_SCHEDULER_TABLE_DEQUEUE]
    je .schedulerEmpty
    mov edx, [ebx]
    add ebx, 4

    cmp ebx, INTERRUPT_SCHEDULER_TABLE_END_ADDRESS
    jb .noEnqueueWrap
    mov ebx, INTERRUPT_SCHEDULER_TABLE_BASE_ADDRESS
    .noEnqueueWrap:

    mov [INTERRUPT_SCHEDULER_TABLE_ENQUEUE], ebx

    call closeSpinlock
    mov eax, edx

    pop ebx
    pop edx

    ret

    .schedulerEmpty:
    mov edx, SCHEDULER_EMPTY_CODE
    jmp .noEnqueueWrap

scheduleInterrupt:                          ; interrupt number in eax
    push edx
    push ebx
    push ecx

    mov edx, eax
    mov eax, INTERRUPT_SCHEDULER_TABLE_DEQUEUE_SPINLOCK
    call openSpinlock

    mov ebx, [INTERRUPT_SCHEDULER_TABLE_DEQUEUE]
    mov ecx, ebx
    add ebx, 4

    cmp ebx, [INTERRUPT_SCHEDULER_TABLE_ENQUEUE]
    je .queueFull                                       ; for interrupts, we just drop them if the queue is full!

    cmp ebx, INTERRUPT_SCHEDULER_TABLE_END_ADDRESS
    jb .noDequeueWrap
    mov ebx, INTERRUPT_SCHEDULER_TABLE_BASE_ADDRESS
    .noDequeueWrap:

    mov [ecx], edx
    mov [INTERRUPT_SCHEDULER_TABLE_DEQUEUE], ebx

    .queueFull:

    call closeSpinlock
    mov eax, edx

    pop ecx
    pop ebx
    pop edx

    ret