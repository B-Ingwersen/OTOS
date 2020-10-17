Timers_APIC_SendEOI:
    push eax

    mov eax, 0
    mov [LOCAL_APIC_PAGED_LOCATION + LOCAL_APIC_EOI_REGISTER], eax

    pop eax
    ret

Timers_APIC_Reload:
    push eax

    ;mov eax, [TIMER_POINTER_TO_RELOAD_TIME]
    mov eax, [ebp + PROCESSOR_CONTEXT_THREAD_TIME]
    mov [LOCAL_APIC_PAGED_LOCATION + LOCAL_APIC_INTERRUPT_TIMER_INTIAL_COUNT], eax

    pop eax
    ret

Timers_APIC_GetCurrentCount:
    push eax

    mov eax, [LOCAL_APIC_PAGED_LOCATION + LOCAL_APIC_INTERRUPT_TIMER_CURRENT_COUNT]
    mov [ebp + PROCESSOR_CONTEXT_THREAD_TIME], eax

    pop eax
    ret

Timers_PIC_SendEOI:
    push eax
    push edx

    mov dx, 0x20			;send EOI signal to to let the cpu know the interrupt is acknowledged
	mov al, 0x20
	out dx, al

    pop edx
    pop eax
    ret

Timers_PIC_Reload:
    push eax
    push edx

    ;mov al, 00110000b
    ;out 0x43, al    ;tell the PIT which channel we're setting

    mov ax, [TIMER_POINTER_TO_RELOAD_TIME]
    out 0x40, al    ;send low byte
    mov al, ah
    out 0x40, al    ;send high byte

    pop edx
    pop eax
    ret

