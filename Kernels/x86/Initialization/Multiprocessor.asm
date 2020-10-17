Multiprocessor_Initialize:
    push eax
    push ebx
    push ecx
    push edx
    push esi
    push edi

    ;Setup Real Mode GDT
        mov [REAL_MODE_GDT + 0x00], dword 0x00000000
        mov [REAL_MODE_GDT + 0x04], dword 0x00000000
        mov [REAL_MODE_GDT + 0x08], dword 0x0000FFFF
        mov [REAL_MODE_GDT + 0x0C], dword 0x003F9A00
        mov [REAL_MODE_GDT + 0x10], dword 0x0000FFFF
        mov [REAL_MODE_GDT + 0x14], dword 0x003F9200

        mov [REAL_MODE_GDT_DESCRIPTOR], word 0x17
        mov [REAL_MODE_GDT_DESCRIPTOR + 2], dword REAL_MODE_GDT

        mov [REAL_MODE_IDT_DESCRIPTOR], word 0x03FF
        mov [REAL_MODE_IDT_DESCRIPTOR + 2], dword 0

    ;FOR BIOS CALLS
        call Thunk_ECX
        mov ebx, [ecx + LoadLocationTable_BootGDTDescriptor]
        mov [START_PROCESSOR_GDTR_TRAMPOLINE], ebx
    
    ; FOR DEBUGGING -- DISABLE SMP WITH LINE BELOW
    ;jmp .NoSMP_Setup

    call InterruptIO_TryACPISetup
    cmp al, 1
    je .SMP_Setup

    call InterruptIO_TryMPSetup
    cmp al, 1
    je .SMP_Setup

    .NoSMP_Setup:
        mov [MULTIPROCESSOR_TYPE], dword MULTIPROCESSOR_TYPE_NO_SMP
        call InterruptIO_LegacySetup
        jmp .setupBootProcessor

    .SMP_Setup:
        call setupMultiprocessor
        ;call InterruptIO_LegacySetup ; TEMPORARY INTERRUPT FIX
        jmp .setupBootProcessor

    .setupBootProcessor:

        mov eax, PROCESSOR_CONTEXTS_MAX_ADDRESS - PROCESSOR_CONTEXT_SIZE
        call setupProcessorContext
        .copyStack:
            lea edi, [eax + PROCESSOR_CONTEXT_STACK_BASE - 7 * 4]
            mov esi, esp
            mov ecx, 7
            rep movsd
            lea esp, [eax + PROCESSOR_CONTEXT_STACK_BASE - 7 * 4]
            mov ebp, eax

    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
    pop eax

    ret

setupProcessorContext:                                  ; eax=processor context base address
    pusha

    .setupGDT:
        lea ebx, [eax + PROCESSOR_CONTEXT_GDT_OFFSET]
        mov [eax + PROCESSOR_CONTEXT_GDT_DESCRIPTOR_OFFSET + 2], ebx
        mov [eax + PROCESSOR_CONTEXT_GDT_DESCRIPTOR_OFFSET], word (GDT_SIZE - 1)
        
        mov [eax + PROCESSOR_CONTEXT_GDT_OFFSET + 8], word 0xFFFF
        mov [eax + PROCESSOR_CONTEXT_GDT_OFFSET + 13], word 0xCF9A
        mov [eax + PROCESSOR_CONTEXT_GDT_OFFSET + 16], word 0xFFFF
        mov [eax + PROCESSOR_CONTEXT_GDT_OFFSET + 21], word 0xCF92
        mov [eax + PROCESSOR_CONTEXT_GDT_OFFSET + 24], word 0xFFFF
        mov [eax + PROCESSOR_CONTEXT_GDT_OFFSET + 29], word 0xCFFA
        mov [eax + PROCESSOR_CONTEXT_GDT_OFFSET + 32], word 0xFFFF
        mov [eax + PROCESSOR_CONTEXT_GDT_OFFSET + 37], word 0xCFF2

        .insertTSSEntry:
            mov [eax + PROCESSOR_CONTEXT_GDT_OFFSET + 45], byte 0xE9
            lea ebx, [eax + PROCESSOR_CONTEXT_TSS_OFFSET]
            mov ecx, ebx
            add ecx, TSS_SIZE
            mov [eax + PROCESSOR_CONTEXT_GDT_OFFSET + 40], cx
            shr ecx, 16
            or [eax + PROCESSOR_CONTEXT_GDT_OFFSET + 46], cl
            mov edx, ebx
            and edx, 0xFFFFFF
            or [eax + PROCESSOR_CONTEXT_GDT_OFFSET + 42], edx
            shr ebx, 24
            mov [eax + PROCESSOR_CONTEXT_GDT_OFFSET + 47], bl

    .insertTSSEntries:
        lea ebx, [eax + PROCESSOR_CONTEXT_STACK_BASE]
        mov [eax + PROCESSOR_CONTEXT_TSS_OFFSET + 4], ebx
        mov [eax + PROCESSOR_CONTEXT_TSS_OFFSET + 8], dword 0x10
        mov [eax + PROCESSOR_CONTEXT_TSS_OFFSET + 0x64], dword TSS_SIZE

    lea ebx, [eax + PROCESSOR_CONTEXT_GDT_DESCRIPTOR_OFFSET]
    lgdt [ebx]

    .tssFlush:
        mov ax, 0x2B
        ltr ax
    
    .setupTimer:
        call Timers_Initialize

    popa
    ret

