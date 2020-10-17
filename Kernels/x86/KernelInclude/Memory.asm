getMemoryTablePage:                             ; return address in eax
    push edx
    push ebx

    mov eax, MEMORY_TABLE_SPINLOCK
    call openSpinlock

    mov ebx, [MEMORY_TABLE_POINTER_TO_DEQUEUE]
    mov edx, [ebx]                              ; get the address of the free page
    add ebx, 4

    cmp ebx, [MEMORY_TABLE_POINTER_TO_END_ADDRESS]
    jb getMemoryTablePage_noDequeueWrap
    mov ebx, [MEMORY_TABLE_POINTER_TO_START_ADDRESS]
    getMemoryTablePage_noDequeueWrap:

    cmp ebx, [MEMORY_TABLE_POINTER_TO_ENQUEUE]
    je memoryTableFail

    mov [MEMORY_TABLE_POINTER_TO_DEQUEUE], ebx

    call closeSpinlock
    mov eax, edx

    pop ebx
    pop edx

    ret

freeMemoryTablePage:                            ; eax is pointer to page to be freed
    push edx
    push ebx

    mov edx, eax
    and dx, PAGING_LOW_BIT_REVERSE_MASK
    mov eax, MEMORY_TABLE_SPINLOCK
    call openSpinlock

    mov ebx, [MEMORY_TABLE_POINTER_TO_ENQUEUE]
    mov [ebx], edx
    add ebx, 4

    cmp ebx, [MEMORY_TABLE_POINTER_TO_END_ADDRESS]
    jb freeMemoryTablePage_noEnqueueWrap
    mov ebx, [MEMORY_TABLE_POINTER_TO_START_ADDRESS]
    freeMemoryTablePage_noEnqueueWrap:

    mov [MEMORY_TABLE_POINTER_TO_ENQUEUE], ebx

    call closeSpinlock
    mov eax, edx

    pop ebx
    pop edx

    ret

memoryTableFail:
    mov eax, 'MEM_'
    mov ebx, 'FAIL'
    xor ecx, ecx
    xor edx, edx
    mov [0xB8000], byte 'M'
    jmp $

checkMemoryRegionNotMapped:                     ; ebx=base address, ecx = # pages, edx = PID
    pusha

    ; ecx = upper limit & check bounds
    cmp ecx, 0x100000
    jae checkMemoryRegionNotMapped_fail1
    shl ecx, PAGE_SHIFT_RIGHT_BITS
    add ecx, ebx
    jc checkMemoryRegionNotMapped_fail1
    and cx, PAGING_LOW_BIT_REVERSE_MASK     ; ecx now is the high limit of the memory region
    cmp ecx, USER_MEMORY_UPPER_LIMIT
    ja checkMemoryRegionNotMapped_fail1
    cmp ebx, USER_MEMORY_LOWER_LIMIT
    jb checkMemoryRegionNotMapped_fail1

    ; set up variables on VM memory map
    xor eax, eax
    mov al, dl
    shl eax, PROCESS_VIRTUAL_MEMORY_MAP_ENRTY_SIZE_SHL_BITS
    add eax, PROCESS_VIRTUAL_MEMORY_MAP_ADDRESS
    
    mov edx, ebx
    shr edx, 10
    and dl, 0xFC
    push eax
    add eax, edx                            ; eax is now the address in the page directory
    shr edx, 10
    and dl, 0xFC
    add edx, [esp]
    add esp, 4
    add edx, OS_PROCESS_PDE_OFFSET          ; edx is now the address in the PDE mapping

    mov edi, ebx
    add edi, 0x400000
    and edi, 0xFFC00000
    checkMemoryRegionNotMapped_checkPTELoop:
        cmp [edx], dword 0
        je checkMemoryRegionNotMapped_continuePTELoop

        cmp edi, ecx
        jb checkMemoryRegionNotMapped_notEndOfPageCheckRange
        mov edi, ecx
        checkMemoryRegionNotMapped_notEndOfPageCheckRange:

        checkMemoryRegionNotMapped_checkPageLoop:
            cmp [eax], dword 0
            jne checkMemoryRegionNotMapped_fail1

            add ebx, 0x1000
            add eax, 4
            cmp ebx, edi
            jb checkMemoryRegionNotMapped_checkPageLoop

        checkMemoryRegionNotMapped_continuePTELoop:
        
        add edx, 4
        mov ebx, edi
        add edi, 0x400000

        cmp ebx, ecx
        jb checkMemoryRegionNotMapped_checkPTELoop

    popa
    mov eax, MEMORY_CHECK_SUCESS_CODE
    ret

    checkMemoryRegionNotMapped_fail1:

    popa
    mov eax, MEMORY_CHECK_FAILURE_CODE
    ret

