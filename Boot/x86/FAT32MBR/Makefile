# directories
PROJECT_ROOT := ../../..
SRC := .
EXE := $(PROJECT_ROOT)/Build/x86/exe/Boot

# compilation tools
ASM := nasm

FAT32MBR_Deps := $(SRC)/FAT32MBR.asm

$(EXE)/FAT32MBR: $(FAT32MBR_Deps)
	$(ASM) $(SRC)/FAT32MBR.asm -o $@

FAT32MBR: $(EXE)/FAT32MBR

all: FAT32MBR

# make clean
clean:
	rm $(EXE)/FAT32MBR