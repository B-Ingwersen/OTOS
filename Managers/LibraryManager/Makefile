
ARCH := x86

# directories
PROJECT_ROOT := ../..
INC := ./include
PUB_INC := $(PROJECT_ROOT)/PublicHeaders
SRC := ./src
EXE := $(PROJECT_ROOT)/Build/$(ARCH)/exe/Managers
OBJ := $(PROJECT_ROOT)/Build/$(ARCH)/obj/Managers/LibraryManager
LIB_OBJ := $(PROJECT_ROOT)/Build/$(ARCH)/obj/Libraries

# compiler/linker
CC := $(PROJECT_ROOT)/Utils/$(ARCH)/cc
LD := $(PROJECT_ROOT)/Utils/$(ARCH)/ld

# linker scripts & compilation flags
LINKER_SCRIPT_1 := $(PROJECT_ROOT)/Utils/$(ARCH)/LinkerScripts/standard1.lds
LINKER_SCRIPT_2 := $(PROJECT_ROOT)/Utils/$(ARCH)/LinkerScripts/standard2.lds
CFLAGS := -m32 -march=i486 -fPIE -shared -nostdlib -ffreestanding -fno-asynchronous-unwind-tables -masm=intel -I$(INC) -I$(PUB_INC)
LDFLAGS := -T $(LINKER_SCRIPT_2)

# make all
TARGET_NAME := LibraryManager

Deps := $(INC)/LibraryManagement.h $(INC)/Messaging.h $(INC)/LibraryManager/MessagingDefinitions.h
Objs := $(OBJ)/LibraryManager.o $(OBJ)/LibraryManagement.o $(OBJ)/Messaging.o
LibObjs := $(LIB_OBJ)/OTOSCore/static/OTOSCore.o

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