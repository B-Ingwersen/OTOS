# directories
PROJECT_ROOT := ../..
SRC := .
EXE := $(PROJECT_ROOT)/Build/x86/exe/Kernels

# compilation tools
ASM := nasm

# make Kernel
_Kernel_Deps := BiosCalls.asm Exceptions.asm IDT.asm MemoryOperations.asm Processes.asm SharedMemory.asm SystemCalls.asm Timers.asm Capabilities.asm ExecutionControls.asm Memory.asm Messaging.asm Scheduler.asm Synchronization.asm Threads.asm
Kernel_Deps := $(patsubst %,$(SRC)/KernelInclude/%,$(_Kernel_Deps))

$(EXE)/Kernel : $(EXE)/InitializeKernel $(SRC)/Kernel.asm $(Kernel_Deps)
	$(ASM) $(SRC)/Kernel.asm -o $@

Kernel: $(EXE)/Kernel

# make InitializeKernel
_InitializeKernel_Deps := FPU.asm IDT.asm InterruptIO.asm Memory.asm Multiprocessor.asm Scheduler.asm SetupManagers.asm Synchronization.asm SystemCalls.asm Timers.asm
InitializeKernel_Deps := $(patsubst %,$(SRC)/Initialization/%,$(_InitializeKernel_Deps))

$(EXE)/InitializeKernel : $(SRC)/InitializeKernel.asm $(InitializeKernel_Deps)
	$(ASM) $(SRC)/InitializeKernel.asm -o $@

InitializeKernel: $(EXE)/InitializeKernel

# make all
all: InitializeKernel Kernel

# make clean
clean:
	rm $(EXE)/InitializeKernel
	rm $(EXE)/Kernel