setupMultiprocessor:
    pusha

    mov edi, [POINTER_TO_LOCAL_APIC_ADDRESS]
    .initializeLocalAPIC:
        mov ebx, [edi + LOCAL_APIC_SPURIOUS_INTERRUPT_REGISTER_OFFSET]
        or bh, 0x11
        mov bl, 0xFF
        mov [edi + LOCAL_APIC_SPURIOUS_INTERRUPT_REGISTER_OFFSET], ebx  ; TODO -- map spurious interrupts

        mov eax, 0x10000
        mov [edi + LOCAL_APIC_LVT_LINT0], eax
        mov [edi + LOCAL_APIC_LVT_LINT1], eax

        and [edi + LOCAL_APIC_TPR_REGISTER], dword 0xFFFFFF00
        mov [edi + LOCAL_APIC_ESR_REGISTER], dword 0
        mov [edi + LOCAL_APIC_DFR_REGISTER], dword 0xF0000000
        and [edi + LOCAL_APIC_LDR_REGISTER], dword 0x00FFFFFF

    .startProcessor:
        .fillInMemoryLocationsInStartCode:
            call .findMemoryLocation
            .findMemoryLocation:
            mov ebx, [esp]
            add ebx, processorStartCode - .findMemoryLocation
            mov [START_PROCESSOR_TRAMPOLINE_LOCATION], byte 0xEA
            mov [START_PROCESSOR_TRAMPOLINE_LOCATION + 1], ebx
            pop ebx
            add ebx, processorStartCode_FarJumpLocation - .findMemoryLocation
            mov [ebx + processorStartCode_JumpAddress - processorStartCode_FarJumpLocation], bx

            call Thunk_ECX
            mov ebx, [ecx + LoadLocationTable_BootGDTDescriptor]
            mov [START_PROCESSOR_GDTR_TRAMPOLINE], ebx

            mov ebx, [edi + LOCAL_APIC_ID_REGISTER_OFFSET]
            mov [BIOS_CALL_PROCESSOR_LOCAL_APIC_ID], ebx

        .runProcessorInitializationFunctions:
            xor ecx, ecx
            mov [edi + LOCAL_APIC_INTERRUPT_COMMAND_REGISTER_UPPER_OFFSET], ecx
            mov ecx, INTERRUPT_COMMAND_INIT
            mov [edi + LOCAL_APIC_INTERRUPT_COMMAND_REGISTER_LOWER_OFFSET], ecx

            mov ecx, 1000000
            .waitLoop1:
                loop .waitLoop1

            xor ecx, ecx
            mov [edi + LOCAL_APIC_INTERRUPT_COMMAND_REGISTER_UPPER_OFFSET], ecx
            mov ecx, INTERRUPT_COMMAND_INIT_DEASSERT
            mov [edi + LOCAL_APIC_INTERRUPT_COMMAND_REGISTER_LOWER_OFFSET], ecx

            mov ecx, 1000000
            .waitLoop2:
                loop .waitLoop2

            xor ecx, ecx
            mov [edi + LOCAL_APIC_INTERRUPT_COMMAND_REGISTER_UPPER_OFFSET], ecx
            mov ecx, INTERRUPT_COMMAND_STARTUP + (START_PROCESSOR_TRAMPOLINE_LOCATION >> 12)
            mov [edi + LOCAL_APIC_INTERRUPT_COMMAND_REGISTER_LOWER_OFFSET], ecx

    popa
    ret

