
InterruptIO_TryACPISetup:
    pusha

    mov ecx, 0x20000 / 16
    mov edx, 0x80000
    .loop3:
        cmp [edx], dword 'RSD '
        jne .loop3_notFound
        cmp [edx + 4], dword 'PTR '
        je .tryACPITables_found
        .loop3_notFound:
        add edx, 16
        loop .loop3
    mov ecx, 0x20000 / 16
    mov edx, 0xE0000
    .loop4:
        cmp [edx], dword 'RSD '
        jne .loop4_notFound
        cmp [edx + 4], dword 'PTR '
        je .tryACPITables_found
        .loop4_notFound:
        add edx, 16
        loop .loop4
    
    .noACPIFound:

    popa
    xor eax, eax
    ret
    
    .tryACPITables_found:
    
    mov edx, [edx + 16]
    mov ecx, [edx + 4]
    sub ecx, 36
    shr ecx, 2              ; ecx = #entries in RSDT
    add edx, 36
    .loop5:
        mov eax, [edx]
        cmp [eax], dword 'APIC'
        je .MADTFound
        add edx, 4
        loop .loop5
    
    jmp .noACPIFound
    
    .MADTFound:
        mov [MULTIPROCESSOR_TYPE], dword MULTIPROCESSOR_TYPE_SMP

        mov edx, eax
        mov eax, [edx + 0x24]
        mov [POINTER_TO_LOCAL_APIC_ADDRESS], eax
        or ax, PAGING_FLAG_PRESENT | PAGING_FLAG_WRITE
        mov [MEMORY_KERNEL_CORE_PTE_ADDRESS + (LOCAL_APIC_PAGED_LOCATION >> 10)], eax

        xor ebx, ebx
        mov esi, 0x2C
        mov edi, [POINTER_TO_LOCAL_APIC_ADDRESS]
        mov edi, [edi + LOCAL_APIC_ID_REGISTER_OFFSET]
        shr edi, 24
        .IterateMADTEntries:
            cmp esi, [edx + 4]
            jae .IterateMADTEntries_ExitLoop
            mov eax, [edx + esi]
            xor ecx, ecx
            mov cl, ah
            ;add esi, ecx
            cmp al, 0
            je .IterateMADTEntries_LocalAPIC
            cmp al, 1
            je .IterateMADTEntries_IOAPIC
            cmp al, 2
            je .IterateMADTEntries_IntOveride

            jmp .IterateMADTEntries_end

            .IterateMADTEntries_IntOveride:
                mov eax, message3
                call Debugging_PrintString

                pusha
                mov ebx, [edx + esi + 4]
                mov ecx, [edx + esi]
                shr ecx, 24
                add ecx, 0x20
                mov ch, [edx + esi + 8]
                and ch, 00001010b                       ; possibly set active low & level-triggered flags
                call InterruptIO_IOAPIC_SetGSRToISR;    TEMPORARY INTERRUPT FIX
                popa

                jmp .IterateMADTEntries_end

            .IterateMADTEntries_IOAPIC:
                mov eax, message1
                call Debugging_PrintString

                pusha
                mov ebx, [edx + esi + 4]
                mov ecx, [edx + esi + 8]
                call InterruptIO_AddIOAPIC; TEMPORARY INTERRUPT FIX
                popa

                jmp .IterateMADTEntries_end

            .IterateMADTEntries_LocalAPIC:
                push eax
                mov eax, message2
                call Debugging_PrintString
                pop eax

                shr eax, 24
                cmp eax, edi
                je .IterateMADTEntries_bootProcessor
                mov [MULTIPROCESSOR_LOCAL_APIC_ID_TABLE + 4 + 4 * ebx], eax
                inc ebx
                jmp .IterateMADTEntries_end
                .IterateMADTEntries_bootProcessor:
                mov [MULTIPROCESSOR_LOCAL_APIC_ID_TABLE], eax

                jmp .IterateMADTEntries_end
            
            .IterateMADTEntries_end:
            add esi, ecx
            jmp .IterateMADTEntries
        
        .IterateMADTEntries_ExitLoop:
        inc ebx
        mov [POINTER_TO_NUMBER_OF_PROCESSORS], ebx

        popa
        xor eax, eax
        inc eax
        ret

InterruptIO_AddIOAPIC:                          ; ebx=address, ecx=global interrupt base
    pusha

    mov [ebx], dword 0
    mov eax, [ebx + 0x10]               ; eax = APIC ID
    mov [ebx], dword 1
    mov edx, [ebx + 0x10]
    shr edx, 16
    mov dh, 0
    inc edx                             ; edx = #entries

    mov esi, MULTIPROCESSOR_IO_APIC_TABLE
    .loop:
        mov edi, esi
        add esi, 16
        cmp [edi], dword 0
        jne .loop
    
    .addIOAPICEntry:
    mov [edi], ebx          ; address
    mov [edi + 4], ecx      ; interrupt base
    mov [edi + 8], edx      ; # entries
    mov [edi + 12], al      ; ID

    mov eax, 0
    mov ebx, ecx
    .addISREntries:
        mov ecx, ebx        ; ecx = GSR
        add ecx, 0x20       ; ebx = ISR
        call InterruptIO_IOAPIC_SetGSRToISR
        inc ebx
        inc eax
        cmp eax, edx
        jb .addISREntries

    mov al, 0x70
    out 0x22, al
    in al, 0x23
    or al, 1
    out 0x23, al

    popa
    ret

