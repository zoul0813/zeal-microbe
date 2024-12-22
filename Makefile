##
# The build variables for ZGDK are all optional.
# Override their value by uncommenting the corresponding line.
##

##
# Project specific
##

# Specify the name of the output binary.
BIN=microbe.bin

# Specify the directory containing the source files.
# INPUT_DIR=src

# Specify the build containing the compiled files.
# OUTPUT_DIR=bin

# Specify the files in the src directory to compile and the name of the final binary.
# By default, all the C files inside `INPUT_DIR` are selected, the `INPUT_DIR` prefix must not be part of the files names.
# SRCS=$(notdir $(wildcard $(INPUT_DIR)/*.c))

# Specify the location of your asserts (aseprite, gif, zts, ztp, ztm, etc)
# ASSETS_DIR=assets

##
# ZOS Specific
##

# Specify the shell to use for sub-commands.
# SHELL = /bin/bash

# Specify the C compiler to use.
# ZOS_CC=sdcc

# Specify the linker to use.
# ZOS_LD=sdldz80

# Specify additional flags to pass to the compiler.
# ZOS_CFLAGS=

# Specify additional flags to pass to the linker.
# ZOS_LDFLAGS=

# Specify the `objcopy` binary that performs the ihex to bin conversion.
# By default it uses `sdobjcopy` or `objcopy` depending on which one is installed.
# OBJCOPY=$(shell which sdobjcopy objcopy | head -1)

##
# ZVB Specific
##

# Specify additional flags to pass to the compiler. This will be concatenated to `ZOS_CFLAGS`.
# ZVB_CFLAGS=-I$(ZVB_SDK_PATH)/include/

# Specify additional flags to pass to the linker. This will be concatenated to `ZOS_LDFLAGS`.
# ZVB_LDFLAGS=-k $(ZVB_SDK_PATH)/lib/ -l zvb_gfx

# ZGDK has sound enabled by default, uncomment this to disable sound and reduce your binary size
# ENABLE_SOUND=0
# ENABLE_CRC32=1

##
# ZGDK Specific
##

# ASEPRITE_PATH ?= ~/.steam/debian-installation/steamapps/common/Aseprite/aseprite
# GIF_SRCS=$(ASEPRITE_SRCS:.aseprite=.gif)
# ZTS_SRCS=$(GIF_SRCS:.gif=.zts)
# ZTM_SRCS=$(TILEMAP_SRCS:.tmx=.ztm)

# Set these #define flags
# EMULATOR=1
# FRAMELOCK=1
# DEBUG=1

##
# User Defined
##

# EXTRA_CFLAGS= -I$(ZGDK_PATH)/include
# EXTRA_LDFLAGS= -k $(ZGDK_PATH)/lib -l zgdk -l zvb_sound

ifdef BREAK
ZOS_CFLAGS += -DBREAK
endif

ifndef ZGDK_PATH
	$(error "Failure: ZGDK_PATH variable not found. It must point to ZGDK path.")
endif

GFX_STRIP = 4

include $(ZGDK_PATH)/base_sdcc.mk

## Add your own rules here

# all::
# 	cp assets/bricked.zmt bin/bricked.zmt