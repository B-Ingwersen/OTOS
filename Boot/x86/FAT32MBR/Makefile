# directories
PROJECT_ROOT := ../../..
SRC := .
EXE := $(PROJECT_ROOT)/Build/x86/exe/Boot

# compilation tools
ASM := nasm

FAT32MBR_Deps := $(SRC)/FAT32MBR.asm

all: FAT32MBR

FAT32MBR: $(EXE)/FAT32MBR

$(EXE)/FAT32MBR: $(FAT32MBR_Deps)
	$(ASM) $(SRC)/FAT32MBR.asm -o $@

# make clean
clean:
	rm $(EXE)/FAT32MBR

.PHONY: all FAT32MBR clean