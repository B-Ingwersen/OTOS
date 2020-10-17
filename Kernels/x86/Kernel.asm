[bits 32]
dd IDT_GenericHandlerRedirect                           ; 0x0

dd Exceptions_Handler_DIVIDE_BY_ZERO                    ; 0x4
dd Exceptions_Handler_DEBUG
dd Exceptions_Handler_NON_MASKABLE_INTERRUPT
dd Exceptions_Handler_BREAKPOINT
dd Exceptions_Handler_OVERFLOW
dd Exceptions_Handler_BOUND_RANGE_EXCEEDED
dd Exceptions_Handler_INVALID_OPCODE
dd Exceptions_Handler_DEVICE_NOT_AVAILABLE
dd Exceptions_Handler_DOUBLE_FAULT
dd Exceptions_Handler_COPROCESSOR_SEGMENT_OVERRUN
dd Exceptions_Handler_INVALID_TSS
dd Exceptions_Handler_SEGMENT_NOT_PRESENT
dd Exceptions_Handler_STACK_SEGMENT_FAULT
dd Exceptions_Handler_GENERAL_PROTECTION_FAULT
dd Exceptions_Handler_PAGE_FAULT
dd Exceptions_Handler_X87_FPU_EXCPETION
dd Exceptions_Handler_ALIGNMENT_CHECK
dd Exceptions_Handler_MACHINE_CHECK
dd Exceptions_Handler_SIMD_FPU_EXCEPTION
dd Exceptions_Handler_VIRTUALIZATION_EXCEPTION
dd Exceptions_Handler_SECURITY_EXCEPTION

dd Timers_APIC_SendEOI                                  ; 0x58
dd Timers_APIC_Reload
dd Timers_PIC_SendEOI
dd Timers_PIC_Reload

dd timeSliceInterrupt                                   ; 0x68

dd memorySystemCall
dd processThreadSystemCall
dd capabilitiesSystemCall
dd messagingSystemCall
dd sharedMemorySystemCall
dd biosSystemCall
dd biosCallKernelInterrupt
dd interruptSystemCall

dd createProcess
dd mapNewOrKernelMemoryIntoProcess
dd changeProcessPermission
dd createThread
dd enterJustProcess
dd exitJustProcess
dd addThreadToScheduler
dd enterSchedulerNoReschedule

%include "./Definitions.asm"
%include "./KernelInclude/IDT.asm"
%include "./KernelInclude/Exceptions.asm"
%include "./KernelInclude/Synchronization.asm"
%include "./KernelInclude/Memory.asm"
%include "./KernelInclude/Timers.asm"
%include "./KernelInclude/Processes.asm"
%include "./KernelInclude/Threads.asm"
%include "./KernelInclude/Scheduler.asm"
%include "./KernelInclude/SharedMemory.asm"
%include "./KernelInclude/MemoryOperations.asm"
%include "./KernelInclude/Capabilities.asm"
%include "./KernelInclude/Messaging.asm"
%include "./KernelInclude/ExecutionControls.asm"
%include "./KernelInclude/SystemCalls.asm"
%include "./KernelInclude/BiosCalls.asm"