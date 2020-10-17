
ARCH := x86

# directories
PROJECT_ROOT := ../..
INC := ./include
PUB_INC := $(PROJECT_ROOT)/PublicHeaders
SRC := ./src
EXE := $(PROJECT_ROOT)/Build/$(ARCH)/exe/Applications
OBJ := $(PROJECT_ROOT)/Build/$(ARCH)/obj/Applications/Shell
LIB_OBJ := $(PROJECT_ROOT)/Build/$(ARCH)/obj/Libraries

# compiler/linker
CC := $(PROJECT_ROOT)/Utils/$(ARCH)/cc
LD := $(PROJECT_ROOT)/Utils/$(ARCH)/ld

# linker scripts & compilation flags
LINKER_SCRIPT_1 := $(PROJECT_ROOT)/Utils/$(ARCH)/LinkerScripts/standard1.lds
LINKER_SCRIPT_2 := $(PROJECT_ROOT)/Utils/$(ARCH)/LinkerScripts/standard2.lds
CFLAGS := -m32 -march=i486 -fPIE -shared -nostdlib -ffreestanding -fno-asynchronous-unwind-tables -masm=intel -I$(INC) -I$(PUB_INC)
LDFLAGS := -T $(LINKER_SCRIPT_2)

_Deps = Graphics.h Interaction.h Keyboard.h Output.h PrintFunctions.h Processes.h Screen.h TrueType.h BuiltInOps/Cd.h BuiltInOps/Clear.h BuiltInOps/ListVideoModes.h BuiltInOps/Ls.h BuiltInOps/SetShellColors.h BuiltInOps/SetShellFont.h BuiltInOps/SetVideoMode.h BuiltInOps/TigerLogo.h BuiltInOps/ViewFile.h

_Objs = Shell.o Graphics.o Interaction.o Keyboard.o Output.o PrintFunctions.o Processes.o Screen.o TrueType.o BuiltInOps/Cd.o BuiltInOps/Clear.o BuiltInOps/ListVideoModes.o BuiltInOps/Ls.o BuiltInOps/SetShellColors.o BuiltInOps/SetShellFont.o BuiltInOps/SetVideoMode.o BuiltInOps/TigerLogo.o BuiltInOps/ViewFile.o

Deps := $(patsubst %,$(INC)/%,$(_Deps))
Objs := $(patsubst %,$(OBJ)/%,$(_Objs))
LibObjs := $(LIB_OBJ)/OTOSCore/dynamic/OTOSCore.o $(LIB_OBJ)/Utilities/static/Utilities.o

TARGET_NAME := Shell

all : $(EXE)/$(TARGET_NAME)

$(EXE)/$(TARGET_NAME) : $(OBJ)/$(TARGET_NAME)
	$(LD) $(LDFLAGS) $^ -o $@

$(OBJ)/$(TARGET_NAME) : $(Objs) $(LibObjs)
	$(CC) -o $@ $^ $(CFLAGS) -Wl,-T $(LINKER_SCRIPT_1)

$(OBJ)/%.o : $(SRC)/%.c $(Deps)
	$(CC) -c -o $@ $< $(CFLAGS)

initialize:
	mkdir -p $(OBJ) $(OBJ)/BuiltInOps

# make clean
clean:
	rm -f $(Objs)
	rm -f $(OBJ)/$(TARGET_NAME)
	rm -f $(EXE)/$(TARGET_NAME)