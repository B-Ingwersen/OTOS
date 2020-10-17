MEMORY_SYSTEM_CALL_TYPE_MAP_MEMORY equ 0x0
MEMORY_SYSTEM_CALL_TYPE_UNMAP_MEMORY equ 0x1
MEMORY_SYSTEM_CALL_TYPE_MOVE_MEMORY equ 0x2
MEMORY_SYSTEM_CALL_TYPE_MAP_KERNEL_MEMORY equ 0x3
SYSTEM_CALL_MEMORY_MAP_NEW_MEMORY equ 0x0
SYSTEM_CALL_MEMORY_MAP_KERNEL_MEMORY equ 0x1
SYSTEM_CALL_PERMISSION_NONE equ 0

memorySystemCall:
    push ebp                                                ; STACK +4
    push edx                                                ; STACK +4
    mov ebp, esp
    and bp, PROCESSOR_CONTEXT_BASE_BIT_MASK

    cmp eax, MEMORY_SYSTEM_CALL_TYPE_MAP_MEMORY
    je .mapMemory
    cmp eax, MEMORY_SYSTEM_CALL_TYPE_UNMAP_MEMORY
    je .unmapMemory
    cmp eax, MEMORY_SYSTEM_CALL_TYPE_MOVE_MEMORY
    je .moveMemory
    cmp eax, MEMORY_SYSTEM_CALL_TYPE_MAP_KERNEL_MEMORY
    je .mapKernelMemory

    .systemCallDoesntExist:

    jmp memorySystemCall_end_fail

    .mapMemory:
        and bx, (PAGING_LOW_BIT_REVERSE_MASK | 1)                        ; set read/write flag if applicable
        shl bl, 1
        or bl, PAGING_FLAG_USER

        mov eax, [esp + 20]
        cmp [eax], dword SYSTEM_CALL_PERMISSION_NONE
        je .mapMemory_noPermission

        ;TODO check capability
        .mapMemory_permissionCheck:
            cmp [PROCESS_INFO_ADDRESS + PROCESS_INFO_PERMISSIONS_BASE + PROCESS_PERMISSION_OTHER_PROCESS_MEMORY], dword 1
            je .mapMemory_permissionGranted

            push dword [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
            push dword 0x40
            push dword [eax]
            mov ebp, edx
            mov eax, MEMORY_SYSTEM_CALL_TYPE_MAP_MEMORY
            call checkCapability
            add esp, 12
            cmp eax, 1
            je .mapMemory_permissionGranted
            jmp memorySystemCall_end

        .mapMemory_noPermission:
        mov edx, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]

        .mapMemory_permissionGranted:

        mov eax, cr3
        push eax
        mov eax, KERNEL_PDE_ADDRESS
        mov cr3, eax

        mov eax, MEMORY_SYSTEM_CALL_NEW_MEMORY
        call mapNewOrKernelMemoryIntoProcess
        
        push eax
        mov eax, [esp + 4]
        mov cr3, eax
        pop eax
        add esp, 4

        jmp memorySystemCall_end   

    .unmapMemory:
        ; set kernel memory flag if needed
        mov ax, bx
        and bx, PAGING_LOW_BIT_REVERSE_MASK
        and ax, 0x1
        shl ax, 9
        or bx, ax

        mov eax, [esp + 20]
        cmp [eax], dword SYSTEM_CALL_PERMISSION_NONE
        je .unmapMemory_noPermission

        .unmapMemory_permissionCheck:
            cmp [PROCESS_INFO_ADDRESS + PROCESS_INFO_PERMISSIONS_BASE + PROCESS_PERMISSION_OTHER_PROCESS_MEMORY], dword 1
            je .unmapMemory_permissionGranted

            push dword [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
            push dword 0x40
            push dword [eax]
            mov ebp, edx
            mov eax, MEMORY_SYSTEM_CALL_TYPE_UNMAP_MEMORY
            call checkCapability
            add esp, 12
            cmp eax, 1
            je .unmapMemory_permissionGranted
            jmp memorySystemCall_end

        .unmapMemory_noPermission:
        mov edx, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]

        .unmapMemory_permissionGranted:

        mov eax, cr3
        push eax
        mov eax, KERNEL_PDE_ADDRESS
        mov cr3, eax

        call removeMemoryFromProcess

        push eax
        mov eax, [esp + 4]
        mov cr3, eax
        pop eax
        add esp, 4

        jmp memorySystemCall_end 

    .moveMemory:
        and bx, (PAGING_LOW_BIT_REVERSE_MASK | 1)                        ; set read/write flag if applicable
        shl bl, 1
        or bl, PAGING_FLAG_USER

        .moveMemory_checkSourcePermission:
            mov eax, [esp + 20]
            cmp [eax + 4], dword SYSTEM_CALL_PERMISSION_NONE
            je .mapMemory_noSourcePermission

            cmp [PROCESS_INFO_ADDRESS + PROCESS_INFO_PERMISSIONS_BASE + PROCESS_PERMISSION_OTHER_PROCESS_MEMORY], dword 1
            je .moveMemory_sourcePermissionGranted

            push dword [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
            push dword 0x40
            push dword [eax]
            mov ebp, edx
            mov eax, MEMORY_SYSTEM_CALL_TYPE_MOVE_MEMORY
            call checkCapability
            .moveMemory_resetProcessorContext:
                mov ebp, esp
                and bp, PROCESSOR_CONTEXT_BASE_BIT_MASK
            add esp, 12
            cmp eax, 1
            je .moveMemory_sourcePermissionGranted
            jmp memorySystemCall_end

            .mapMemory_noSourcePermission:
            mov esi, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]

            .moveMemory_sourcePermissionGranted:
        .moveMemory_CheckDestinationPermission:
            mov eax, [esp + 20]
            cmp [eax], dword SYSTEM_CALL_PERMISSION_NONE
            je .mapMemory_noDestinationPermission

            cmp [PROCESS_INFO_ADDRESS + PROCESS_INFO_PERMISSIONS_BASE + PROCESS_PERMISSION_OTHER_PROCESS_MEMORY], dword 1
            je .moveMemory_destinationPermissionGranted

            push dword [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
            push dword 0x40
            push dword [eax]
            mov ebp, edx
            mov eax, MEMORY_SYSTEM_CALL_TYPE_MOVE_MEMORY
            call checkCapability
            add esp, 12
            cmp eax, 1
            je .moveMemory_destinationPermissionGranted
            jmp memorySystemCall_end

            .mapMemory_noDestinationPermission:
            mov esi, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]

            .moveMemory_destinationPermissionGranted:
        
        mov eax, cr3
        push eax
        mov eax, KERNEL_PDE_ADDRESS
        mov cr3, eax

        mov eax, MEMORY_SYSTEM_CALL_MOVE_MEMORY
        call moveOrShareMemoryToProcess

        push eax
        mov eax, [esp + 4]
        mov cr3, eax
        pop eax
        add esp, 4

        jmp memorySystemCall_end   
    
    .mapKernelMemory:
        cmp [PROCESS_INFO_ADDRESS + PROCESS_INFO_PERMISSIONS_BASE + PROCESS_PERMISSION_KERNEL_MEMORY], dword 1
        jne memorySystemCall_end_fail
        
        and bx, (PAGING_LOW_BIT_REVERSE_MASK | 1)                        ; set read/write flag if applicable
        shl bl, 1
        or bl, PAGING_FLAG_USER
        
        mov eax, [esp + 20]
        cmp [eax], dword SYSTEM_CALL_PERMISSION_NONE
        je .mapKernelMemory_noPermission

        ;TODO check capability
        .mapKernelMemory_permissionCheck:
            cmp [PROCESS_INFO_ADDRESS + PROCESS_INFO_PERMISSIONS_BASE + PROCESS_PERMISSION_OTHER_PROCESS_MEMORY], dword 1
            je .mapKernelMemory_permissionGranted

            push dword [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
            push dword 0x40
            push dword [eax]
            mov ebp, edx
            mov eax, MEMORY_SYSTEM_CALL_TYPE_MAP_KERNEL_MEMORY
            call checkCapability
            add esp, 12
            cmp eax, 1
            je .mapKernelMemory_permissionGranted
            jmp memorySystemCall_end

        .mapKernelMemory_noPermission:
        mov edx, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]

        .mapKernelMemory_permissionGranted:

        mov eax, cr3
        push eax
        mov eax, KERNEL_PDE_ADDRESS
        mov cr3, eax

        mov eax, MEMORY_SYSTEM_CALL_KERNEL_MEMORY
        call mapNewOrKernelMemoryIntoProcess

        push eax
        mov eax, [esp + 4]
        mov cr3, eax
        pop eax
        add esp, 4

        jmp memorySystemCall_end 
        
        

    memorySystemCall_end_fail:
    xor eax, eax
    memorySystemCall_end:

    pop edx
    pop ebp
    jmp systemCallEnd

