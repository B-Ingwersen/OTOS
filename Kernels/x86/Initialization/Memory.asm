
Memory_Initialize:
    pusha

    .calculateMemorySize:

    xor ecx, ecx                                    ; ecx = 0; ecx stores the accumulated size
    xor edx, edx
    push dword MEMORY_KENREL_MEMORY_MAX_ADDRESS

    .fillMemoryTableEntryPoint:

    mov esi, MEMORY_FREE_MEMORY_TABLE_ADDRESS         ; esi is pointer
    mov edi, [MEMORY_FREE_MEMORY_TABLE_ADDRESS]       ; edi is the maximum pointer
    shl edi, 5
    add edi, MEMORY_FREE_MEMORY_TABLE_ADDRESS

    .biosTableLoop:
        add esi, MEMORY_FREE_MEMORY_TABLE_ENTRY_SIZE
        cmp esi, edi
        jae .biosTableEnd

        ; DEBUGGING -- print the memory table
            ;cmp edx, MEMORY_INITIALIZE_MEMORY_FILL_TABLE_MODE
            ;jne .noPrint
            ;pusha
            ;    mov edx, esi
            ;    mov ecx, 8
            ;    call debugging_PrintHex
            ;    
            ;    add edx, 8
            ;    mov ecx, 8
            ;    call debugging_PrintHex
            ;
            ;    add edx, 8
            ;    mov ecx, 4
            ;    call debugging_PrintHex
            ;    
            ;    add [0x5000], dword 68
            ;popa
            ;.noPrint:

        ; check if segment is flagged as free
        cmp [esi + 16], dword 1
        jne .biosTableLoop

        mov eax, [esi + 4]                          ; get the higher 4 bits
        test eax, eax                               ; ignore segments starting above 4gb
        jne .biosTableLoop

        ; push up bottom of segment to above kernel memory
        mov eax, [esi]                              ; get the 32 bit base address
        cmp eax, [esp]
        jae .segmentNotBelowKernelMemory
        mov eax, [esp]
        .segmentNotBelowKernelMemory:

        ; push up bottom of segment to a page boundary
        mov bx, ax
        and bx, PAGING_LOW_BIT_MASK
        cmp bx, 0
        je .lowSegmentOnPage
        and ax, PAGING_LOW_BIT_REVERSE_MASK
        add eax, PAGE_SIZE
        .lowSegmentOnPage:

        ; make ebx the max of the segment; restrict it to 4GB, and constrain it
        ; to a page boundary
        mov ebx, [esi + 12]
        test ebx, ebx
        je .highSegmentBelow4Gig
            mov ebx, FOUR_GIG_MINUS_ONE_PAGE
            jmp .highSegmentAbove4Gig
        .highSegmentBelow4Gig:
            mov ebx, [esi + 8]
            and bx, PAGING_LOW_BIT_REVERSE_MASK
        .highSegmentAbove4Gig:

        ; ignore segment if it has no valid entries in it
        cmp eax, ebx
        jae .biosTableLoop

        ; cehck if filling the memory table or accumulating entries
        cmp edx, MEMORY_INITIALIZE_MEMORY_FILL_TABLE_MODE
        je .fillTableMode_start

        ; accumulate the number of valid pages in ecx
        sub ebx, eax
        shr ebx, PAGE_SHIFT_RIGHT_BITS
        add ecx, ebx
        jmp .biosTableLoop

        ; saves pages to the page table; ecx is the address in the page table,
        ; eax is the page address itself, ebx is the maximum page
        .fillTableMode_start:
        ;pusha
        ;    push eax
        ;    mov edx, esp
        ;    mov ecx, 4
        ;    call debugging_PrintHex
        ;    mov [esp], ebx
        ;    mov ecx, 4
        ;    call debugging_PrintHex
        ;    pop eax
        ;    add [0x5000], dword 120
        ;popa

        .fillTableMode:
            mov [ecx], eax
            add eax, PAGE_SIZE
            add ecx, 4
            cmp eax, ebx
            jb .fillTableMode
        
        jmp .biosTableLoop

    .biosTableEnd:

    ; if page table entries filled in, then process is done
    cmp edx, MEMORY_INITIALIZE_MEMORY_FILL_TABLE_MODE
    je .doneSegmentAccumulation


    ; zero the tables that define the kernel's virtual address space
    push ecx
    mov edi, KERNEL_PDE_ADDRESS
    mov ecx, (PAGE_SIZE / 4)
    xor eax, eax
    rep stosd
    mov edi, MEMORY_KERNEL_CORE_PTE_ADDRESS
    mov ecx, (PAGE_SIZE / 4)
    rep stosd
    mov edi, MEMORY_MEM_TABLE_PTE_ADDRESS
    mov ecx, (PAGE_SIZE / 4)
    rep stosd
    pop ecx

    ; calculate the size and end address of the memory table
    mov ebx, ecx
    shl ebx, 2                                      ; ebx = size of memory table
    add ebx, MEMORY_MEM_TABLE_BASE + PAGE_SIZE
    and bx, PAGING_LOW_BIT_REVERSE_MASK             ; ebx = memory table end address
    mov [esp], ebx                                  ; save where the end of the table is
    mov [MEMORY_TABLE_POINTER_TO_END_ADDRESS], ebx

    ; add pte entries for the memory table
    mov eax, MEMORY_MEM_TABLE_BASE | PAGING_FLAG_PRESENT | PAGING_FLAG_WRITE
    mov edx, MEMORY_MEM_TABLE_PTE_ADDRESS
    .fillMemoryTablePTE:
        mov [edx], eax
        add eax, PAGE_SIZE
        add edx, 4
        cmp eax, ebx
        jb .fillMemoryTablePTE
    
    ; fill in 1:1 mapping for page up to the memory table (0x0 - 0x102000)
    xor eax, eax
    or al, PAGING_FLAG_PRESENT | PAGING_FLAG_WRITE
    mov ebx, MEMORY_MEM_TABLE_BASE
    mov edx, MEMORY_KERNEL_CORE_PTE_ADDRESS
    .fillKernelCorePTE:
        mov [edx], eax
        add eax, PAGE_SIZE
        add edx, 4
        cmp eax, ebx
        jb .fillKernelCorePTE

    ; fill basic PDE entries
    mov eax, KERNEL_PDE_ADDRESS
    mov [eax], dword MEMORY_KERNEL_CORE_PTE_ADDRESS | PAGING_FLAG_PRESENT | PAGING_FLAG_WRITE   ; 1:1 mappings of 0x0 - 0x102000
    mov [eax + 4], dword MEMORY_MEM_TABLE_PTE_ADDRESS | PAGING_FLAG_PRESENT | PAGING_FLAG_WRITE ; memory table
    mov [eax + 8], dword KERNEL_PDE_ADDRESS | PAGING_FLAG_PRESENT | PAGING_FLAG_WRITE           ; recursive mapping

    mov ecx, MEMORY_MEM_TABLE_BASE
    mov [MEMORY_TABLE_POINTER_TO_DEQUEUE], ecx 
    mov [MEMORY_TABLE_POINTER_TO_START_ADDRESS], ecx
    mov edx, MEMORY_INITIALIZE_MEMORY_FILL_TABLE_MODE
    jmp .fillMemoryTableEntryPoint

    .doneSegmentAccumulation:
    mov [MEMORY_TABLE_POINTER_TO_ENQUEUE], ecx                  ; save where next freed page should go in memory table

    ; adjust addresses because table will be accessed under paging
    mov eax, MEMORY_MEM_TABLE_VIRTUAL_MEMORY_OFFSET
    add [MEMORY_TABLE_POINTER_TO_ENQUEUE], eax
    add [MEMORY_TABLE_POINTER_TO_DEQUEUE], eax
    add [MEMORY_TABLE_POINTER_TO_START_ADDRESS], eax
    add [MEMORY_TABLE_POINTER_TO_END_ADDRESS], eax

    pop eax                                                     ; fix the stack

    popa
    ret

MEMORY_FREE_MEMORY_TABLE_ADDRESS        equ 0x1000
MEMORY_FREE_MEMORY_TABLE_ENTRY_SIZE     equ 0x20

MEMORY_INITIALIZE_MEMORY_FILL_TABLE_MODE    equ 1

MEMORY_MEM_TABLE_PTE_ADDRESS            equ MEMORY_KENREL_MEMORY_MAX_ADDRESS + 1 * PAGE_SIZE
MEMORY_MEM_TABLE_BASE                   equ MEMORY_KENREL_MEMORY_MAX_ADDRESS + 2 * PAGE_SIZE
MEMORY_MEM_TABLE_VIRTUAL_MEMORY_BASE    equ 0x400000
MEMORY_MEM_TABLE_VIRTUAL_MEMORY_OFFSET  equ MEMORY_MEM_TABLE_VIRTUAL_MEMORY_BASE - MEMORY_MEM_TABLE_BASE