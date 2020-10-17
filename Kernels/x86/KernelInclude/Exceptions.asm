Exceptions_Handler_DIVIDE_BY_ZERO:
    pusha
    mov al, 0x00
    jmp Exceptions_DefaultHandler
Exceptions_Handler_DEBUG:
    pusha
    mov al, 0x00
    jmp Exceptions_DefaultHandler
Exceptions_Handler_NON_MASKABLE_INTERRUPT:
    pusha
    mov al, 0x02
    jmp Exceptions_DefaultHandler
Exceptions_Handler_BREAKPOINT:
    pusha
    mov al, 0x03
    jmp Exceptions_DefaultHandler
Exceptions_Handler_OVERFLOW:
    pusha
    mov al, 0x04
    jmp Exceptions_DefaultHandler
Exceptions_Handler_BOUND_RANGE_EXCEEDED:
    pusha
    mov al, 0x05
    jmp Exceptions_DefaultHandler
Exceptions_Handler_INVALID_OPCODE:
    pusha
    mov al, 0x06
    jmp Exceptions_DefaultHandler
Exceptions_Handler_DEVICE_NOT_AVAILABLE:
    pusha
    mov al, 0x07
    jmp Exceptions_DefaultHandler
Exceptions_Handler_DOUBLE_FAULT:
    pusha
    mov al, 0x08
    jmp Exceptions_DefaultHandler
Exceptions_Handler_COPROCESSOR_SEGMENT_OVERRUN:
    pusha
    mov al, 0x09
    jmp Exceptions_DefaultHandler
Exceptions_Handler_INVALID_TSS:
    pusha
    mov al, 0x0A
    jmp Exceptions_DefaultHandler
Exceptions_Handler_SEGMENT_NOT_PRESENT:
    pusha
    mov al, 0x0B
    jmp Exceptions_DefaultHandler
Exceptions_Handler_STACK_SEGMENT_FAULT:
    pusha
    mov al, 0x0C
    jmp Exceptions_DefaultHandler
Exceptions_Handler_GENERAL_PROTECTION_FAULT:
    pusha
    mov al, 0x0D
    jmp Exceptions_DefaultHandler
Exceptions_Handler_PAGE_FAULT:
    pusha
    mov al, 0x0E
    jmp Exceptions_DefaultHandler
Exceptions_Handler_X87_FPU_EXCPETION:
    pusha
    mov al, 0x10
    jmp Exceptions_DefaultHandler
Exceptions_Handler_ALIGNMENT_CHECK:
    pusha
    mov al, 0x11
    jmp Exceptions_DefaultHandler
Exceptions_Handler_MACHINE_CHECK:
    pusha
    mov al, 0x12
    jmp Exceptions_DefaultHandler
Exceptions_Handler_SIMD_FPU_EXCEPTION:
    pusha
    mov al, 0x13
    jmp Exceptions_DefaultHandler
Exceptions_Handler_VIRTUALIZATION_EXCEPTION:
    pusha
    mov al, 0x14
    jmp Exceptions_DefaultHandler
Exceptions_Handler_SECURITY_EXCEPTION:
    pusha
    mov al, 0x1E
    jmp Exceptions_DefaultHandler