checkMemoryRegionMapped:                     ; ebx=base address, ecx = # pages, edx = PID
                                            ; ebx includes acceptable OS flags
    pusha

    mov bp, bx                              ; ax becomes the unpermitted flags
    xor bp, OS_PAGING_FLAGS_MASK
    and bp, OS_PAGING_FLAGS_MASK

    ; ecx = upper limit & check bounds
    cmp ecx, 0x100000
    jae checkMemoryRegionMapped_fail1
    shl ecx, PAGE_SHIFT_RIGHT_BITS
    add ecx, ebx
    jc checkMemoryRegionMapped_fail1
    and cx, PAGING_LOW_BIT_REVERSE_MASK     ; ecx now is the high limit of the memory region
    cmp ecx, USER_MEMORY_UPPER_LIMIT
    ja checkMemoryRegionMapped_fail1
    cmp ebx, USER_MEMORY_LOWER_LIMIT
    jb checkMemoryRegionMapped_fail1

    ; set up variables on VM memory map
    xor eax, eax                                                ; zero eax
    mov al, dl                                                  ; move process number
    shl eax, PROCESS_VIRTUAL_MEMORY_MAP_ENRTY_SIZE_SHL_BITS
    add eax, PROCESS_VIRTUAL_MEMORY_MAP_ADDRESS                 ; get memory map location
    
    mov edx, ebx
    shr edx, 10
    and dl, 0xFC
    push eax
    add eax, edx                            ; eax is now the address in the page directory
    shr edx, 10
    and dl, 0xFC
    add edx, [esp]
    add esp, 4
    add edx, OS_PROCESS_PDE_OFFSET          ; edx is now the address in the PDE mapping

    mov edi, ebx
    add edi, 0x400000
    and edi, 0xFFC00000
    checkMemoryRegionMapped_checkPTELoop:
        cmp [edx], dword 0
        je checkMemoryRegionMapped_fail1

        cmp edi, ecx ; check if next 4MB region is in the range
        jb checkMemoryRegionMapped_notEndOfPageCheckRange
        mov edi, ecx
        checkMemoryRegionMapped_notEndOfPageCheckRange:

        checkMemoryRegionMapped_checkPageLoop:
            mov esi, [eax]
            cmp esi, dword 0
            je checkMemoryRegionMapped_fail1

            and si, bp                      ; if any page has an unacceptable flag, the check fails
            cmp si, 0
            jne checkMemoryRegionMapped_fail1

            add ebx, 0x1000
            add eax, 4
            cmp ebx, edi
            jb checkMemoryRegionMapped_checkPageLoop
            
        checkMemoryRegionMapped_continuePTELoop:
        
        add edx, 4
        mov ebx, edi
        add edi, 0x400000

        cmp ebx, ecx
        jb checkMemoryRegionMapped_checkPTELoop

    popa
    mov eax, MEMORY_CHECK_SUCESS_CODE
    ret

    checkMemoryRegionMapped_fail1:

    popa
    mov eax, MEMORY_CHECK_FAILURE_CODE
    ret



mapPTEsForMemoryRegion:
    pusha

    shl ecx, PAGE_SHIFT_RIGHT_BITS
    add ecx, ebx

    shr ebx, 20
    and bl, 0xFC
    sub ecx, PAGE_SIZE
    shr ecx, 20

    and edx, 0xFF
    shl edx, PROCESS_VIRTUAL_MEMORY_MAP_ENRTY_SIZE_SHL_BITS
    add edx, PROCESS_VIRTUAL_MEMORY_MAP_ADDRESS

    mov edi, ebx
    shl edi, 10
    add edi, edx
    add edx, OS_PROCESS_PDE_OFFSET

    mapPTEsForMemoryRegion_loop:
        cmp [edx + ebx], dword 0
        jne mapPTEsForMemoryRegion_loop_alreadyMapped

        call getMemoryTablePage
        call zeroPagePagingEnabled
        or ax, OS_PAGING_FLAG_GENERIC_PROTECTED | PAGING_FLAG_PRESENT | PAGING_FLAG_WRITE | PAGING_FLAG_USER
        mov [edx + ebx], eax
        and ax, PAGING_LOW_BIT_REVERSE_MASK
        invlpg [edi]
        ;mov eax, edi
        ;call zeroPage

        mapPTEsForMemoryRegion_loop_alreadyMapped:

        add ebx, 4
        add edi, PAGE_SIZE
        cmp ebx, ecx
        jbe mapPTEsForMemoryRegion_loop
    
    popa
    ret

