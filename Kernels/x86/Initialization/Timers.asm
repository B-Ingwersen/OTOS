Timers_PIC_Initialize:
    pusha

    call GetKernelAddress_ECX

    mov ebx, [ecx + EXTERNAL_CALL_Timers_PIC_SendEOI]
    add ebx, ecx
    mov [TIMER_POINTER_TO_EOI_FUNCTION], ebx
    
    mov ebx, [ecx + EXTERNAL_CALL_Timers_PIC_Reload]
    add ebx, ecx
    mov [TIMER_POINTER_TO_RELOAD_FUNCTION], ebx

    mov al, 0xFF
    mov dx, 0xA1
    out dx, al

    mov al, 0xFE
    mov dx, 0x21
	out dx, al

    mov [TIMER_POINTER_TO_RELOAD_TIME], dword 0x1000

    mov al, 00110000b
    out 0x43, al    ;tell the PIT which channel we're setting

    popa
    ret

Timers_APIC_Initialize:
    pusha

    call GetKernelAddress_ECX

    mov ebx, [ecx + EXTERNAL_CALL_Timers_APIC_SendEOI]
    add ebx, ecx
    mov [TIMER_POINTER_TO_EOI_FUNCTION], ebx
    
    mov ebx, [ecx + EXTERNAL_CALL_Timers_APIC_Reload]
    add ebx, ecx
    mov [TIMER_POINTER_TO_RELOAD_FUNCTION], ebx

    pop eax
    mov ebx, INTERRUPT_ID_TIMER
    mov edi, [POINTER_TO_LOCAL_APIC_ADDRESS]
    mov [edi + LOCAL_APIC_INTERRUPT_LVT_TIMER_REGISTER], ebx
    mov ebx, 0xA
    mov [edi + LOCAL_APIC_INTERRUPT_TIMER_DIVIDER_REGISTER], ebx
    xor ebx, ebx
    mov [edi + LOCAL_APIC_TPR_REGISTER], ebx

    cmp esp, 0x8000
    ja .noCalibrate

    .calibrateAPICTimer:
        mov [edi + LOCAL_APIC_INTERRUPT_TIMER_INTIAL_COUNT], dword 0x80000000

        mov al, 00110110b
        out 0x43, al    ;tell the PIT which channel we're setting

        mov ax, 0xF000
        out 0x40, al    ;send low byte
        mov al, ah
        out 0x40, al    ;send high byte

        mov ecx, 10000
        .waitLoop:
            loop .waitLoop
        
        mov ebx, [edi + LOCAL_APIC_INTERRUPT_TIMER_CURRENT_COUNT]

        mov al, 00000000b    ; al = channel in bits 6 and 7, remaining bits clear
        out 0x43, al         ; Send the latch command
    
        xor eax, eax
        in al, 0x40          ; al = low byte of count
        mov ah, al           ; ah = low byte of count
        in al, 0x40          ; al = high byte of count
        rol ax, 8

        mov ecx, 0xF000
        sub ecx, eax
        mov eax, 0x80000000
        sub eax, ebx
        shl eax, 12
        xor edx, edx
        div ecx
        
        mov [TIMER_POINTER_TO_RELOAD_TIME], eax
    
    .noCalibrate:

    popa
    ret

Timers_Initialize:
    cmp [MULTIPROCESSOR_TYPE], dword MULTIPROCESSOR_TYPE_SMP
    je .apicTimer
    cmp [MULTIPROCESSOR_TYPE], dword MULTIPROCESSOR_TYPE_NO_SMP
    je .picTimer

    ret

    .picTimer:
        call Timers_PIC_Initialize
        ret

    .apicTimer:
        call Timers_APIC_Initialize
        ret

Timers_InsertInterrupt:
    pusha

    mov dl, 10001111b                              ; SWITCHED TO TASK GATE SO NO INTERRUPT DISABLE

    call GetKernelAddress_ECX
    mov eax, [ecx + EXTERNAL_CALL_TimeSliceInterrupt]
    add eax, ecx
    mov ebx, INTERRUPT_ID_TIMER
    call IDT_AddEntry

    popa
    ret