Exceptions_DefaultHandler:
    mov edi, eax

    ; row 0
        ; error code
        mov ebx, 3 * 80
        mov [0xB8000 + 2 * ebx], byte 'E'
        add ebx, 2
        mov ecx, 4
        lea edx, [esp + 8 * 4]
        call debugging_PrintHex

        ; instruction pointer
        add ebx, 10
        mov [0xB8000 + 2 * ebx], byte 'I'
        add ebx, 2
        mov ecx, 4
        lea edx, [esp + 9 * 4]
        call debugging_PrintHex

        ; flags
        add ebx, 10
        mov [0xB8000 + 2 * ebx], byte 'F'
        add ebx, 2
        mov ecx, 4
        lea edx, [esp + 11 * 4]
        call debugging_PrintHex

    ; row 1
        mov ebx, 4 * 80
        mov [0xB8000 + 2 * ebx], byte 'A'
        add ebx, 2
        mov ecx, 4
        lea edx, [esp + 7 * 4]
        call debugging_PrintHex

        add ebx, 10
        mov [0xB8000 + 2 * ebx], byte 'B'
        add ebx, 2
        mov ecx, 4
        lea edx, [esp + 4 * 4]
        call debugging_PrintHex

        add ebx, 10
        mov [0xB8000 + 2 * ebx], byte 'C'
        add ebx, 2
        mov ecx, 4
        lea edx, [esp + 6 * 4]
        call debugging_PrintHex

        add ebx, 10
        mov [0xB8000 + 2 * ebx], byte 'D'
        add ebx, 2
        mov ecx, 4
        lea edx, [esp + 5 * 4]
        call debugging_PrintHex
    ;row 2
        mov ebx, 5 * 80
        mov [0xB8000 + 2 * ebx], byte 'S'
        add ebx, 2
        mov ecx, 4
        lea edx, [esp + 1 * 4]
        call debugging_PrintHex

        add ebx, 10
        mov [0xB8000 + 2 * ebx], byte 'D'
        add ebx, 2
        mov ecx, 4
        lea edx, [esp + 0 * 4]
        call debugging_PrintHex

        add ebx, 10
        mov [0xB8000 + 2 * ebx], byte 'b'
        add ebx, 2
        mov ecx, 4
        lea edx, [esp + 2 * 4]
        call debugging_PrintHex

        add ebx, 10
        mov [0xB8000 + 2 * ebx], byte 's'
        add ebx, 2
        mov ecx, 4
        lea edx, [esp + 12 * 4]
        call debugging_PrintHex
    ;row 3
        mov ebx, 6 * 80
        mov [0xB8000 + 2 * ebx], byte '0'
        add ebx, 2
        mov ecx, 4
        mov eax, cr0
        push eax
        mov edx, esp
        call debugging_PrintHex

        add ebx, 10
        mov [0xB8000 + 2 * ebx], byte '2'
        add ebx, 2
        mov ecx, 4
        mov eax, cr2
        mov [esp], eax
        call debugging_PrintHex

        add ebx, 10
        mov [0xB8000 + 2 * ebx], byte '3'
        add ebx, 2
        mov ecx, 4
        mov eax, cr3
        mov [esp], eax
        call debugging_PrintHex

        add ebx, 10
        mov [0xB8000 + 2 * ebx], byte '4'
        add ebx, 2
        mov ecx, 4
        mov eax, cr4
        mov [esp], eax
        call debugging_PrintHex
        pop eax

    ;row 4
        mov ebp, esp
        and bp, PROCESSOR_CONTEXT_BASE_BIT_MASK
        mov ebx, 7 * 80
        mov [0xB8000 + 2 * ebx], byte 'P'
        add ebx, 2
        mov ecx, 4
        lea edx, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
        call debugging_PrintHex

        add ebx, 10
        mov [0xB8000 + 2 * ebx], byte 'T'
        add ebx, 2
        mov ecx, 4
        lea edx, [ebp + PROCESSOR_CONTEXT_CURRENT_THREAD_PAGED_ADDRESS]
        call debugging_PrintHex
    
    mov eax, edi

    mov ebx, 0xEEEE0123
    mov ecx, [esp + 8 * 4]
    mov edx, [esp + 9 * 4]
    mov [0xB8000], byte 'E'
    mov [0xB8002], byte 'x'
    mov [0xB8004], byte '.'

    mov ebx, 4
    push eax
    mov edx, esp
    mov ecx, 1
    call debugging_PrintHex
    add esp, 4

    and eax, 0xFF
    or eax, 0xEEEE0000
    mov ebx, [esp + 9 * 4] ; error code
    mov ecx, [esp + 10 * 4] ; eip

    ;mov [0xB8002], al
    ;add [0xB8002], byte '0'
    jmp $

debugging_PrintHex:     ; ecx = iterations, ebx=offset, edx = data
    pusha
    add ebx, ecx
    add ebx, ecx
    dec ebx
    add ebx, ebx
    add ebx, 0xB8000

    .loop:
        mov al, [edx]
        and al, 0xF
        add al, '0'
        cmp al, '9'
        jbe .g1
        add al, 'A' - ('0' + 10)
        .g1:
        mov [ebx], al

        sub ebx, 2

        mov al, [edx]
        shr al, 4
        add al, '0'
        cmp al, '9'
        jbe .g2
        add al, 'A' - ('0' + 10)
        .g2:
        mov [ebx], al

        inc edx
        sub ebx, 2
        loop .loop
    
    popa
    ret