SYSTEM_CALL_CAPABILITIES_SETUP equ 0
SYSTEM_CALL_CAPABILITIES_DELETE equ 1

capabilitiesSystemCall:
    push ebp
    mov ebp, esp
    and bp, PROCESSOR_CONTEXT_BASE_BIT_MASK

    push edx
    mov edx, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
    call processCapabilitiesOperations
    pop edx

    pop ebp
    jmp systemCallEnd

SYSTEM_CALL_BIOS_CALL_READ equ 1
SYSTEM_CALL_BIOS_CALL_WRITE equ 2
SYSTEM_CALL_BIOS_KERNEL_RW_LOCATION equ 0x10000

biosSystemCall:
    cmp [PROCESS_INFO_ADDRESS + PROCESS_INFO_PERMISSIONS_BASE + PROCESS_PERMISSION_BIOS_CALL], dword 1
    jne .fail

    push ebp
    push eax

    mov eax, BIOS_CALL_SPINLOCK
    call openSpinlock

    .writeDataToBIOSCallArea:
        mov eax, [esp]
        and eax, SYSTEM_CALL_BIOS_CALL_WRITE
        cmp eax, 0
        je .noWrite

        pusha
        mov edi, SYSTEM_CALL_BIOS_KERNEL_RW_LOCATION
        mov ecx, edx
        rep movsb
        popa

        .noWrite:

    .copyVariableStack:
        pusha
        mov esi, ebx
        mov edi, BIOS_CALL_VARIABLE_STACK_BASE
        mov ecx, BIOS_CALL_VARIABLE_STACK_SIZE
        rep movsb
        popa

    .setCheckStatus:
        xor eax, eax
        mov [BIOS_CALL_OPERATION_STATUS], eax

    .callBootProcessor:
        cmp [MULTIPROCESSOR_TYPE], dword MULTIPROCESSOR_TYPE_NO_SMP
        je .callBootProcessor_OnBootProcessor

        mov eax, [BIOS_CALL_PROCESSOR_LOCAL_APIC_ID]
        cmp eax, [LOCAL_APIC_PAGED_LOCATION + LOCAL_APIC_ID_REGISTER_OFFSET]
        je .callBootProcessor_OnBootProcessor
        mov [LOCAL_APIC_PAGED_LOCATION + LOCAL_APIC_INTERRUPT_COMMAND_REGISTER_UPPER_OFFSET], ecx
        mov eax, 0x00004000 + INTERRUPT_ID_BIOS_KERNEL_CALL
        mov [LOCAL_APIC_PAGED_LOCATION + LOCAL_APIC_INTERRUPT_COMMAND_REGISTER_LOWER_OFFSET], eax
        jmp .callBootProcessor_end

        .callBootProcessor_OnBootProcessor:
            int INTERRUPT_ID_BIOS_KERNEL_CALL
        
        .callBootProcessor_end:

    .waitForOperationCompleteLoop:
        xor eax, eax
        cmp [BIOS_CALL_OPERATION_STATUS], eax
        je .waitForOperationCompleteLoop
    
    .copyBackVariableStack:
        pusha
        mov esi, BIOS_CALL_VARIABLE_STACK_BASE
        mov edi, ebx
        mov ecx, BIOS_CALL_VARIABLE_STACK_SIZE
        rep movsb
        popa

    .readDataFromBiosCallArea:
        mov eax, [esp]
        and eax, SYSTEM_CALL_BIOS_CALL_READ
        cmp eax, 0
        je .noRead

        pusha
        mov esi, SYSTEM_CALL_BIOS_KERNEL_RW_LOCATION
        rep movsb
        popa

        .noRead:
    
    mov eax, BIOS_CALL_SPINLOCK
    call closeSpinlock

    pop eax
    pop ebp

    xor eax, eax
    inc eax
    jmp systemCallEnd

    .fail:
    xor eax, eax
    jmp systemCallEnd