InterruptIO_IOAPIC_SetGSRToISR:                  ; ebx = GSR, ECX = ISR (+ 0x8000 for level, + 0x2000 for active low pin polarity)
    pusha

    mov edx, MULTIPROCESSOR_IO_APIC_TABLE
    .findIOAPIC:
        cmp [edx], dword 0
        je .fail

        mov esi, [edx + 4]      ; base
        mov edi, esi
        add edi, [edx + 8]      ; limit

        cmp ebx, esi
        jb .findIOAPIC_wrongAPIC
        cmp ebx, edi
        jb .foundIOAPIC

        .findIOAPIC_wrongAPIC:
        add edx, 16
        jmp .findIOAPIC

    .foundIOAPIC:
        sub ebx, esi            ; offset for this apic

        xor esi, esi

        cmp cl, 0x20
        jne .notPITInterrupt
            mov esi, 0x00010000

        .notPITInterrupt:
    
        mov eax, [edx]          ; address of APIC_

        add ebx, ebx
        add ebx, 0x11

        ; high byte first
        mov [eax], ebx
        mov edx, [eax + 0x10] 
        xor edx, edx
        mov [eax], ebx
        mov [eax + 0x10], edx

        ;low byte
        dec ebx
        ;or ch, 00000001b
        and ecx, 0x0000FFFF
        or ecx, esi
        mov [eax], ebx
        mov [eax + 0x10], ecx

    .fail:

    popa
    ret


message1: db "IO APIC/", 0
message2: db "Local APIC/", 0
message3: db "Int Overide/", 0

InterruptIO_TryMPSetup:
    pusha
    mov eax, MP_FLOATING_POINT_STRUCTURE_SEARCH_STRING

    mov ecx, 0x20000 / 16
    mov edx, 0x80000
    .loop1:
        cmp [edx], eax
        je .foundStructure
        add edx, 16
        loop .loop1
    
    mov ecx, 0x40000 / 16
    mov edx, 0xC0000
    .loop2:
        cmp [edx], eax
        je .foundStructure
        add edx, 16
        loop .loop2
    
    .noStructure:               ; System doesn't support multiprcocessor, return to perform legacy setup
        popa
        xor eax, eax
        ret

    .foundStructure:
        cmp byte [edx + MP_FPS_MP_FEATURES_1_OFFSET], 0
        jne .noStructure                ; TODO -- fix for default structures

    .getSMPAndIOInformation:
        mov [MULTIPROCESSOR_TYPE], dword MULTIPROCESSOR_TYPE_SMP

        .saveMultiprocessorConfigurationInformation:
            mov ecx, [edx + MP_FPS_MP_CONFIG_POINTER_OFFSET]
            mov [POINTER_TO_MP_CONFIG_TABLE_ADDRESS], ecx

            xor esi, esi
            mov si, [ecx + MP_CONFIG_ENTRY_COUNT_OFFSET]                   ; entry count

            mov eax, [ecx + MP_CONFIG_LOCAL_APIC_ADDRESS_OFFSET]
            mov [POINTER_TO_LOCAL_APIC_ADDRESS], eax
            or ax, PAGING_FLAG_PRESENT | PAGING_FLAG_WRITE
            mov [MEMORY_KERNEL_CORE_PTE_ADDRESS + (LOCAL_APIC_PAGED_LOCATION >> 10)], eax
            ;invlpg [LOCAL_APIC_PAGED_LOCATION]

            add ecx, MP_CONFIG_TABLE_LENGTH
            mov [POINTER_TO_PROCESSOR_INFO_BASE_ADDRESS], ecx

        .getBootProcessorAPICID:
            xor ebx, ebx
            mov edi, [POINTER_TO_LOCAL_APIC_ADDRESS]
            mov edi, [edi + LOCAL_APIC_ID_REGISTER_OFFSET]
            shr edi, 24

        .getMPTableEntries:
            cmp esi, 0
            je .getMPTableEntries_done
            dec esi

            cmp [ecx], byte 0
            je .getMPTableEntries_processor

            cmp [ecx], byte 2
            je .getMPTableEntries_IOAPIC

            cmp [ecx], byte 3
            je .getMPTableEntries_IOIntAssignment

            add ecx, 8
            jmp .getMPTableEntries


            .getMPTableEntries_processor:
                xor eax, eax
                mov al, [ecx + 3]           ; flags
                mov ah, [ecx + 1]           ; apic id
                add ecx, 20

                and al, 1
                cmp al, 0
                je .getMPTableEntries

                shr eax, 8
                cmp eax, edi
                je .getMPTableEntries_bootProcessor
                mov [MULTIPROCESSOR_LOCAL_APIC_ID_TABLE + 4 + 4 * ebx], eax
                inc ebx
                jmp .getMPTableEntries
                .getMPTableEntries_bootProcessor:
                mov [MULTIPROCESSOR_LOCAL_APIC_ID_TABLE], eax
                jmp .getMPTableEntries
            
            .getMPTableEntries_IOAPIC:
                xor eax, eax
                mov al, [ecx + 3]
                mov esi, [ecx + 4]
                add ecx, 8

                and al, 1
                cmp al, 0
                je .getMPTableEntries
                pusha
                mov ebx, esi
                mov ecx, 0
                call InterruptIO_AddIOAPIC
                popa
                jmp .getMPTableEntries
            
            .getMPTableEntries_IOIntAssignment:
                pusha
                    xor eax, eax
                    mov al, [ecx + 5]
                    xor ebx, ebx
                    mov bl, [ecx + 7]
                    mov dl, [ecx + 2]
                    and dl, 00001010b
                    mov ah, dl                  ; set active low & level triggered flags is set

                    .notLevelTriggered:
                    mov ecx, eax
                    call InterruptIO_IOAPIC_SetGSRToISR
                popa
                add ecx, 8
                jmp .getMPTableEntries


        .getMPTableEntries_done:
        inc ebx
        mov [POINTER_TO_NUMBER_OF_PROCESSORS], ebx

        popa
        xor eax, eax
        inc eax
        ret                 ; success: continue to SMP setup