mapNewMemoryForRegion:                                   ; ebx=base address (bx include desired flags), ecx = # pages, edx = PID
    
    ; check if ecx is zero -- do nothing so
    test ecx, ecx
    je mapNewMemoryForRegion_done    
    
    pusha

    push ebx

    and edx, 0xFF
    shl edx, PROCESS_VIRTUAL_MEMORY_MAP_ENRTY_SIZE_SHL_BITS
    add edx, PROCESS_VIRTUAL_MEMORY_MAP_ADDRESS
    shr ebx, 10
    and bl, 0xFC
    add edx, ebx

    pop ebx
    and bx, PAGING_LOW_BIT_MASK
    ;xor bh, bh
    ;shl bl, PAGING_FLAG_WRITE_SHIFT_BITS
    or bx, PAGING_FLAG_PRESENT ;| PAGING_FLAG_USER

    mapNewMemoryForRegion_loop:
        call getMemoryTablePage
        or ax, bx
        mov [edx], eax

        add edx, 4
        loop mapNewMemoryForRegion_loop
    
    popa

    mapNewMemoryForRegion_done:

    ret

unmapMemoryForRegion:                                   ; ebx=base address (bx include desired flags), ecx = # pages, edx = PID
    ; check that the number of pages is not zero
    test ecx, ecx
    je unmapMemoryForRegion_doNothing

    pusha

    and edx, 0xFF
    shl edx, PROCESS_VIRTUAL_MEMORY_MAP_ENRTY_SIZE_SHL_BITS
    add edx, PROCESS_VIRTUAL_MEMORY_MAP_ADDRESS
    shr ebx, 10
    and bl, 0xFC
    add edx, ebx

    mov edi, edx
    mov esi, ecx

    unmapMemoryForRegion_loop:
        mov eax, [edx]

        and ax, OS_PAGING_FLAGS_MASK
        cmp ax, OS_PAGING_FLAG_KERNEL
        je unmapMemoryForRegion_loop_noPageFree
        cmp ax, OS_PAGING_FLAG_SHARED
        je unmapMemoryForRegion_loop_noPageFree

        mov eax, [edx]
        and ax, PAGING_LOW_BIT_REVERSE_MASK
        call freeMemoryTablePage

        unmapMemoryForRegion_loop_noPageFree:
        mov [edx], dword 0

        add edx, 4
        loop unmapMemoryForRegion_loop

    popa

    unmapMemoryForRegion_doNothing:
    ret

mapKernelMemoryForRegion:                                   ; ebx=base address (bx include desired flags), ecx = # pages, edx = PID, edi = Kernel Memory Base
    pusha

    push ebx

    and edx, 0xFF
    shl edx, PROCESS_VIRTUAL_MEMORY_MAP_ENRTY_SIZE_SHL_BITS
    add edx, PROCESS_VIRTUAL_MEMORY_MAP_ADDRESS
    shr ebx, 10
    and bl, 0xFC
    add edx, ebx

    pop ebx
    xor bh, bh
    ;shl bl, PAGING_FLAG_WRITE_SHIFT_BITS
    or di, bx
    or di, PAGING_FLAG_PRESENT | OS_PAGING_FLAG_KERNEL ;| PAGING_FLAG_USER

    mapKernelMemoryForRegion_loop:
        mov [edx], edi

        add edx, 4
        add edi, PAGE_SIZE
        loop mapKernelMemoryForRegion_loop
    
    popa
    ret

