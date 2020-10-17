
ARCH := x86

# directories
PROJECT_ROOT := ../..
INC := ./include
PUB_INC := $(PROJECT_ROOT)/PublicHeaders
SRC := ./src
EXE := $(PROJECT_ROOT)/Build/$(ARCH)/exe/Managers
OBJ := $(PROJECT_ROOT)/Build/$(ARCH)/obj/Managers/KeyboardManager
LIB_OBJ := $(PROJECT_ROOT)/Build/$(ARCH)/obj/Libraries

# compiler/linker
CC := $(PROJECT_ROOT)/Utils/$(ARCH)/cc
LD := $(PROJECT_ROOT)/Utils/$(ARCH)/ld

# linker scripts & compilation flags
LINKER_SCRIPT_1 := $(PROJECT_ROOT)/Utils/$(ARCH)/LinkerScripts/standard1.lds
LINKER_SCRIPT_2 := $(PROJECT_ROOT)/Utils/$(ARCH)/LinkerScripts/standard2.lds
CFLAGS := -m32 -march=i486 -Os -fPIE -shared -nostdlib -ffreestanding -fno-asynchronous-unwind-tables -masm=intel -I$(INC) -I$(PUB_INC)
LDFLAGS := -T $(LINKER_SCRIPT_2)

TARGET_NAME := KeyboardManager

Deps := $(INC)/IPC.h $(INC)/Messaging.h $(INC)/PS2.h $(INC)/Translation.h $(INC)/Debugging.h $(INC)/InterruptHandler.h $(PUB_INC)/KeyboardManager/MessagingDefinitions.h
Objs := $(OBJ)/KeyboardManager.o $(OBJ)/IPC.o $(OBJ)/Messaging.o $(OBJ)/PS2.o $(OBJ)/Translation.o $(OBJ)/InterruptHandler.o $(OBJ)/Debugging.o
LibObjs := $(LIB_OBJ)/OTOSCore/dynamic/OTOSCore.o $(LIB_OBJ)/Utilities/static/Utilities.o

all : $(EXE)/$(TARGET_NAME)

$(EXE)/$(TARGET_NAME) : $(OBJ)/$(TARGET_NAME)
	$(LD) $(LDFLAGS) $^ -o $@

$(OBJ)/$(TARGET_NAME) : $(Objs) $(LibObjs)
	$(CC) -o $@ $^ $(CFLAGS) -Wl,-T $(LINKER_SCRIPT_1)

$(OBJ)/%.o : $(SRC)/%.c $(Deps)
	$(CC) -c -o $@ $< $(CFLAGS)

initialize:
	mkdir -p $(OBJ)

# make clean
clean:
	rm -f $(Objs)
	rm -f $(OBJ)/$(TARGET_NAME)
	rm -f $(EXE)/$(TARGET_NAME)

.PHONY: all initialize clean