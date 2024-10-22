#
# SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
#
# SPDX-License-Identifier: CC0-1.0
#

SHELL := /bin/bash

ifeq '$(findstring ;,$(PATH))' ';'
    detected_OS := Windows
else
    detected_OS := $(shell uname 2>/dev/null || echo Unknown)
    detected_OS := $(patsubst CYGWIN%,Cygwin,$(detected_OS))
    detected_OS := $(patsubst MSYS%,MSYS,$(detected_OS))
    detected_OS := $(patsubst MINGW%,MSYS,$(detected_OS))
endif

STAT_BYTES = stat
ifeq ($(detected_OS),Darwin)
	STAT_BYTES += -f %z
# TODO: Support Windows?
else
	STAT_BYTES += -c %s
endif

# Directory where source files are and where the binaries will be put
INPUT_DIR=src
ASSETS_DIR=assets
OUTPUT_DIR=bin

# Specify the files to compile and the name of the final binary
SRCS=$(wildcard $(INPUT_DIR)/*.c)
ASEPRITE_SRCS=$(wildcard $(ASSETS_DIR)/*.aseprite)
TILEMAP_SRCS=$(wildcard $(ASSETS_DIR)/*.tmx)
BIN=microbe.bin

# Include directory containing Zeal 8-bit OS header files.
ifndef ZOS_PATH
$(error "Please define ZOS_PATH environment variable. It must point to Zeal 8-bit OS source code path.")
endif
ifndef ZVB_SDK_PATH
$(error "Please define ZVB_SDK_PATH environment variable. It must point to Zeal Video Board SDK path.")
endif
ZVB_INCLUDE=$(ZVB_SDK_PATH)/include/
ZOS_INCLUDE=$(ZOS_PATH)/kernel_headers/sdcc/include/
ZVB_LIB_PATH=$(ZVB_SDK_PATH)/lib/
ZOS_LIB_PATH=$(ZOS_PATH)/kernel_headers/sdcc/lib
ASEPRITE_PATH ?= ~/.steam/debian-installation/steamapps/common/Aseprite/aseprite
# Regarding the linking process, we will need to specify the path to the crt0 REL file.
# It contains the boot code for C programs as well as all the C functions performing syscalls.
CRT_REL=$(ZOS_PATH)/kernel_headers/sdcc/bin/zos_crt0.rel


# Compiler, linker and flags related variables
CC=sdcc
# Specify Z80 as the target, compile without linking, and place all the code in TEXT section
# (_CODE must be replace).
CFLAGS=-mz80 --opt-code-speed -c --codeseg TEXT -I$(ZOS_INCLUDE) -I$(ZVB_INCLUDE)
ifdef EMULATOR
CFLAGS+=-DEMULATOR
endif
LD=sdldz80
# Make sure the whole program is relocated at 0x4000 as request by Zeal 8-bit OS.
LDFLAGS=-n -mjwx -i -b _HEADER=0x4000 -k $(ZOS_LIB_PATH) -l z80 -k $(ZVB_LIB_PATH) -l zvb_gfx -l zvb_sound
# Binary used to convert ihex to binary
OBJCOPY=objcopy

# Generate the intermediate Intel Hex binary name
BIN_HEX=$(patsubst %.bin,%.ihx,$(BIN))
# Generate the rel names for C source files. Only keep the file names, and add output dir prefix.
SRCS_REL=$(subst $(INPUT_DIR)/,$(OUTPUT_DIR)/,$(patsubst %.c,%.rel,$(SRCS)))
SRCS_ASM_REL=$(subst $(INPUT_DIR)/,$(OUTPUT_DIR)/,$(patsubst %.asm,%.rel,$(SRCS)))
GIF_SRCS=$(ASEPRITE_SRCS:.aseprite=.gif)
ZTS_SRCS=$(GIF_SRCS:.gif=.zts)
ZTM_SRCS=$(TILEMAP_SRCS:.tmx=.ztm)


.PHONY: all clean

all: clean $(GIF_SRCS) $(ZTS_SRCS) $(ZTM_SRCS) $(OUTPUT_DIR) $(OUTPUT_DIR)/$(BIN_HEX) $(OUTPUT_DIR)/$(BIN)
	@bash -c 'echo -e "\x1b[32;1mSuccess, binary generated: $(OUTPUT_DIR)/$(BIN)\x1b[0m"'
	@echo "uartrcv $$($(STAT_BYTES) $(OUTPUT_DIR)/$(BIN)) $(BIN)"


$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

# Generate a REL file for each source file. In fact, SDCC doesn't support compiling multiple source file
# at once. We have to create the same directory structure in output dir too.
$(SRCS_REL): $(OUTPUT_DIR)/%.rel : $(INPUT_DIR)/%.c
	@mkdir -p $(OUTPUT_DIR)/$(dir $*)
	$(CC) $(CFLAGS) -o $(OUTPUT_DIR)/$(dir $*) $<

# Generate the final Intel HEX binary.
$(OUTPUT_DIR)/$(BIN_HEX): $(CRT_REL) $(SRCS_REL)
	$(LD) $(LDFLAGS) $(OUTPUT_DIR)/$(BIN_HEX) $(CRT_REL) $(SRCS_REL)

# Convert the Intel HEX file to an actual binary.
$(OUTPUT_DIR)/$(BIN):
	$(OBJCOPY) --input-target=ihex --output-target=binary $(OUTPUT_DIR)/$(BIN_HEX) $(OUTPUT_DIR)/$(BIN)

%.gif: %.aseprite
	if [ -f $(ASEPRITE_PATH) ]; then $(ASEPRITE_PATH) -b --sheet $@ $<; fi

%.zts: %.gif
	$(ZVB_SDK_PATH)/tools/zeal2gif/gif2zeal.py -i $< -t $@ -p $(patsubst %.zts,%.ztp,$@) -c

%.ztm: %.tmx
	$(ZVB_SDK_PATH)/tools/tiled2zeal/tiled2zeal.py -i $< -m $@

clean:
	rm -fr bin/

cleanall: clean
	rm -rf assets/*.zt[psm]