
x86-buildAll: x86-bootloaders x86-all x86-diskImage-burnBootloaders x86-diskImage-copyFiles

x86-all: x86-boot-fat32mbr x86-bootloaders x86-kernel x86-library-OTOSCore x86-library-Utilities x86-manager-ProcessManager x86-manager-DiskManager x86-manager-LibraryManager x86-manager-ScreenManager x86-manager-KeyboardManager x86-application-Shell

x86-bootloaders:
	$(MAKE) -C Bootloaders/x86 all

x86-boot-fat32mbr:
	$(MAKE) -C Boot/x86/FAT32MBR all

x86-kernel:
	$(MAKE) -C Kernels/x86 all

x86-library-OTOSCore:
	$(MAKE) -C Libraries/OTOSCore all

x86-library-Utilities:
	$(MAKE) -C Libraries/Utilities all

x86-manager-ProcessManager:
	$(MAKE) -C Managers/ProcessManager all

x86-manager-DiskManager:
	$(MAKE) -C Managers/DiskManager all

x86-manager-LibraryManager:
	$(MAKE) -C Managers/LibraryManager all

x86-manager-ScreenManager:
	$(MAKE) -C Managers/ScreenManager all

x86-manager-KeyboardManager:
	$(MAKE) -C Managers/KeyboardManager all

x86-application-Shell:
	$(MAKE) -C Applications/Shell all

x86-diskImage-create:
	sudo ./Utils/x86/DiskImages/createFAT32.sh Build/x86/testImage.img

x86-diskImage-burnBootloaders:
	sudo ./Utils/x86/DiskImages/burnBootloaders.sh Build/x86/testImage.img Build/x86/Bootloaders/MBR-Bootloader.bin Build/x86/Bootloaders/FAT32-Bootloader.bin

x86-diskImage-copyFiles:
	sudo ./Utils/x86/DiskImages/writeFiles.sh Build/x86/testImage.img Build/x86/exe Build/x86/Mount

x86-diskImage-mount:
	sudo ./Utils/x86/DiskImages/mount.sh Build/x86/testImage.img Build/x86/Mount

x86-diskImage-remove:
	rm Build/x86/testImage.img

_X86_OBJ_DIRS := Applications Managers Libraries
_X86_EXE_DIRS := Applications Managers Libraries Kernels Boot
X86_OBJ_DIRS := $(patsubst %, Build/x86/obj/%,$(_X86_OBJ_DIRS))
X86_EXE_DIRS := $(patsubst %, Build/x86/exe/%,$(_X86_EXE_DIRS))

x86-initializeAll: x86-initialize-directories x86-diskImage-create x86-initialize-cross

x86-initialize-directories:
	mkdir -p Build/x86/Bootloaders Build/x86/Mount $(X86_OBJ_DIRS) $(X86_EXE_DIRS)
	$(MAKE) -C Libraries/OTOSCore initialize
	$(MAKE) -C Libraries/Utilities initialize
	$(MAKE) -C Managers/ProcessManager initialize
	$(MAKE) -C Managers/DiskManager initialize
	$(MAKE) -C Managers/LibraryManager initialize
	$(MAKE) -C Managers/ScreenManager initialize
	$(MAKE) -C Managers/KeyboardManager initialize
	$(MAKE) -C Applications/Shell initialize
	cp -rf DataFiles/* Build/x86/exe/

x86-initialize-cross:
	mkdir Utils/x86/cross
	./Utils/x86/buildCross.sh Utils/x86/cross

x86-cleanAll:
	$(MAKE) -C Bootloaders/x86 clean
	$(MAKE) -C Boot/x86/FAT32MBR clean
	$(MAKE) -C Kernels/x86 clean
	$(MAKE) -C Libraries/OTOSCore clean
	$(MAKE) -C Libraries/Utilities clean
	$(MAKE) -C Managers/ProcessManager clean
	$(MAKE) -C Managers/DiskManager clean
	$(MAKE) -C Managers/LibraryManager clean
	$(MAKE) -C Managers/ScreenManager clean
	$(MAKE) -C Managers/KeyboardManager clean
	$(MAKE) -C Applications/Shell clean

x86-test:
	qemu-system-x86_64 Build/x86/testImage.img -monitor stdio -smp 4