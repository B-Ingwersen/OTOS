# directories
PROJECT_ROOT := ../..
SRC := .
EXE := $(PROJECT_ROOT)/Build/x86/Bootloaders

# compilation tools
ASM := nasm

# make MBR_Bootloader
MBR-Bootloader_Deps := $(SRC)/MBR-Bootloader.asm

$(EXE)/MBR-Bootloader.bin: $(MBR-Bootloader_Deps)
	$(ASM) $(SRC)/MBR-Bootloader.asm -o $@

MBR-Bootloader: $(EXE)/MBR-Bootloader.bin

# make FAT32-Bootloader
FAT32-Bootloader_Deps := $(SRC)/FAT32-Bootloader.asm

 $(EXE)/FAT32-Bootloader.bin: $(FAT32-Bootloader_Deps)
	$(ASM) $(SRC)/FAT32-Bootloader.asm -o $@

FAT32-Bootloader: $(EXE)/FAT32-Bootloader.bin

# make all
all: MBR-Bootloader FAT32-Bootloader

# make clean
clean:
	rm $(EXE)/MBR-Bootloader.bin
	rm $(EXE)/FAT32-Bootloader.bin
