# directories
PROJECT_ROOT := ../..
SRC := .
EXE := $(PROJECT_ROOT)/Build/x86/Bootloaders

# compilation tools
ASM := nasm

# make all
all: MBR-Bootloader FAT32-Bootloader

# make MBR_Bootloader
MBR-Bootloader_Deps := $(SRC)/MBR-Bootloader.asm

MBR-Bootloader: $(EXE)/MBR-Bootloader.bin

$(EXE)/MBR-Bootloader.bin: $(MBR-Bootloader_Deps)
	$(ASM) $(SRC)/MBR-Bootloader.asm -o $@

# make FAT32-Bootloader
FAT32-Bootloader_Deps := $(SRC)/FAT32-Bootloader.asm

FAT32-Bootloader: $(EXE)/FAT32-Bootloader.bin

 $(EXE)/FAT32-Bootloader.bin: $(FAT32-Bootloader_Deps)
	$(ASM) $(SRC)/FAT32-Bootloader.asm -o $@

# make clean
clean:
	rm $(EXE)/MBR-Bootloader.bin
	rm $(EXE)/FAT32-Bootloader.bin

.PHONY: all MBR-Bootloader FAT32-Bootloader clean
