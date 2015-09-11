include build/arch/$(ARCH).mk

OUT:=out/$(ARCH)
HOST_OUT:=out/host

HOST_CC:=gcc
HOST_AR:=ar
HOST_STRIP:=strip
HOST_RANLIB:=ranlib

CC:=$(CROSS_COMPILE)gcc
AR:=$(CROSS_COMPILE)ar
STRIP:=$(CROSS_COMPILE)strip
RANLIB:=$(CROSS_COMPILE)ranlib

ECHO:=echo
INFO:=echo '  '
MKDIR:=mkdir -p
RM:=rm -rf
TOUCH:=touch
INSTALL:=install

GLOBAL_CFLAGS+=-Iinclude -I$(OUT)/include

ifeq ($(DEBUG),1)
GLOBAL_CFLAGS+=-g
GLOBAL_HOST_CFLAGS+=-g
endif

CLEAN_TARGETS:=

ifeq ($(CONFIG_NAME),)
CONFIG_NAME:=config.h
endif

CONFIG_FILE:=$(OUT)/include/$(CONFIG_NAME)

all:

.PHONY: all

include build/object.mk
