ARCH:=i686-linux
DEBUG:=1
#UTF8_CHAR:=1
MM_DEBUG:=1
Q:=@

GLOBAL_CFLAGS:=-Wall -O2
GLOBAL_HOST_CFLAGS:=-Wall -O2

CONFIG_NAME:=sophon_config.h
CONFIG_INCLUDE:=ARCH
CONFIG_BOOL:=DEBUG MM_DEBUG UTF8_CHAR
CONFIG_STRING:=
CONFIG_PREFIX:=SOPHON_

SOPHONJS_LIBRARY:=libsophonjs
SOPHONJS_LIBRARY_SRCS:=src/sophon_vm.c\
	src/sophon_mm.c\
	src/sophon_util.c\
	src/sophon_double_pool.c\
	src/sophon_hash.c\
	src/sophon_gc.c\
	src/sophon_string.c\
	src/sophon_conv.c\
	src/sophon_strtoi.c\
	src/sophon_strtod.c\
	src/sophon_char_table_search.c\
	src/sophon_object.c\
	src/sophon_value.c

include build/head.mk

TARGET:=$(SOPHONJS_LIBRARY)
SRCS:=$(SOPHONJS_LIBRARY_SRCS)
include build/dynamic_library.mk

TARGET:=$(SOPHONJS_LIBRARY)
SRCS:=$(SOPHONJS_LIBRARY_SRCS)
include build/static_library.mk

TARGET:=sophonjs
SRCS:=src/sophonjs.c
SLIBS:=libsophonjs
include build/executable.mk

INSTALL_SLIBS:=libsophonjs
INSTALL_DLIBS:=libsophonjs
INSTALL_EXES:=sophonjs
INSTALL_HEADERS:=$(wildcard include/*.h)
INSTALL_HEADERS_PREFIX:=sophon
include build/install.mk

include build/tail.mk
