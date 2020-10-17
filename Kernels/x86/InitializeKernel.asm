[bits 32]
jmp start

times 4 - ($ - $$) db 0

LoadLocationTable_InitializeKernel_Start:   dd 0
LoadLocationTable_InitializeKernel_Length:  dd 0
LoadLocationTable_Kernel_Start:             dd 0
LoadLocationTable_Kernel_Length:            dd 0
LoadLocationTable_ProcessManager_Start:     dd 0
LoadLocationTable_ProcessManager_Length:    dd 0
LoadLocationTable_DiskManager_Start:        dd 0
LoadLocationTable_DiskManager_Length:       dd 0
LoadLocationTable_BootGDTDescriptor:        dd 0

start:
mov esp, 0x8000
mov ebp, esp

mov [0x5000], dword 0xB80A0                 ; For debugging only -- TODO: remove w/ other debugging code

.zeroAllTables:
    mov edi, KERNEL_DATA_AREA_BASE_ADDRESS
    mov ecx, KERNEL_DATA_AREA_LENGTH
    xor eax, eax
    rep stosb

call IDT_Initialize
mov [0xB8000], byte '1'
call Memory_Initialize
mov [0xB8000], byte '2'

call FPU_Initialize
call Multiprocessor_Initialize
mov [0xB8000], byte '3'
call Scheduler_Initialize
mov [0xB8000], byte '4'
call Timers_InsertInterrupt
mov [0xB8000], byte '5'
call SystemCalls_Initialize

mov [0xB8000], byte '6'

;mov ecx, 800
;mov edx, 0x102000
;.printPagesLoop:
;    push ecx
;    mov ecx, 4
;    ;call debugging_PrintHex
;    pop ecx
;
;    add edx, 4
;
;    loop .printPagesLoop

.moveToPagedEnvironment:
    mov eax, KERNEL_PDE_ADDRESS
    mov cr3, eax
    mov eax, cr0
    or eax, 0x80000001
    mov cr0, eax

mov [0xB8000], byte '7'

call SetupManagers

call GetKernelAddress_ECX
add ecx, [ecx + EXTERNAL_CALL_EnterSchedulerNoReschedule]
mov [MULTIPROCESSOR_INITIALIZATION_DONE], dword 1
cmp [MULTIPROCESSOR_TYPE], dword MULTIPROCESSOR_TYPE_SMP
    jne .noSMPIntEnable
    sti
    .noSMPIntEnable:
jmp ecx

Thunk_ECX:
    call .thunk
    .thunk:
    pop ecx
    sub ecx, .thunk
    ret

GetKernelAddress_ECX:
    call Thunk_ECX
    mov ecx, [ecx + LoadLocationTable_Kernel_Start]
    ret

Debugging_PrintString:
    pusha

    call Thunk_ECX
    add eax, ecx
    mov ebx, [0x5000]
    .loop:
        mov cl, [eax]
        ;mov [ebx], cl                  ; Uncomment lines to reenable debugging messages!
        ;mov [ebx + 1], byte 0x0F
        add ebx, 2
        inc eax

        cmp cl, 0
        jne .loop
    
    mov [0x5000], ebx

    popa
    ret


debugging_PrintHex:     ; ecx = iterations, edx = data
    pusha
    mov ebx, [0x5000]
    add ebx, ecx
    add ebx, ecx
    add ebx, ecx
    add ebx, ecx
    mov [0x5000], ebx
    add [0x5000], dword 4
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

%include "./Definitions.asm"
%include "./Initialization/IDT.asm"
%include "./Initialization/Memory.asm"
%include "./Initialization/FPU.asm"
%include "./Initialization/Multiprocessor.asm"
%include "./Initialization/Timers.asm"
%include "./Initialization/Scheduler.asm"
%include "./Initialization/SystemCalls.asm"
%include "./Initialization/SetupManagers.asm"
%include "./Initialization/InterruptIO.asm"
