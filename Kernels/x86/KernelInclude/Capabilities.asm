
processCapabilitiesOperations:
    push ebx
    push ecx
    push edi
    push esi
    push eax

    mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_CAPABILITIES_SPINLOCK
    call openSpinlock

    ;mov edx, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
    mov edi, [PROCESS_INFO_ADDRESS + PROCESS_INFO_CAPABILITIES_BASE_ADDRESS]
    mov esi, [PROCESS_INFO_ADDRESS + PROCESS_INFO_CAPABILITIES_N_PAGES]

    mov eax, cr3
    push eax
    mov eax, KERNEL_PDE_ADDRESS
    mov cr3, eax

    mov eax, [esp + 4]
    cmp eax, SYSTEM_CALL_CAPABILITIES_SETUP
    je .setup
    cmp eax, SYSTEM_CALL_CAPABILITIES_DELETE
    je .delete
    
    xor eax, eax
    mov [esp + 4], eax
    jmp .nothingFound

    .setup:
        cmp edi, 0
        jne .nothingFound

        and bx, PAGING_LOW_BIT_REVERSE_MASK
        or bx, PAGING_FLAG_WRITE | PAGING_FLAG_USER | OS_PAGING_FLAG_GENERIC_PROTECTED
        mov eax, MEMORY_SYSTEM_CALL_NEW_MEMORY
        call mapNewOrKernelMemoryIntoProcess
        mov [esp + 4], eax

        pop eax
        mov cr3, eax

        mov eax, [esp + 4]
        cmp eax, MEMORY_SYSTEM_CALL_SUCCESS
        jne .end

        mov [PROCESS_INFO_ADDRESS + PROCESS_INFO_CAPABILITIES_BASE_ADDRESS], ebx
        mov [PROCESS_INFO_ADDRESS + PROCESS_INFO_CAPABILITIES_N_PAGES], ecx

        jmp .end

    .delete:
        cmp edi, 0
        je .nothingFound

        mov ebx, edi
        mov ecx, esi
        or ebx, OS_PAGING_FLAG_GENERIC_PROTECTED
        call removeMemoryFromProcess

        pop eax
        mov cr3, eax

        mov eax, [esp + 4]
        cmp eax, MEMORY_SYSTEM_CALL_SUCCESS
        jne .end

        xor ebx, ebx
        mov [PROCESS_INFO_ADDRESS + PROCESS_INFO_CAPABILITIES_BASE_ADDRESS], ebx
        mov [PROCESS_INFO_ADDRESS + PROCESS_INFO_CAPABILITIES_N_PAGES], ebx

        jmp .end
    
    .nothingFound:

        pop eax
        mov cr3, eax
    
    .end:

    mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_CAPABILITIES_SPINLOCK
    call closeSpinlock

    pop eax
    pop esi
    pop edi
    pop ecx
    pop ebx
    ret

checkCapability:                        ; before hand, in this order, push the current process, system call #, capability #
                                        ; eax-edx, esi, edi = system call values, ebp = PID of to check process
    push eax
    push ebx
    push ecx
    push edx
    push esi
    push edi

    mov eax, cr3
    push eax
    mov eax, ebp
    call enterJustProcess
    cmp eax, PROCESS_ENTER_SUCCESS
    jne .fail1

    ;TODO

    mov eax, ebp
    pop ebx
    call exitJustProcess

    .fail1:

    pop eax

    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
    add esp, 4

    ret