InterruptIO_LegacySetup:
    pusha

    .initializePIC:
        mov dx, 0x20		; Starts initialization process for both PICS
        mov al, 0x11			;This process reprograms them to use interrupts
        out dx, al			;0x20 through 0x2A for their IRQ lines
        call .IOWait
        mov dx, 0xA0
        mov al, 0x11
        out dx, al
        call .IOWait

        mov dx, 0x21		; Thorough the PIC data gate
        mov al, 0x20		; issue ICW2
        out dx, al		; Setting the vector offsets for each PIC
        call .IOWait
        mov dx, 0xA1		; 0x20 for the master
        mov al, 0x28		; 0x28 for the slave
        out dx, al
        call .IOWait

        mov dx, 0x21		; Issue ICW4 through the PIC data gate
        mov al, 0x04		; Tell the master there is a slave PIC at IRQ2
        out dx, al
        call .IOWait
        mov dx, 0xA1		; Tell the slave its cascade identity
        mov al, 0x02
        out dx, al
        call .IOWait

        mov dx, 0x21		; Issue ICW4 through PIC data gate
        mov al, 0x01		; Tell each to use
        out dx, al		; 8086/88 (MCS-80/85) mode
        call .IOWait
        mov dx, 0xA1
        mov al, 0x01
        out dx, al
        call .IOWait

        mov dx, 0x21		; reset the masks to how they were before
        mov al, 0x00
        out dx, al
        mov dx, 0xA1
        mov al, 0x00
        out dx, al

        mov dx, 0x21		; unmask all -- interrupts are routed to Generic Handlers & Will Be Ignored Until Entries Are Added
        mov al, 0x00
        out dx, al

        popa
        ret

    .IOWait:
        pusha			; outputs with another unused gate to force previous io instruction to complete
        mov al, 0x0
        mov dx, 0x80
        out dx, al
        popa
        ret

TEST_SetupKeyboard:
	pusha

        .disableTranslation:			; make sure that the ps2 controller doesn't try to convert back to set 1
            call .waitForWriteReady

            mov dx, 0x64			; ask to read a the control byte from the ps2 controller
            mov al, 0x20
            out dx, al

            call .waitForCommandResponse

            mov dx, 0x60			; when its sent, read it from 0x60
            in al, dx
            and al, 10111111b		; set the 6th bit 
            mov bl, al

            call .waitForWriteReady

            mov dx, 0x64			; ask to write the byte out
            mov al, 0x60
            out dx, al

            call .waitForWriteReady

            mov al, bl			; when its sent, write from 0x60
            mov dx, 0x60
            out dx, al

            call .waitForWriteReady

        popa
        ret

    .waitForCommandResponse:			; checks the ps/2 status register to see if the data byte in is ready

        mov dx, 0x64			; poll bit 1
        in al, dx
        and al, 0x01
        cmp al, 0x01
        jne .waitForCommandResponse

        ret

    .waitForWriteReady:			; check is the previous command has been processed and can recieve another

        mov dx, 0x64
        in al, dx
        and al, 00000010b		; poll bit 2
        cmp al, 0x0
        jne .waitForWriteReady

        ret