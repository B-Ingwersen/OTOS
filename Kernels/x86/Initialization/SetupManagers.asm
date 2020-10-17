PROCESS_MANAGER_PID equ 0x00010001
DISK_MANAGER_PID equ 0x00010002
ROOT_THREAD_STACK_LOCATION equ 0xFFB00000
ROOT_THREAD_CONTEXT_LOCATION equ 0xFFB01000

SetupManagers:
    pusha

    .setupProcessManager:
        call Thunk_ECX
        mov ebx, [ecx + LoadLocationTable_ProcessManager_Start]
        mov ecx, [ecx + LoadLocationTable_ProcessManager_Length]
        mov dx, cx
        and dx, 0x0FFF
        cmp dx, 0
        je .processManager_ExactPageLength
            add ecx, 0x1000
        .processManager_ExactPageLength:
        shr ecx, 12
        mov eax, 0x00010001
        call setupProcess

        mov eax, 0x00010001
        mov ebx, PROCESS_PERMISSION_PROCESS_MANAGEMENT
        call setProcessPermission
        mov ebx, PROCESS_PERMISSION_OTHER_PROCESS_MEMORY
        call setProcessPermission
        mov ebx, PROCESS_PERMISSION_OTHER_PROCESS_THREAD
        call setProcessPermission
        mov ebx, PROCESS_PERMISSION_KERNEL_MEMORY
        call setProcessPermission

    .setupDiskManager:
        call Thunk_ECX
        mov ebx, [ecx + LoadLocationTable_DiskManager_Start]
        mov ecx, [ecx + LoadLocationTable_DiskManager_Length]
        mov dx, cx
        and dx, 0x0FFF
        cmp dx, 0
        je .diskManager_ExactPageLength
            add ecx, 0x1000
        .diskManager_ExactPageLength:
        shr ecx, 12
        mov eax, 0x00010002
        call setupProcess

        mov eax, 0x00010002
        mov ebx, PROCESS_PERMISSION_BIOS_CALL
        call setProcessPermission
        mov ebx, PROCESS_PERMISSION_KERNEL_MEMORY
        call setProcessPermission
        mov ebx, PROCESS_PERMISSION_OTHER_PROCESS_MEMORY
        call setProcessPermission

    popa
    ret

setupProcess:                ;eax = PID, ebx=baseAddress, ecx=#pages
    pusha
    push ecx
    push ebx
    push eax

    call GetKernelAddress_ECX
    mov edi, ecx

    mov eax, [esp]
    mov ecx, [edi + EXTERNAL_CALL_CreateProcess]
    add ecx, edi
    call ecx

    mov ebx, 0x100000 | PAGING_FLAG_WRITE | PAGING_FLAG_USER
    mov ecx, [esp + 8]
    mov edx, [esp]
    mov eax, MEMORY_SYSTEM_CALL_NEW_MEMORY
    mov esi, [edi + EXTERNAL_CALL_MapNewOrKernelMemoryIntoProcess]
    add esi, edi
    call esi

    mov ebx, ROOT_THREAD_STACK_LOCATION | PAGING_FLAG_WRITE | PAGING_FLAG_USER
    mov ecx, 1
    mov edx, [esp]
    mov eax, MEMORY_SYSTEM_CALL_NEW_MEMORY
    mov esi, [edi + EXTERNAL_CALL_MapNewOrKernelMemoryIntoProcess]
    add esi, edi
    call esi

    mov eax, [esp]
    mov ecx, [edi + EXTERNAL_CALL_EnterJustProcess]
    add ecx, edi
    call ecx

        pusha
            mov esi, [esp + 32 + 4]
            mov edi, 0x100000
            mov ecx, [esp + 32 + 8]
            shl ecx, 12
            rep movsb
        popa

    mov eax, [esp]
    mov ebx, KERNEL_PDE_ADDRESS
    mov ecx, [edi + EXTERNAL_CALL_ExitJustProcess]
    add ecx, edi
    call ecx

    push edi
        mov eax, [edi + EXTERNAL_CALL_CreateThread]
        add eax, edi
        
        mov edx, [esp + 4]
        mov ecx, ROOT_THREAD_CONTEXT_LOCATION
        mov edi, 0x100000
        mov esi, ROOT_THREAD_STACK_LOCATION + 0xFFC

        call eax
    pop edi

    mov eax, [esp]
    mov ebx, ROOT_THREAD_CONTEXT_LOCATION
    mov ecx, [edi + EXTERNAL_CALL_AddThreadToScheduler]
    add ecx, edi
    call ecx

    pop eax
    pop ebx
    pop ecx
    popa
    ret

setProcessPermission:               ; eax = pid, ebx = permission
    pusha

    .getKernelThunk:
        push ecx
        call GetKernelAddress_ECX
        mov edi, ecx
        pop ecx

    mov edx, eax
    mov ecx, 1
    mov eax, [edi + EXTERNAL_CALL_ChangeProcessPermission]
    add eax, edi
    call eax

    popa
    ret