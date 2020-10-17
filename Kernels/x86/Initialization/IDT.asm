IDT_AddEntry: 			; handler in eax, int# in ebx, flags in edx
	pusha

	shl ebx, 3
	add ebx, IDT_BASE_ADDRESS

	mov [ebx], ax		; put lower half of address at start
	shr eax, 16
	mov [ebx + 6], ax		; put upper half of adress at this point
	mov ax, 0x0008
	mov [ebx + 2], ax		; kernal offset??
	mov al, 0x00
	mov [ebx + 4], al		; necessary 0 section
	mov [ebx + 5], dl

	popa
	ret

IDT_Initialize:
	pusha

    .disablePIC:
        mov al, 0xff
        out 0xa1, al
        out 0x21, al

    .fillInIDTDescriptor:
        mov [IDT_DESCRIPTOR_ADDRESS], word 2047
        mov [IDT_DESCRIPTOR_ADDRESS + 2], dword IDT_BASE_ADDRESS

    .zeroTheIDT:
        xor eax, eax
        mov edi, IDT_BASE_ADDRESS
        mov ecx, 2048
        rep movsb
    
    call IDT_SetupGenericHandlers
    call Exceptions_Initialize

    .loadTheIDT:
        cli			; clear interrupts, load the idt, reinstate interrupts
        mov eax, IDT_DESCRIPTOR_ADDRESS
        lidt [IDT_DESCRIPTOR_ADDRESS]

	popa
	ret

IDT_SetupGenericHandlers:
    pusha

    call GetKernelAddress_ECX
    add ecx, [ecx + EXTERNAL_CALL_IDT_GenericHandlerRedirect]

    mov eax, IDT_GENERIC_HANDLER_REDIRECT_TABLE
    xor ebx, ebx
    mov dl, 10001110b

    .loop:
        mov edi, ecx
        sub edi, eax
        sub edi, 8                  ; relative jump to IDT_GenericHandlerRedirect

        ;Produces machine code for:
        ;   pusha
        ;   mov al, {IDT Entry #}
        ;   jmp IDT_GenericHandlerRedirect

        mov [eax], dword 0xE900B060
        mov [eax + 2], bl
        mov [eax + 4], edi

        call IDT_AddEntry

        add eax, 8
        inc ebx
        cmp ebx, 256
        jb .loop

    popa
    ret

Exceptions_Initialize:
    pusha

    mov dl, 10001111b                                       ; SWITCHED TO TASK GATE SO NO INTERRUPT DISABLE
    call GetKernelAddress_ECX

    mov eax, [ecx + EXTERNAL_CALL_Exceptions_Handler_DIVIDE_BY_ZERO]
    add eax, ecx
    mov ebx, INTERRUPT_ID_DIVIDE_BY_ZERO
    call IDT_AddEntry

    mov eax, [ecx + EXTERNAL_CALL_Exceptions_Handler_DEBUG]
    add eax, ecx
    mov ebx, INTERRUPT_ID_DEBUG
    call IDT_AddEntry
    
    mov eax, [ecx + EXTERNAL_CALL_Exceptions_Handler_NON_MASKABLE_INTERRUPT]
    add eax, ecx
    mov ebx, INTERRUPT_ID_NON_MASKABLE_INTERRUPT
    call IDT_AddEntry

    mov eax, [ecx + EXTERNAL_CALL_Exceptions_Handler_BREAKPOINT]
    add eax, ecx
    mov ebx, INTERRUPT_ID_BREAKPOINT
    call IDT_AddEntry

    mov eax, [ecx + EXTERNAL_CALL_Exceptions_Handler_OVERFLOW]
    add eax, ecx
    mov ebx, INTERRUPT_ID_OVERFLOW
    call IDT_AddEntry

    mov eax, [ecx + EXTERNAL_CALL_Exceptions_Handler_BOUND_RANGE_EXCEEDED]
    add eax, ecx
    mov ebx, INTERRUPT_ID_BOUND_RANGE_EXCEEDED
    call IDT_AddEntry

    mov eax, [ecx + EXTERNAL_CALL_Exceptions_Handler_INVALID_OPCODE]
    add eax, ecx
    mov ebx, INTERRUPT_ID_INVALID_OPCODE
    call IDT_AddEntry

    mov eax, [ecx + EXTERNAL_CALL_Exceptions_Handler_DEVICE_NOT_AVAILABLE]
    add eax, ecx
    mov ebx, INTERRUPT_ID_DEVICE_NOT_AVAILABLE
    call IDT_AddEntry

    mov eax, [ecx + EXTERNAL_CALL_Exceptions_Handler_DOUBLE_FAULT]
    add eax, ecx
    mov ebx, INTERRUPT_ID_DOUBLE_FAULT
    call IDT_AddEntry

    mov eax, [ecx + EXTERNAL_CALL_Exceptions_Handler_COPROCESSOR_SEGMENT_OVERRUN]
    add eax, ecx
    mov ebx, INTERRUPT_ID_COPROCESSOR_SEGMENT_OVERRUN
    call IDT_AddEntry

    mov eax, [ecx + EXTERNAL_CALL_Exceptions_Handler_INVALID_TSS]
    add eax, ecx
    mov ebx, INTERRUPT_ID_INVALID_TSS
    call IDT_AddEntry

    mov eax, [ecx + EXTERNAL_CALL_Exceptions_Handler_SEGMENT_NOT_PRESENT]
    add eax, ecx
    mov ebx, INTERRUPT_ID_SEGMENT_NOT_PRESENT
    call IDT_AddEntry

    mov eax, [ecx + EXTERNAL_CALL_Exceptions_Handler_GENERAL_PROTECTION_FAULT]
    add eax, ecx
    mov ebx, INTERRUPT_ID_GENERAL_PROTECTION_FAULT
    call IDT_AddEntry

    mov eax, [ecx + EXTERNAL_CALL_Exceptions_Handler_PAGE_FAULT]
    add eax, ecx
    mov ebx, INTERRUPT_ID_PAGE_FAULT
    call IDT_AddEntry

    mov eax, [ecx + EXTERNAL_CALL_Exceptions_Handler_X87_FPU_EXCPETION]
    add eax, ecx
    mov ebx, INTERRUPT_ID_X87_FPU_EXCPETION
    call IDT_AddEntry

    mov eax, [ecx + EXTERNAL_CALL_Exceptions_Handler_ALIGNMENT_CHECK]
    add eax, ecx
    mov ebx, INTERRUPT_ID_ALIGNMENT_CHECK
    call IDT_AddEntry

    mov eax, [ecx + EXTERNAL_CALL_Exceptions_Handler_MACHINE_CHECK]
    add eax, ecx
    mov ebx, INTERRUPT_ID_MACHINE_CHECK
    call IDT_AddEntry

    mov eax, [ecx + EXTERNAL_CALL_Exceptions_Handler_SIMD_FPU_EXCEPTION]
    add eax, ecx
    mov ebx, INTERRUPT_ID_SIMD_FPU_EXCEPTION
    call IDT_AddEntry

    mov eax, [ecx + EXTERNAL_CALL_Exceptions_Handler_VIRTUALIZATION_EXCEPTION]
    add eax, ecx
    mov ebx, INTERRUPT_ID_VIRTUALIZATION_EXCEPTION
    call IDT_AddEntry

    mov eax, [ecx + EXTERNAL_CALL_Exceptions_Handler_SECURITY_EXCEPTION]
    add eax, ecx
    mov ebx, INTERRUPT_ID_SECURITY_EXCEPTION
    call IDT_AddEntry

    popa
    ret