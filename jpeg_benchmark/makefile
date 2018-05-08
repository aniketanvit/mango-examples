# -------------------------------------------------------------
# mango makefile
#
# Copyright (C) 2012-2016 Twilight 3D Finland Oy Ltd.
# -------------------------------------------------------------

# -------------------------------------------------------------
# configuration
# -------------------------------------------------------------

LIBNAME = test

INCLUDE_BASE  = ./
SOURCE_BASE   = ./
SOURCE_DIRS   = ./
OBJECTS_PATH  = objects

# common compiler options (LLVM/CLANG/GCC)
OPTIONS       = -c -Wall -O3 -ffast-math
OPTIONS_GCC   = -ftree-vectorize
#OPTIONS_X86   = -msse4
OPTIONS_X86   = -mavx

# linker options after objects (gcc 4.9 workaround)
LINK_POST     =

#NEON          = -mfpu=neon
NEON          = -mfpu=neon-fp16 -mfp16-format=ieee

OS   = $(shell uname)
ARCH = $(shell uname -m)

# -------------------------------------------------------------
# Linux
# -------------------------------------------------------------

ifeq (Linux, $(OS))

  LIBRARY  = $(LIBNAME)
  CLEAN    = rm -fr $(LIBRARY) $(OBJECTS_PATH)
  LINK_POST += -lmango -lpthread -ldl -lX11 -lGL -lturbojpeg

  # Intel x86 64 bit GCC
  ifeq (x86_64, $(ARCH))
    CC    = gcc $(OPTIONS) $(OPTIONS_GCC) $(OPTIONS_X86) -fPIC
    CPP   = g++ $(OPTIONS) $(OPTIONS_GCC) $(OPTIONS_X86) -fPIC -ansi -std=c++14
    LINK  = g++ -s -o $(LIBRARY)
endif

  # Intel x86 32 bit GCC
  ifeq (i686, $(ARCH))
    CC     = gcc $(OPTIONS) $(OPTIONS_GCC) $(OPTIONS_X86) -g
    CPP    = g++ $(OPTIONS) $(OPTIONS_GCC) $(OPTIONS_X86) -g -ansi -std=c++14
    LINK   = g++ -s -o $(LIBRARY)
  endif

  # ARM GCC (Tested on Raspberry Pi)
  ifeq (armv6l, $(ARCH))
    CC     = gcc $(OPTIONS) $(OPTIONS_GCC) -g -mfpu=vfp -mfloat-abi=hard
    CPP    = g++ $(OPTIONS) $(OPTIONS_GCC) -g -mfpu=vfp -mfloat-abi=hard -ansi -std=c++0x
    LINK   = g++ -s -o $(LIBRARY)
  endif

  # ARM GCC (Tested on ODROID-U3)
  ifeq (armv7l, $(ARCH))
    CC     = gcc $(OPTIONS) $(OPTIONS_GCC) -g -marm -mcpu=cortex-a9 -mtune=cortex-a9 $(NEON) -mfloat-abi=hard -funsafe-math-optimizations -fPIC
    CPP    = g++ $(OPTIONS) $(OPTIONS_GCC) -g -marm -mcpu=cortex-a9 -mtune=cortex-a9 $(NEON) -mfloat-abi=hard -funsafe-math-optimizations -fPIC -ansi -std=c++14 -fpermissive
    LINK   = g++ -s -o $(LIBRARY)
  endif

endif

# -------------------------------------------------------------
# objects
# -------------------------------------------------------------

SOURCES += $(foreach dir,$(SOURCE_DIRS),$(wildcard $(SOURCE_BASE)/$(dir)/*.cpp) $(wildcard $(SOURCE_BASE)/$(dir)/*.c) $(wildcard $(SOURCE_BASE)/$(dir)/*.mm))
OBJECTS += $(addprefix $(OBJECTS_PATH)/,$(patsubst %.cpp,%.o, $(patsubst %.c,%.o, $(patsubst %.mm,%.o, $(abspath $(SOURCES))))))

# -------------------------------------------------------------
# rules
# -------------------------------------------------------------

all: $(LIBRARY)

$(OBJECTS_PATH)/%.o: %.mm
	@echo [Compile OBJC] $<
	@-mkdir -p $(@D)
	@$(CPP) -I$(INCLUDE_BASE) $< -o $@

$(OBJECTS_PATH)/%.o: %.cpp
	@echo [Compile C++] $<
	@-mkdir -p $(@D)
	@$(CPP) -I$(INCLUDE_BASE) $< -o $@

$(OBJECTS_PATH)/%.o: %.c
	@echo [Compile C] $<
	@-mkdir -p $(@D)
	@$(CC) $< -o $@

$(LIBRARY): $(OBJECTS)
	@echo [Link] $@
	@$(LINK) $(OBJECTS) $(LINK_POST)

clean:
	@echo [Clean]
	@$(CLEAN)
