SystemCalls_Initialize:
    pusha

    mov dl, 11101111b                                           ; SWITCHED TO TASK GATE SO NO INTERRUPT DISABLE
    call GetKernelAddress_ECX
    
    mov eax, [ecx + EXTERNAL_CALL_MemorySystemCall]
    add eax, ecx
    mov ebx, INTERRUPT_ID_SYSTEM_CALL_MEMORY
    call IDT_AddEntry

    mov eax, [ecx + EXTERNAL_CALL_ProcessThreadSystemCall]
    add eax, ecx
    mov ebx, INTERRUPT_ID_SYSTEM_CALL_PROCESS_THREAD
    call IDT_AddEntry

    mov eax, [ecx + EXTERNAL_CALL_CapabilitiesSystemCall]
    add eax, ecx
    mov ebx, INTERRUPT_ID_SYSTEM_CALL_CAPABILITIES
    call IDT_AddEntry

    mov eax, [ecx + EXTERNAL_CALL_MessagingSystemCall]
    add eax, ecx
    mov ebx, INTERRUPT_ID_SYSTEM_CALL_MESSAGING
    call IDT_AddEntry

    mov eax, [ecx + EXTERNAL_CALL_SharedMemorySystemCall]
    add eax, ecx
    mov ebx, INTERRUPT_ID_SYSTEM_CALL_SHARED_MEMORY
    call IDT_AddEntry

    mov eax, [ecx + EXTERNAL_CALL_BiosSystemCall]
    add eax, ecx
    mov ebx, INTERRUPT_ID_SYSTEM_CALL_BIOS
    call IDT_AddEntry

    mov eax, [ecx + EXTERNAL_CALL_InterruptSystemCall]
    add eax, ecx
    mov ebx, INTERRUPT_ID_SYSTEM_CALL_INTERRUPT
    call IDT_AddEntry

    mov dl, 10001111b                                           ; don't allow user processes to call this one
                                                                ; SWITCHED TO TASK GATE SO NO INTERRUPT DISABLE
    mov eax, [ecx + EXTERNAL_CALL_BiosCallKernelInterrupt]
    add eax, ecx
    mov ebx, INTERRUPT_ID_BIOS_KERNEL_CALL
    call IDT_AddEntry

    popa
    ret