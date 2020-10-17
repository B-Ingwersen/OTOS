
biosCallKernelInterrupt:
    pusha

    mov ebp, esp
    and bp, PROCESSOR_CONTEXT_BASE_BIT_MASK
    call Timers_APIC_GetCurrentCount

    cli

    .disableExternalInterrupts:
        cmp [MULTIPROCESSOR_TYPE], dword MULTIPROCESSOR_TYPE_SMP
        jne .noDisableAPIC

        mov ebx, [LOCAL_APIC_PAGED_LOCATION + LOCAL_APIC_SPURIOUS_INTERRUPT_REGISTER_OFFSET]
        and bh, 0xFE
        mov [LOCAL_APIC_PAGED_LOCATION + LOCAL_APIC_SPURIOUS_INTERRUPT_REGISTER_OFFSET], ebx

        .noDisableAPIC:
        .disablePIC:
            mov dx, 0x21		; reset the masks to how they were before
            mov al, 0x00
            out dx, al
            mov dx, 0xA1
            mov al, 0xff
            out dx, al

            mov dx, 0x21		; mask everything -- don't want any pesky interrupts showing up
            mov al, 0xFF
            out dx, al

    xor eax, eax
    mov [BIOS_CALL_SUCCESS], eax

    call callBIOS

    ; EXPERIMENTAL -- FLUSH PS2
        ;cli
        mov ecx, 0x1234
        .loop:
            in al, 0x64
            and al, 1
            cmp al, 0
            je .loop_end
            in al, 0x60
            jmp .loop

        inc ecx
        
        .loop_end:
        
        ;call .fixPS2_waitForWriteReady
        ;    mov al, 0x20
        ;    out 0x64, al
        ;inc ecx
        ;    call .fixPS2_waitForCommandResponse
        ;    in al, 0x60
        ;    mov bl, al
        ;    and bl, 0xBF
        ;inc ecx
        ;    call .fixPS2_waitForWriteReady
        ;    mov al, 0x60
        ;    out 0x64, al
        ;inc ecx
        ;    call .fixPS2_waitForWriteReady
        ;    mov al, bl
        ;    out 0x60, al
        ;    call .fixPS2_waitForWriteReady
    ; END EXPERIMENTAL

    .reenableExternalInterrupts:
        cmp [MULTIPROCESSOR_TYPE], dword MULTIPROCESSOR_TYPE_SMP
        jne .reenable_legacy

        .disablePIC2:
            mov dx, 0x21		; reset the masks to how they were before
            mov al, 0x00
            out dx, al
            mov dx, 0xA1
            mov al, 0xff
            out dx, al

            mov dx, 0x21		; mask everything -- don't want any pesky interrupts showing up
            mov al, 0xFF
            out dx, al

            mov eax, 0x10000
            mov [LOCAL_APIC_PAGED_LOCATION + LOCAL_APIC_LVT_LINT0], eax
            mov [LOCAL_APIC_PAGED_LOCATION + LOCAL_APIC_LVT_LINT1], eax

            and [LOCAL_APIC_PAGED_LOCATION + LOCAL_APIC_TPR_REGISTER], dword 0xFF000000
            mov [LOCAL_APIC_PAGED_LOCATION + LOCAL_APIC_ESR_REGISTER], dword 0
            mov [LOCAL_APIC_PAGED_LOCATION + LOCAL_APIC_DFR_REGISTER], dword 0xF0000000
            and [LOCAL_APIC_PAGED_LOCATION + LOCAL_APIC_LDR_REGISTER], dword 0x00FFFFFF

            mov ebx, [LOCAL_APIC_PAGED_LOCATION + LOCAL_APIC_SPURIOUS_INTERRUPT_REGISTER_OFFSET]
            or bh, 0x11
            mov bl, 0xFF
            mov [LOCAL_APIC_PAGED_LOCATION + LOCAL_APIC_SPURIOUS_INTERRUPT_REGISTER_OFFSET], ebx
            
            mov ebx, INTERRUPT_ID_TIMER
            mov [LOCAL_APIC_PAGED_LOCATION + LOCAL_APIC_INTERRUPT_LVT_TIMER_REGISTER], ebx
            mov ebx, 0xA
            mov [LOCAL_APIC_PAGED_LOCATION + LOCAL_APIC_INTERRUPT_TIMER_DIVIDER_REGISTER], ebx
            xor ebx, ebx
            mov [LOCAL_APIC_PAGED_LOCATION + LOCAL_APIC_TPR_REGISTER], ebx

            ;sti                ;TEMPORARY INTERRUPT FIX
            jmp .reenable_done ;TEMPORARY INTERRUPT FIX

        .reenable_legacy:
            sti
            mov dx, 0x21		; reset the masks to how they were before
            mov al, 0x00
            out dx, al
            mov dx, 0xA1
            mov al, 0x00
            out dx, al

            mov dx, 0x21		; unmask all -- interrupts are routed to Generic Handlers & Will Be Ignored Until Entries Are Added
            mov al, 0x00
            out dx, al

        .reenable_done:

    mov ebp, esp
    and bp, PROCESSOR_CONTEXT_BASE_BIT_MASK
    mov eax, [TIMER_POINTER_TO_RELOAD_FUNCTION]
    call eax

    inc byte [BIOS_CALL_SUCCESS]
    mov eax, 1
    mov [BIOS_CALL_OPERATION_STATUS], eax

	.sendAPIC_EOI:
		xor ebx, ebx
		xor ecx, ecx
		mov bl, INTERRUPT_ID_BIOS_KERNEL_CALL
		mov cl, bl
		and cl, 0x1F				; get 32 bit offset
		shr ebx, 5
		shl ebx, 4
		add ebx, 0x100 + LOCAL_APIC_PAGED_LOCATION	; ISR register offset of the local apic
		mov edx, [ebx]
		shr edx, cl
		and dl, 1
		cmp dl, 0
		je .APICNoEOINeeded

    	mov [LOCAL_APIC_PAGED_LOCATION + LOCAL_APIC_EOI_REGISTER], dword 0
	
		.APICNoEOINeeded:

    popa
    iretd
    jmp systemCallEnd

