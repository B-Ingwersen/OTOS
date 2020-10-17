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

IDT_GenericHandlerRedirect:	
	and eax, 0x000000FF			; isolate ISR #
	mov ebx, eax
	shl ebx, 4					; multiply by 16 for offset
	cmp [PROCESS_INTERRUPT_HANDLERS_TABLE + ebx], dword 0
	je .noProcessHandler

    mov edx, [PROCESS_INTERRUPT_HANDLERS_TABLE + ebx + 8]
    cmp edx, dword 0
    je .noImmediateHandler

        pusha

        mov eax, cr3
        push eax

        xor eax, eax
        mov al, [PROCESS_INTERRUPT_HANDLERS_TABLE + ebx]
        mov eax, [PROCESS_TABLE_ADDRESS + 4 * eax]
        and ax, PAGING_LOW_BIT_REVERSE_MASK
        mov cr3, eax

        push ebp
        mov ebp, esp
        call edx
        pop ebp

        pop eax
        mov cr3, eax

        popa

    .noImmediateHandler:

    call scheduleInterrupt

    .noProcessHandler:
    
	cmp [MULTIPROCESSOR_TYPE], dword MULTIPROCESSOR_TYPE_SMP
	je .sendAPIC_EOI   ;TEMPORARY INTERRUPT FIX

	.sendPIC_EOI:
		mov dx, 0x20			;send EOI signal to to let the cpu know the interrupt is acknowledged
		mov al, 0x20
		out dx, al

        mov [ebp + PROCESSOR_CONTEXT_THREAD_TIME], dword 0

		jmp .interruptReturn   ;TEMPORARY INTERRUPT FIX

	.sendAPIC_EOI:
		xor ebx, ebx
		xor ecx, ecx
		mov bl, al
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

		mov eax, 0
    	mov [LOCAL_APIC_PAGED_LOCATION + LOCAL_APIC_EOI_REGISTER], eax
	
		.APICNoEOINeeded:

	.interruptReturn:

	popa
	jmp genericInterruptReturn

IDT_ProcessScheduledInterrupt:							; eax = interrupt number
	shl eax, 4
	mov edx, [PROCESS_INTERRUPT_HANDLERS_TABLE + eax]		; PID
	mov ebx, [PROCESS_INTERRUPT_HANDLERS_TABLE + eax + 4]	; handler address
	
    mov ebp, esp
	and bp, PROCESSOR_CONTEXT_BASE_BIT_MASK

    mov eax, KERNEL_PDE_ADDRESS
    mov cr3, eax

	call IDT_InterruptMessage
	ret

IDT_InterruptMessage:									; edx = PID, ebx = address, ebp must be set to processor context
    pusha

	push ebx											; save call address

	call getMemoryTablePage
	mov edi, eax			; thread context
	call getMemoryTablePage
	mov esi, eax			; stack

    .tryEnterMessageProcess:
        mov eax, edx
        call enterJustProcess
        cmp eax, PROCESS_ENTER_SUCCESS
        jne .fail2

        xor eax, eax
        mov al, dl
        lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * eax]
        call openSpinlock

        mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_MESSAGING_SPINLOCK
        call openSpinlock

    .checkForMessageHandler:
        cmp [PROCESS_INFO_ADDRESS + PROCESS_INFO_MESSAGING_TABLE_BASE_ADDRESS], dword 0
        je .fail3

    .getOpenSlotInTable:
        push ebx

        mov ecx, [PROCESS_INFO_ADDRESS + PROCESS_INFO_MESSAGING_TABLE_BASE_ADDRESS]
        shr ecx, 10
        and cl, 0xFC
        add ecx, RECURSIVE_PTE_MAP_BASE
        mov ebx, 0
        .lookForOpenSlot:
            mov eax, [ecx]
            and al, PAGING_FLAG_PRESENT
            cmp al, 0
            je .slotFound

            add ecx, 8
            inc ebx
            cmp ebx, MESSAGING_N_SLOTS
            jb .lookForOpenSlot
            jmp .fail4

        .slotFound:
        pop ebx
    
    .insertPages:
        or di, OS_PAGING_FLAG_THREAD_CONTEXT | PAGING_FLAG_PRESENT | PAGING_FLAG_WRITE
        or si, OS_PAGING_FLAG_GENERIC_PROTECTED | PAGING_FLAG_PRESENT | PAGING_FLAG_WRITE | PAGING_FLAG_USER
        ;mov [ecx], edi
        ;mov [ecx + 4], esi
        mov [ecx], esi                              ; data page BELOW
        mov [ecx + 4], edi                          ; thread context ABOVE
        shl ecx, 10
        invlpg [ecx]
        invlpg [ecx + PAGE_SIZE]
    
    .recordChanges:
        mov [ecx + THREAD_CURRENT_PROCESS], edx
        mov eax, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]

        add ecx, PAGE_SIZE
        mov [ebp + PROCESSOR_CONTEXT_CURRENT_THREAD_PAGED_ADDRESS], ecx

		mov [ecx + THREAD_MESSAGE_STACK_BASE], dword 0			; current process -- guarentees return fail
		mov [ecx + THREAD_MESSAGE_STACK_BASE + 12], esi			; data page -- must be freed on failure
		mov [ecx + THREAD_MESSAGE_STACK_POINTER], dword MESSAGE_STACK_ENTRY_SIZE

        sub ecx, PAGE_SIZE
        mov eax, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
        mov [ecx + 2048 - 8], eax                                           ; first is the PID
        mov [ecx + 2048 - 4], ecx                                           ; second is the base address
        mov [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS], edx

    .success:
		pop ebx					; restore handler call address

		add ecx, PAGE_SIZE
        mov [ecx + THREAD_INTERRUPT_STACK_SAVE + 7 * 4], ebx				; handler
        mov [ecx + THREAD_INTERRUPT_STACK_SAVE + 8 * 4], dword 0x1B			; code seg
        mov [ecx + THREAD_INTERRUPT_STACK_SAVE + 9 * 4], dword 0x202		; flags
		lea eax, [ecx - PAGE_SIZE + 2048 - 12]
        mov [ecx + THREAD_INTERRUPT_STACK_SAVE + 10 * 4], eax				; 
        mov [ecx + THREAD_INTERRUPT_STACK_SAVE + 11 * 4], dword 0x23		; data seg

        mov [ecx + THREAD_EXECUTION_STATE], dword THREAD_EXECUTION_STATE_MASK_EXECUTING

        fsave [ecx + THREAD_INTERRUPT_FP_SAVE]

        mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_MESSAGING_SPINLOCK
        call closeSpinlock

        xor eax, eax
        mov al, dl
        lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * eax]
        call closeSpinlock

        jmp enterScheduler_setTimerSlice

    .fail4:
        pop ebx
    .fail3:
        mov eax, PROCESS_INFO_ADDRESS + PROCESS_INFO_MESSAGING_SPINLOCK
        call closeSpinlock

        xor eax, eax
        mov al, dl
        lea eax, [PROCESS_SPINLOCK_TABLE_ADDRESS + 4 * eax]
        call closeSpinlock

        push ebx
        mov eax, edx
        mov ebx, KERNEL_PDE_ADDRESS
        call exitJustProcess
        pop ebx	

	.fail2:
        mov eax, esi
        call freeMemoryTablePage
        mov eax, edi
        call freeMemoryTablePage

        pop ebx

		popa

		ret