processorStartCode:
    [bits 16]
    cli
    mov eax, [START_PROCESSOR_GDTR_TRAMPOLINE]
    lgdt [eax]

    mov eax, cr0
    or al, 1
    mov cr0, eax

        db 0xEA
    processorStartCode_JumpAddress:
        dw 0
        dw 8
    processorStartCode_FarJumpLocation:
        [bits 32]
        mov ax, 0x10 	; segment registers are redirected to what's defined in the GDT
        mov ds, ax		; set all of them through ax
        mov ss, ax
        mov es, ax
        mov fs, ax
        mov gs, ax

        jmp processorStartCode_beginProtectedMode
    processorStartCode_beginProtectedMode:
        mov eax, [POINTER_TO_LOCAL_APIC_ADDRESS]
        mov ebx, [eax + LOCAL_APIC_ID_REGISTER_OFFSET]
        shr ebx, 24                                         ; ebx is Local APIC ID
        xor eax, eax

    .findProcessorNumber:
        cmp [MULTIPROCESSOR_LOCAL_APIC_ID_TABLE + 4 * eax], ebx
        je .findProcessorNumber_done
        inc eax
        cmp eax, [POINTER_TO_NUMBER_OF_PROCESSORS]
        jb .findProcessorNumber
        jmp processorStartError

    .findProcessorNumber_done:
        mov ecx, eax
        shl eax, 10                                      ; each gets 1024 bytes of stack space
        mov ebp, PROCESSOR_CONTEXTS_MAX_ADDRESS - PROCESSOR_CONTEXT_SIZE
        sub ebp, eax
        mov eax, ebp
        mov esp, ebp
        add esp, PROCESSOR_CONTEXT_STACK_BASE

        call setupProcessorContext

        lidt [IDT_DESCRIPTOR_ADDRESS]

        mov eax, KERNEL_PDE_ADDRESS
        mov cr3, eax
        mov eax, cr0
        or eax, 0x80000001
        mov cr0, eax

        .setupLocalApic:
            mov ebx, [LOCAL_APIC_PAGED_LOCATION + LOCAL_APIC_SPURIOUS_INTERRUPT_REGISTER_OFFSET]
            or bh, 0x11
            mov bl, 0xFF
            mov [LOCAL_APIC_PAGED_LOCATION + LOCAL_APIC_SPURIOUS_INTERRUPT_REGISTER_OFFSET], ebx
            
            mov eax, 0x10000
            mov [LOCAL_APIC_PAGED_LOCATION + LOCAL_APIC_LVT_LINT0], eax
            mov [LOCAL_APIC_PAGED_LOCATION + LOCAL_APIC_LVT_LINT1], eax

            and [LOCAL_APIC_PAGED_LOCATION + LOCAL_APIC_TPR_REGISTER], dword 0xFF000000
            mov [LOCAL_APIC_PAGED_LOCATION + LOCAL_APIC_ESR_REGISTER], dword 0
            mov [LOCAL_APIC_PAGED_LOCATION + LOCAL_APIC_DFR_REGISTER], dword 0xF0000000
            and [LOCAL_APIC_PAGED_LOCATION + LOCAL_APIC_LDR_REGISTER], dword 0x00FFFFFF

        .waitForInitialization:
            cmp [MULTIPROCESSOR_INITIALIZATION_DONE], dword 1
            jne .waitForInitialization
        
        call FPU_Initialize
        
        call GetKernelAddress_ECX
        add ecx, [ecx + EXTERNAL_CALL_EnterSchedulerNoReschedule]
        sti
        jmp ecx

        mov eax, 0x12345679
        .hltLoop:
            hlt
            jmp .hltLoop

processorStartError:
    mov eax, 0xFFFFFFFF
    .htlLoop:
        hlt
        jmp .htlLoop