.fixPS2_waitForWriteReady:
    in al, 0x64
    and al, 2
    cmp al, 0
    jne .fixPS2_waitForWriteReady
    ret

.fixPS2_waitForCommandResponse:
    in al, 0x64
    and al, 1
    cmp al, 1
    jne .fixPS2_waitForCommandResponse
    ret

[bits 32]

callBIOS:
    .saveAndMoveStack:
        mov [BIOS_CALL_REAL_MODE_STACK - 4], esp
        mov esp, BIOS_CALL_REAL_MODE_STACK - 4

    .savePaging:
        mov eax, cr3
        push eax

    .fillInJumpLocations:
        call .getLocation
        .getLocation:
        pop edi
        sub edi, .getLocation

        mov eax, edi
        add eax, nextJump1
        mov [edi + JumpAddress1], eax
        mov ax, di
        add ax, nextJump2
        mov [edi + JumpAddress2], ax
        mov ax, di
        add ax, nextJump3
        mov [edi + JumpAddress3], ax
        mov ax, di
        add ax, nextJump4
        mov [edi + JumpAddress4], eax

    .saveRegisters:
        mov eax, cr0
        mov [BIOS_CALL_SAVE_CR0], eax
        and eax, 0x7FFFFFFF
        mov cr0, eax

        mov ax, ds
        mov [BIOS_CALL_SAVE_DS], ax
        mov ax, es
        mov [BIOS_CALL_SAVE_ES], ax
        mov ax, fs
        mov [BIOS_CALL_SAVE_FS], ax
        mov ax, gs
        mov [BIOS_CALL_SAVE_GS], ax

    .finishLeavingPaging:
        mov eax, 0
        mov cr3, eax

    .enter16Bit:
        lgdt [REAL_MODE_GDT_DESCRIPTOR]

        db 0xEA
        JumpAddress1:   dd 0
        dw 0x0008
        nextJump1:

        [bits 16]

        mov ax, DATA_SEG
        mov ds, ax
        mov es, ax
        mov fs, ax
        mov gs, ax
        mov ss, ax

        lidt [REAL_MODE_IDT_DESCRIPTOR]
        sti

    .enterRealMode:
        mov eax, cr0
        and eax, 0xFFFFFFFE
        mov cr0, eax

        db 0xEA
        JumpAddress2:   dw 0
        dw 0x0000

        nextJump2:

        mov ax, 0
        mov ds, ax
        mov fs, ax
        mov gs, ax
        mov ss, ax
        mov ax, 0x1000
        mov es, ax

    .executeInterrupt:
        mov eax, [BIOS_CALL_INTERRUPT]
        mov [di + interruptAddress], al

        mov edi, [BIOS_CALL_EDI]
        mov esi, [BIOS_CALL_ESI]
        mov ebp, [BIOS_CALL_EBP]
        mov ebx, [BIOS_CALL_EBX]
        mov edx, [BIOS_CALL_EDX]
        mov ecx, [BIOS_CALL_ECX]
        mov eax, [BIOS_CALL_EAX]

        ;push word [BIOS_CALL_EFLAGS]
        push word 0x0202
        popf

        db 0xCD
        interruptAddress:
        db 0x00

        pushf
        pop word [BIOS_CALL_EFLAGS]

        mov [BIOS_CALL_EDI], edi
        mov [BIOS_CALL_ESI], esi
        mov [BIOS_CALL_EBP], ebp
        mov [BIOS_CALL_EBX], ebx
        mov [BIOS_CALL_EDX], edx
        mov [BIOS_CALL_ECX], ecx
        mov [BIOS_CALL_EAX], eax

    .returnToProtectedMode:
        cli
        mov eax, [START_PROCESSOR_GDTR_TRAMPOLINE]
        lgdt [eax]

        mov eax, cr0	; cr0 can't be directly modified, so it's first moved to a register
        or eax, 0x1	; the first bit is changed to 1
        mov cr0, eax	; cr0 is updated, and now we are in 32-bit protected mode

        db 0xEA
        JumpAddress3: dw 0
        dw 0x08

        nextJump3:
        [bits 32]

        mov ax, DATA_SEG 	; segment registers are redirected to what's defined in the GDT
        mov ss, ax

        lgdt [PROCESSOR_CONTEXTS_MAX_ADDRESS - PROCESSOR_CONTEXT_SIZE + PROCESSOR_CONTEXT_GDT_DESCRIPTOR_OFFSET]

        mov ax, DATA_SEG
        mov ds, ax
        mov ax, DATA_SEG
        mov es, ax
        mov ax, DATA_SEG
        mov fs, ax
        mov ax, DATA_SEG
        mov gs, ax

        db 0xEA
        JumpAddress4:   dd 0
        dw 0x08
        nextJump4:

        mov ax, [BIOS_CALL_SAVE_DS]
        mov ds, ax
        mov ax, [BIOS_CALL_SAVE_ES]
        mov es, ax
        mov ax, [BIOS_CALL_SAVE_FS]
        mov fs, ax
        mov ax, [BIOS_CALL_SAVE_GS]
        mov gs, ax

        lidt [IDT_DESCRIPTOR_ADDRESS]

    .restorePagingAndStack:
        pop eax
        mov cr3, eax

        mov eax, [BIOS_CALL_SAVE_CR0]
        mov cr0, eax

        pop esp

        ret

CODE_SEG equ 0x08
DATA_SEG equ 0x10
