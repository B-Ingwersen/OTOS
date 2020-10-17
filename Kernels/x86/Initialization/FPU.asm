
FPU_Initialize:

    mov eax, cr0
    and eax, 0xFFFFFFF3     ; clear EM, TS
    or eax, 0x00000032      ; set ET, NE, MP
    mov cr0, eax

    mov eax, cr4
    and eax, 0xFFFCF9FF
    mov cr4, eax

    finit
    ; set the control word
    ; respect positive & negative infinity
    ; round to nearest
    ; 64 bit rounding
    ; mask all interrupts
    mov eax, 0x137F
    push eax
    fldcw [esp]
    pop eax

    ret