moveProcessMemoryRegion:                                   ; ebx=base address (bx include desired flags), ecx = # pages, edx = PID, edi=source base address, esi=source PID
    pusha

    push ebx

    and edx, 0xFF
    shl edx, PROCESS_VIRTUAL_MEMORY_MAP_ENRTY_SIZE_SHL_BITS
    add edx, PROCESS_VIRTUAL_MEMORY_MAP_ADDRESS
    shr ebx, 10
    and bl, 0xFC
    add edx, ebx

    and esi, 0xFF
    shl esi, PROCESS_VIRTUAL_MEMORY_MAP_ENRTY_SIZE_SHL_BITS
    add esi, PROCESS_VIRTUAL_MEMORY_MAP_ADDRESS
    shr edi, 10
    and di, 0xFFFC
    add esi, edi

    pop ebx
    xor bh, bh
    ;shl bl, PAGING_FLAG_WRITE_SHIFT_BITS
    or bx, PAGING_FLAG_PRESENT | PAGING_FLAG_USER

    moveProcessMemoryRegion_loop:
        mov eax, [esi]
        and ax, PAGING_LOW_BIT_REVERSE_MASK
        or ax, bx
        mov [edx], eax
        mov [esi], dword 0

        add edx, 4
        add esi, 4
        loop moveProcessMemoryRegion_loop
    
    popa
    ret

copyProcessMemoryRegion:                                   ; ebx=base address (bx include desired flags), ecx = # pages, edx = PID, edi=source base address, esi=source PID
    pusha

    push ebx

    and edx, 0xFF
    shl edx, PROCESS_VIRTUAL_MEMORY_MAP_ENRTY_SIZE_SHL_BITS
    add edx, PROCESS_VIRTUAL_MEMORY_MAP_ADDRESS
    shr ebx, 10
    and bl, 0xFC
    add edx, ebx

    and esi, 0xFF
    shl esi, PROCESS_VIRTUAL_MEMORY_MAP_ENRTY_SIZE_SHL_BITS
    add esi, PROCESS_VIRTUAL_MEMORY_MAP_ADDRESS
    shr edi, 10
    and di, 0xFFFC
    add esi, edi

    pop ebx
    and bx, PAGING_LOW_BIT_MASK
    or bx, PAGING_FLAG_PRESENT | PAGING_FLAG_USER | OS_PAGING_FLAG_SHARED

    copyProcessMemoryRegion_loop:
        mov eax, [esi]
        and ax, PAGING_LOW_BIT_REVERSE_MASK
        or ax, bx
        mov [edx], eax

        add edx, 4
        add esi, 4
        loop copyProcessMemoryRegion_loop
    
    popa
    ret

changeFlagsForMemoryRegion:                                 ; ebx=base address (with desired flags), ecx = # pages, edx = PID
    pusha

    push ebx

    and edx, 0xFF
    shl edx, PROCESS_VIRTUAL_MEMORY_MAP_ENRTY_SIZE_SHL_BITS
    add edx, PROCESS_VIRTUAL_MEMORY_MAP_ADDRESS
    shr ebx, 10
    and bl, 0xFC
    add edx, ebx

    pop ebx
    and bx, PAGING_NON_CPU_BIT_MASK

    changeFlagsForMemoryRegion_loop:
        mov eax, [edx]
        and ax, PAGING_REVERSE_NON_CPU_BIT_MASK 
        or ax, bx
        mov [edx], eax

        add edx, 4
        loop changeFlagsForMemoryRegion_loop
    
    popa
    ret


;TODO: REMOVE
zeroPagePagingEnabled:                       ; eax = page address
    push eax
    push ecx
    push edi
    push ebx

    mov ebx, eax
    mov eax, ZERO_PAGE_SPINLOCK
    call openSpinlock

    or bx, OS_PAGING_FLAG_GENERIC_PROTECTED | PAGING_FLAG_PRESENT | PAGING_FLAG_USER
    mov [KERNEL_PDE_ADDRESS + 12], ebx
    mov edi, KERNEL_PDE_RECURSIVE_ACCESS + 3 * PAGE_SIZE
    invlpg [edi]
    mov ecx, PAGE_SIZE / 4
    xor eax, eax
    rep stosd

   ; mov edi, PROCESS_VIRTUAL_MEMORY_MAP_ENRTY_SIZE * 2 + 3 * PAGE_SIZE
    mov [KERNEL_PDE_ADDRESS + 12], dword 0
    invlpg [KERNEL_PDE_RECURSIVE_ACCESS + 3 * PAGE_SIZE]

    mov eax, ZERO_PAGE_SPINLOCK
    call closeSpinlock

    pop ebx
    pop edi
    pop ecx
    pop eax

    ret