SYSTEM_CALL_PROCESS_THREAD_CREATE_THREAD equ 0
SYSTEM_CALL_PROCESS_THREAD_DESTROY_THREAD equ 1
SYSTEM_CALL_PROCESS_THREAD_CHANGE_THREAD_EXECUTION equ 2
SYSTEM_CALL_PROCESS_THREAD_CREATE_PROCESS equ 3
SYSTEM_CALL_PROCESS_THREAD_DESTROY_PROCESS equ 4
SYSTEM_CALL_PROCESS_THREAD_CHANGE_PROCESS_PERMISSION equ 5

SYSTEM_CALL_CHANGE_THREAD_EXECUTION_YIELD_TIME_SLICE equ 0
SYSTEM_CALL_CHANGE_THREAD_EXECUTION_SCHEDULE equ 1
SYSTEM_CALL_CHANGE_THREAD_EXECUTION_HALT equ 2

processThreadSystemCall:
    push ebp                                                ; STACK +4
    mov ebp, esp
    and bp, PROCESSOR_CONTEXT_BASE_BIT_MASK

    cmp eax, SYSTEM_CALL_PROCESS_THREAD_CREATE_THREAD
    je .createThread
    cmp eax, SYSTEM_CALL_PROCESS_THREAD_CREATE_PROCESS
    je .createProcess
    cmp eax, SYSTEM_CALL_PROCESS_THREAD_CHANGE_THREAD_EXECUTION
    je .changeThreadExecution
    cmp eax, SYSTEM_CALL_PROCESS_THREAD_DESTROY_THREAD
    je .destroyThread
    cmp eax, SYSTEM_CALL_PROCESS_THREAD_DESTROY_PROCESS
    je .destroyProcess
    cmp eax, SYSTEM_CALL_PROCESS_THREAD_CHANGE_PROCESS_PERMISSION
    je .changeProcessPermission

    xor eax, eax
    jmp .end

    .createThread:
        mov eax, [esp + 16]
        cmp [eax], dword SYSTEM_CALL_PERMISSION_NONE
        je .createThread_noPermission

        cmp [PROCESS_INFO_ADDRESS + PROCESS_INFO_PERMISSIONS_BASE + PROCESS_PERMISSION_OTHER_PROCESS_THREAD], dword 1
        je .createThread_permissionGranted

        push dword [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
        push dword 0x41
        push dword [eax]
        mov ebp, edx
        mov eax, SYSTEM_CALL_PROCESS_THREAD_DESTROY_THREAD
        call checkCapability
        add esp, 12
        cmp eax, 1
        je .createThread_permissionGranted
        jmp .end
    
        .createThread_noPermission:
        mov edx, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
        .createThread_permissionGranted:

        mov eax, cr3
        push eax
        mov eax, KERNEL_PDE_ADDRESS
        mov cr3, eax

        call createThread

        push ebx
        mov ebx, [esp + 4]
        mov cr3, ebx
        pop ebx
        add esp, 4

        jmp .end


    .createProcess:
        xor eax, eax
        cmp [PROCESS_INFO_ADDRESS + PROCESS_INFO_PERMISSIONS_BASE + PROCESS_PERMISSION_PROCESS_MANAGEMENT], dword 1
        jne .end

        mov eax, cr3
        push eax
        mov eax, KERNEL_PDE_ADDRESS
        mov cr3, eax

        mov eax, edx
        call createProcess

        push ebx
        mov ebx, [esp + 4]
        mov cr3, ebx
        pop ebx
        add esp, 4

        jmp .end
    
    .changeThreadExecution: ; ecx = sub operation, ebx = thread, edx = pid
        .testYieldTimeSlice:
            cmp ecx, SYSTEM_CALL_CHANGE_THREAD_EXECUTION_YIELD_TIME_SLICE
            jne .notYieldTimeSlice
                pop ebp

                push eax
                push ebx
                push ecx
                push edx
                push edi
                push esi
                push ebp

                mov ebp, esp
                and bp, PROCESSOR_CONTEXT_BASE_BIT_MASK
                jmp timeSliceInterrupt_userModeInterrupt

            .notYieldTimeSlice:

        mov eax, [esp + 16]                             ; get program's userspace stack pointer
        cmp [eax], dword SYSTEM_CALL_PERMISSION_NONE    ; [eax] = permission
        je .changeThreadExecution_noPermission

        cmp [PROCESS_INFO_ADDRESS + PROCESS_INFO_PERMISSIONS_BASE + PROCESS_PERMISSION_OTHER_PROCESS_THREAD], dword 1
        je .changeThreadExecution_permissionGranted

        push dword [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
        push dword 0x41
        push dword [eax]
        mov ebp, edx
        mov eax, SYSTEM_CALL_PROCESS_THREAD_CREATE_PROCESS
        call checkCapability
        add esp, 12
        cmp eax, 1
        je .changeThreadExecution_permissionGranted
        jmp .end
    
        .changeThreadExecution_noPermission:
        mov edx, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]  ; if using no permission, change current process to calling process
        .changeThreadExecution_permissionGranted:

        mov eax, ecx                                        ; eax = operation
        call changeThreadExecution

        jmp .end

    .destroyThread:
        mov eax, [esp + 16]
        cmp [eax], dword SYSTEM_CALL_PERMISSION_NONE
        je .destroyThread_noPermission

        cmp [PROCESS_INFO_ADDRESS + PROCESS_INFO_PERMISSIONS_BASE + PROCESS_PERMISSION_OTHER_PROCESS_THREAD], dword 1
        je .destroyThread_permissionGranted

        push dword [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
        push dword 0x41
        push dword [eax]
        mov ebp, edx
        mov eax, SYSTEM_CALL_PROCESS_THREAD_DESTROY_THREAD
        call checkCapability
        add esp, 12
        cmp eax, 1
        je .destroyThread_permissionGranted
        jmp .end
    
        .destroyThread_noPermission:
        mov edx, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
        .destroyThread_permissionGranted:

        mov eax, cr3
        push eax

        mov eax, edx
        call enterJustProcess
        cmp eax, PROCESS_ENTER_SUCCESS
        jne .end

        and bx, PAGING_LOW_BIT_REVERSE_MASK
        call deleteThread
        
        .freeStackPage:
            mov eax, KERNEL_PDE_ADDRESS
            mov cr3, eax

            push ebx
            cmp esi, dword 0
            je .skipFreeStackPage
            mov ebx, esi
            call removeMemoryFromProcess
            pop ebx

        .skipFreeStackPage:

        push ebx
        mov ebx, [esp + 4]
        mov eax, edx
        call exitJustProcess
        pop ebx
        add esp, 4

        .checkOwnProcessAndThread:
            cmp edx, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
            jne .notOwnProcessAndThread
            cmp ebx, [ebp + PROCESSOR_CONTEXT_CURRENT_THREAD_PAGED_ADDRESS]
            jne .notOwnProcessAndThread

            mov eax, edx
            mov ebx, KERNEL_PDE_ADDRESS
            call exitJustProcess

            mov edx, 0xDEAD0003
            jmp enterSchedulerNoReschedule

        .notOwnProcessAndThread:

        jmp .end

    .destroyProcess:
        mov eax, [esp + 16]
        cmp [eax], dword SYSTEM_CALL_PERMISSION_NONE
        je .destroyProcess_noPermission

        cmp [PROCESS_INFO_ADDRESS + PROCESS_INFO_PERMISSIONS_BASE + PROCESS_PERMISSION_PROCESS_MANAGEMENT], dword 1
        je .destroyProcess_permissionGranted

        push dword [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
        push dword 0x41
        push dword [eax]
        mov ebp, edx
        mov eax, SYSTEM_CALL_PROCESS_THREAD_DESTROY_PROCESS
        call checkCapability
        add esp, 12
        cmp eax, 1
        je .destroyProcess_permissionGranted
        jmp .end
    
        .destroyProcess_noPermission:
        mov edx, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
        .destroyProcess_permissionGranted:

        mov eax, cr3
        push eax

        call deleteProcess
        cmp eax, dword 1
        jne .destroyProcessFail

        .checkIfFromCurrentProcess:
            ;TODO check if currentProcess = pid, if so, go to scheduler no resched.
            push edx
            cmp edx, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
            mov edx, 0xDEAD0004
            je enterSchedulerNoReschedule
            pop edx
        
        .success:
            pop eax
            mov cr3, eax
            xor eax, eax
            inc eax
            jmp .end

        .destroyProcessFail:

        pop eax
        mov cr3, eax
        xor eax, eax
        jmp .end

    .changeProcessPermission:                              ; edx = PID, ebx = permission, ecx = value
        xor eax, eax
        cmp [PROCESS_INFO_ADDRESS + PROCESS_INFO_PERMISSIONS_BASE + PROCESS_PERMISSION_PROCESS_MANAGEMENT], dword 1
        jne .end

        call changeProcessPermission
        jmp .end

    .end:

    pop ebp
    jmp systemCallEnd

SYSTEM_CALL_SHARED_MEMORY_INITIALIZE equ 0
SYSTEM_CALL_SHARED_MEMORY_CREATE_SEGMENT equ 1
SYSTEM_CALL_SHARED_MEMORY_MODIFY_SEGMENT_PERMISSIONS equ 2
SYSTEM_CALL_SHARED_MEMORY_MAP_SEGMENT equ 3
SYSTEM_CALL_SHARED_MEMORY_UNMAP_SEGMENT equ 4
SYSTEM_CALL_SHARED_MEMORY_DESTROY_SEGMENT equ 5
SYSTEM_CALL_SHARED_MEMORY_DESTROY_ALL equ 6
sharedMemorySystemCall:
    push ebp
    mov ebp, esp
    and bp, PROCESSOR_CONTEXT_BASE_BIT_MASK

    cmp eax, SYSTEM_CALL_SHARED_MEMORY_INITIALIZE
    je .initialize
    cmp eax, SYSTEM_CALL_SHARED_MEMORY_CREATE_SEGMENT
    je .createSegment
    cmp eax, SYSTEM_CALL_SHARED_MEMORY_MODIFY_SEGMENT_PERMISSIONS
    je .modifySegmentPermissions
    cmp eax, SYSTEM_CALL_SHARED_MEMORY_MAP_SEGMENT
    je .mapSegment
    cmp eax, SYSTEM_CALL_SHARED_MEMORY_UNMAP_SEGMENT
    je .unmapSegment
    cmp eax, SYSTEM_CALL_SHARED_MEMORY_DESTROY_SEGMENT
    je .destroySegment
    cmp eax, SYSTEM_CALL_SHARED_MEMORY_DESTROY_ALL
    je .destroyAll

    jmp .fail

    .initialize:
        call initializeSharedMemory
        jmp .end
    
    .createSegment:
        call addSharedMemorySegment
        jmp .end
    
    .modifySegmentPermissions:
        push esi
        mov esi, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
        mov eax, SHARED_MEMORY_MODIFY_CHANGE_PERMISSIONS
        call modifySharedMemSegment
        pop esi
        jmp .end
    
    .mapSegment:
        mov eax, [esp + 16]
        cmp [eax], dword SYSTEM_CALL_PERMISSION_NONE
        je .mapSegment_noPermission

        .mapSegment_permissionCheck:
            cmp [PROCESS_INFO_ADDRESS + PROCESS_INFO_PERMISSIONS_BASE + PROCESS_PERMISSION_OTHER_PROCESS_MEMORY], dword 1
            je .mapSegment_permissionGranted

            push dword [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
            push dword 0x44
            push dword [eax]
            mov ebp, edx
            mov eax, SYSTEM_CALL_SHARED_MEMORY_MAP_SEGMENT
            call checkCapability
            add esp, 12
            cmp eax, 1
            je .mapSegment_permissionGranted
            jmp .fail

        .mapSegment_noPermission:
        mov edx, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]

        .mapSegment_permissionGranted:

        mov eax, cr3
        push ebx
        push eax
        mov eax, esi
        call enterJustProcess
        cmp eax, PROCESS_ENTER_SUCCESS
        jne .mapSegmentFail

        mov eax, SHARED_MEMORY_MODIFY_ADD_SEGMENT
        call modifySharedMemSegment

        pop ebx
        push eax
        mov eax, esi
        call exitJustProcess
        pop eax
        pop ebx

        jmp .end
        
        .mapSegmentFail:

        pop ebx
        pop ebx
        jmp .fail
    
    .unmapSegment:
        cmp [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS], edx
        je .unmapSegment_permissionGranted
        cmp [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS], esi
        je .unmapSegment_permissionGranted
        jmp .fail
        
        .unmapSegment_permissionGranted:

        mov eax, cr3
        push ebx
        push eax
        mov eax, esi
        call enterJustProcess
        cmp eax, PROCESS_ENTER_SUCCESS
        jne .unmapSegmentFail

        mov eax, SHARED_MEMORY_MODIFY_REMOVE_SEGMENT
        call modifySharedMemSegment

        pop ebx
        mov eax, esi
        call exitJustProcess
        pop ebx

        jmp .end
        
        .unmapSegmentFail:

        pop ebx
        pop ebx
        jmp .end
    
    .destroySegment:
        push esi
        mov esi, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
        mov eax, DELETE_SHARED_MEMORY_SEGMENT
        call deleteSharedMemorySegment
        pop esi
        jmp .end
    
    .destroyAll:
        push esi
        mov esi, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
        call deleteAllSharedMemory
        pop esi
        jmp .end

    .fail:
    xor eax, eax

    .end:
    pop ebp

    jmp systemCallEnd

SYSTEM_CALL_MESSAGING_INITIALIZE equ 0
SYSTEM_CALL_MESSAGING_SET_MESSAGE_HANDLER equ 1
SYSTEM_CALL_MESSAGING_MESSAGE_PROCESS equ 2
SYSTEM_CALL_MESSAGING_MESSAGE_RETURN equ 3
messagingSystemCall:
    push ebp
    mov ebp, esp
    and bp, PROCESSOR_CONTEXT_BASE_BIT_MASK

    cmp eax, SYSTEM_CALL_SHARED_MEMORY_INITIALIZE
    je .initialize
    cmp eax, SYSTEM_CALL_MESSAGING_SET_MESSAGE_HANDLER
    je .setHandler
    cmp eax, SYSTEM_CALL_MESSAGING_MESSAGE_PROCESS
    je .messageProcess
    cmp eax, SYSTEM_CALL_MESSAGING_MESSAGE_RETURN
    je .messageReturn
    jmp .fail

    .initialize:
        call initializeMessaging
        jmp .end
    
    .setHandler:
        call setMessagingHandlerFunction
        jmp .end
    
    .messageProcess:
        mov esi, [esp + 4]
        mov edi, [esp + 16]
        call messageProcess
        jmp .end
    .messageReturn:  
        mov eax, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
        mov ebx, [ebp + PROCESSOR_CONTEXT_CURRENT_THREAD_PAGED_ADDRESS]

        call messageReturn

        push edx
        cmp ebx, dword 0
        mov edx, 0xDEAD0005
        je enterSchedulerNoReschedule
        pop edx

        mov [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS], eax
        mov [ebp + PROCESSOR_CONTEXT_CURRENT_THREAD_PAGED_ADDRESS], ebx

        jmp enterThreadExecution

    .fail:
    xor eax, eax

    .end:

    pop ebp
    jmp systemCallEnd

SYSTEM_CALL_INTERRUPTS_GET_INTERRUPT_FORWARD equ 1
interruptSystemCall:
    push ebp
    mov ebp, esp
    and bp, PROCESSOR_CONTEXT_BASE_BIT_MASK

    cmp eax, SYSTEM_CALL_INTERRUPTS_GET_INTERRUPT_FORWARD
    je .getInterruptForward
    jmp .fail

    .getInterruptForward:           ; ebx = interrupt number, ecx = handler address
        cmp [PROCESS_INFO_ADDRESS + PROCESS_INFO_PERMISSIONS_BASE + PROCESS_PERMISSION_INTERRUPT_FORWARD], dword 1
        jne .fail

        cmp ebx, 256
        jae .fail

        mov eax, [ebp + PROCESSOR_CONTEXT_CURRENT_PROCESS]
        push ebx
        shl ebx, 4
        mov [PROCESS_INTERRUPT_HANDLERS_TABLE + ebx], eax
        mov [PROCESS_INTERRUPT_HANDLERS_TABLE + ebx + 4], ecx
        mov [PROCESS_INTERRUPT_HANDLERS_TABLE + ebx + 8], edx
        pop ebx

        xor eax, eax
        inc eax
        jmp .end

    .fail:
        xor eax, eax
    .end:

    pop ebp
    jmp systemCallEnd

genericInterruptReturn:
systemCallEnd:
    push edx
    mov dl, [esp + 4 + 4]
    and dl, 0xF8
    cmp dl, 0x8
    jne .userModeReturn

    pop edx
    iret

    .userModeReturn:
    pop edx

    push eax
    push ebx
    push ecx
    push edx
    push edi
    push esi
    push ebp

    mov ebp, esp
    and bp, PROCESSOR_CONTEXT_BASE_BIT_MASK
    mov eax, [ebp + PROCESSOR_CONTEXT_CURRENT_THREAD_PAGED_ADDRESS]

    mov ecx, THREAD_STACK_SAVE_SIZE / 4
    mov esi, esp
    lea edi, [eax + THREAD_INTERRUPT_STACK_SAVE]
    rep movsd
    add esp, THREAD_STACK_SAVE_SIZE

    fsave [eax + THREAD_INTERRUPT_FP_SAVE]

    jmp enterThreadExecution