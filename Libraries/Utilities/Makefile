
ARCH := x86

# directories
PROJECT_ROOT := ../..
INC := ./include
PUB_INC := $(PROJECT_ROOT)/PublicHeaders
SRC := .
OBJ := $(PROJECT_ROOT)/Build/$(ARCH)/obj/Libraries/Utilities
EXE := $(PROJECT_ROOT)/Build/$(ARCH)/exe/Libraries

# compiler & linker
CC := $(PROJECT_ROOT)/Utils/$(ARCH)/cc
LD := $(PROJECT_ROOT)/Utils/$(ARCH)/ld
LIB_MAKER := python3 $(PROJECT_ROOT)/Utils/$(ARCH)/LibraryMaker.py
LINKER_SCRIPT_1 := $(PROJECT_ROOT)/Utils/$(ARCH)/LinkerScripts/standardlib.lds
LINKER_SCRIPT_2 := $(PROJECT_ROOT)/Utils/$(ARCH)/LinkerScripts/standard2.lds
CFLAGS := -m32 -march=i486 -Os -fPIE -nostdlib -ffreestanding -fno-asynchronous-unwind-tables -masm=intel -I$(INC) -I$(PUB_INC)
LDFLAGS := -T $(LINKER_SCRIPT_2)


_Deps := Definitions.h Queues.h

_Objs := Queues.o

Deps = $(patsubst %,$(INC)/%,$(_Deps))
LibObjs = $(patsubst %,$(OBJ)/lib/%,$(_Objs))
GlobalObjs = $(patsubst %,$(OBJ)/global/%,$(_Objs))
StaticObjs = $(patsubst %,$(OBJ)/static/%,$(_Objs))
DynamicObjs = $(patsubst %,$(OBJ)/dynamic/%,$(_Objs))

# TODO -- add dynamic when ready
all: static

# build all components for static linking
static : $(OBJ)/static/Utilities.o

# build all components for dynamic linking
dynamic : $(OBJ)/dynamic/Utilities.o $(EXE)/Utilities

# build the dynamically linkable executable file
$(EXE)/Utilities : $(OBJ)/lib/Utilities $(OBJ)/lib/_symbolOffsets.bin
	$(LD) -T $(LINKER_SCRIPT_2) $< -o $@
	cat $(OBJ)/lib/_symbolOffsets.bin $@ > $(EXE)/Utilities_tmp
	mv $(EXE)/Utilities_tmp $@

# create the object file with function stubs for linking the dynamically
# linkable executable
$(OBJ)/dynamic/Utilities.o : $(DynamicObjs) $(OBJ)/global/_DynamicLink.o $(OBJ)/global/Initialization.o
	$(LD) -relocatable $^ -o $@

# create the ELF for the dynamically linked part of file; must 
$(OBJ)/lib/Utilities : $(LibObjs)
	$(CC) -o $@ $^ $(CFLAGS) -Wl,-T $(LINKER_SCRIPT_1)

# create the statically linkable version of the library
$(OBJ)/static/Utilities.o : $(StaticObjs) $(OBJ)/global/_StaticLink.o $(OBJ)/global/Initialization.o
	$(LD) -relocatable $^ -o $@

# build each component of the statically linked object file
$(OBJ)/static/%.o : $(OBJ)/lib/%.o $(OBJ)/global/%.o
	$(LD) -relocatable $^ -o $@

# create the static function stub files for dynamic linking
$(OBJ)/dynamic/%_dyn.o : $(SRC)/dynamic/%.c $(OBJ)/global/%.o
	$(CC) -c -o $@ $< $(CFLAGS)

# build the complete static component for dynamic liking (including the global
# component)
$(OBJ)/dynamic/%.o : $(OBJ)/dynamic/%_dyn.o $(OBJ)/global/%.o
	$(LD) -relocatable $^ -o $@

# generate the source code for the function stubs when dynamically linking
$(SRC)/dynamic/%.c : $(OBJ)/lib/Utilities $(Deps)
	$(LIB_MAKER) $(OBJ)/lib/Utilities -n Utilities -i $(INC) -o $(SRC)/dynamic -t $(OBJ)/lib/_symbolOffsets.bin -s ./.symbolList.txt

# build the symbol offset table for the dynamically linked executable
$(OBJ)/lib/_symbolOffsets.bin : $(OBJ)/lib/Utilities $(Deps)
	$(LIB_MAKER) $(OBJ)/lib/Utilities -n Utilities -i $(INC) -o $(SRC)/dynamic -t $@

# create a component of the core library
$(OBJ)/lib/%.o : $(SRC)/src/%.c $(Deps)
	$(CC) -c -o $@ $< $(CFLAGS)

# create a global component of the library (a piece that is statically linked
# to all programs regardsless of whether the rest of the library is dynamically
# or statically linked)
$(OBJ)/global/%.o : $(SRC)/global/%.c $(Deps)
	$(CC) -c -o $@ $< $(CFLAGS)

initialize:
	mkdir -p $(OBJ)/dynamic $(OBJ)/global $(OBJ)/lib $(OBJ)/static
	mkdir -p $(SRC)/dynamic

clean:
	rm -f $(LibObjs)
	rm -f $(OBJ)/lib/Utilities
	rm -f $(OBJ)/lib/_symbolOffsets.bin
	rm -f $(GlobalObjs)
	rm -f $(OBJ)/global/_DynamicLink.o $(OBJ)/global/_StaticLink.o
	rm -f $(StaticObjs)
	rm -f $(OBJ)/static/Utilities.o
	rm -f $(DynamicObjs)
	rm -f $(OBJ)/dynamic/Utilities.o
	rm -f $(SRC)/dynamic/*.c
	rm -f $(SRC)/dynamic/CStandardLibrary/*.c
	rm -f $(EXE)/Utilities

.PHONY: all static dynamic initialize clean
