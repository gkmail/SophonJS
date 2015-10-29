include config.mk

Q:=@

TOOL_DEPS:=0
INSFILE_VERSION:=1
VERSION:=0.0.1

GLOBAL_CFLAGS:=-Wall -O2
GLOBAL_HOST_CFLAGS:=-Wall -O2
GLOBAL_LDFLAGS:=-lm

CONFIG_NAME:=sophon_config.h
CONFIG_INCLUDE:=ARCH

CONFIG_NUMBER:=DEBUG\
	MM_DEBUG\
	8BITS_CHAR\
	LINE_INFO\
	MATH DATE URI\
	JSON\
	REGEXP\
	CONSOLE\
	ESCAPE_FUNC\
	UNESCAPE_FUNC\
	STRING_SUBSTR_FUNC\
	DATE_YEAR_FUNC\
	DATE_TO_GMT_STRING_FUNC\
	EVAL_FUNC\
	FUNCTION_CONSTRUCTOR\
	PARSER_STACK_SIZE\
	PARSER_OBJECT_STACK_SIZE\
	PARSER_FUNC_STACK_SIZE\
	PARSER_FRAME_STACK_SIZE\
	ARRAY_EXPAND_SIZE\
	LEX_INPUT_BUF_SIZE\
	JSON_PARSER_STACK_SIZE\
	GC_START_SIZE\
	BLOCK_STACK_SIZE\
	FRAME_STACK_SIZE\
	REGEXP_BRANCH_COUNT\
	INSFILE_VERSION

CONFIG_STRING:=VERSION

CONFIG_PREFIX:=SOPHON_

SOPHONJS_LIBRARY:=libsophonjs
SOPHONJS_LIBRARY_SRCS:=src/sophon_vm.c\
	src/sophon_mm.c\
	src/sophon_util.c\
	src/sophon_number_pool.c\
	src/sophon_hash.c\
	src/sophon_gc.c\
	src/sophon_string.c\
	src/sophon_conv.c\
	src/sophon_strtod.c\
	src/sophon_char_table_search.c\
	src/sophon_object.c\
	src/sophon_value.c\
	src/sophon_frame.c\
	src/sophon_stack.c\
	src/sophon_closure.c\
	src/sophon_module.c\
	src/sophon_lex.c\
	src/sophon_parser.c\
	src/sophon_function.c\
	src/sophon_ins.c\
	src/sophon_array.c\
	src/sophon_decl.c\
	src/sophon_global_object.c\
	src/sophon_dtostr.c\
	src/sophon_date.c\
	src/sophon_qsort.c\
	src/sophon_arguments.c\
	src/sophon_json.c\
	src/sophon_regexp.c\
	src/sophon_insfile.c

include build/head.mk

ifeq ($(BUILD_DLIB), 1)
TARGET:=$(SOPHONJS_LIBRARY)
SRCS:=$(SOPHONJS_LIBRARY_SRCS)
include build/dlib.mk
INSTALL_DLIBS:=libsophonjs
endif

TARGET:=$(SOPHONJS_LIBRARY)
SRCS:=$(SOPHONJS_LIBRARY_SRCS)
include build/slib.mk
INSTALL_SLIBS:=libsophonjs

ifeq ($(BUILD_EXE), 1)
TARGET:=sophonjs
SRCS:=src/sophonjs.c
SLIBS:=libsophonjs
include build/exe.mk
INSTALL_EXES:=sophonjs
endif

TARGET:=js_lex_gen
SRCS:=tools/js_lex_gen.c
LDFLAGS:=-ltea
AUTO_BUILD:=0
include build/host_exe.mk

TARGET:=js_parser_gen
SRCS:=tools/js_parser_gen.c
LDFLAGS:=-ltea
AUTO_BUILD:=0
include build/host_exe.mk

TARGET:=json_parser_gen
SRCS:=tools/json_parser_gen.c
LDFLAGS:=-ltea
AUTO_BUILD:=0
include build/host_exe.mk

TARGET:=uc_table_gen
SRCS:=tools/uc_table_gen.c
AUTO_BUILD:=0
include build/host_exe.mk

ifeq ($(BUILD_TOOLS),1)
TOOL_DEPS:=1
endif

ifeq ($(TOOL_DEPS),1)
src/sophon_js_lex.c: $(HOST_OUT)/js_lex_gen$(HOST_EXE_SUFFIX)
src/sophon_js_parser.h: $(HOST_OUT)/js_parser_gen$(HOST_EXE_SUFFIX)
src/sophon_js_parser.c: $(HOST_OUT)/js_parser_gen$(HOST_EXE_SUFFIX)
src/sophon_json_parser.c: $(HOST_OUT)/json_parser_gen$(HOST_EXE_SUFFIX)
src/uc_id_start_table.c: $(HOST_OUT)/uc_table_gen$(HOST_EXE_SUFFIX)
src/uc_id_cont_table.c: $(HOST_OUT)/uc_table_gen$(HOST_EXE_SUFFIX)
else
TOOL_TARGETS:=src/sophon_js_lex.c src/sophon_js_parser.h src/sophon_js_parser.c src/sophon_json_parser.c src/uc_id_start_table.c src/uc_id_cont_table.c
tools:
	make BUILD_TOOLS=1 $(TOOL_TARGETS)

.PHONY: tools
endif

src/sophon_js_lex.c:
	$(Q)$(INFO) GEN $@
	$(Q)$(HOST_OUT)/js_lex_gen$(HOST_EXE_SUFFIX) >$@ 2>/dev/null

src/sophon_js_parser.h:
	$(Q)$(INFO) GEN $@
	$(Q)$(HOST_OUT)/js_parser_gen$(HOST_EXE_SUFFIX) h >$@ 2>/dev/null

src/sophon_js_parser.c:
	$(Q)$(INFO) GEN $@
	$(Q)$(HOST_OUT)/js_parser_gen$(HOST_EXE_SUFFIX) >$@ 2>/dev/null

src/sophon_json_parser.c:
	$(Q)$(INFO) GEN $@
	$(Q)$(HOST_OUT)/json_parser_gen$(HOST_EXE_SUFFIX) >$@ 2>/dev/null

src/uc_id_start_table.c:
	$(Q)$(INFO) GEN $@
	$(Q)$(HOST_OUT)/uc_table_gen$(HOST_EXE_SUFFIX) uc_id_start Ll Lu Lt Lm Lo Nl >$@ 2>/dev/null

src/uc_id_cont_table.c:
	$(Q)$(INFO) GEN $@
	$(Q)$(HOST_OUT)/uc_table_gen$(HOST_EXE_SUFFIX) uc_id_cont Ll Lu Lt Lm Lo Nl Mn Nd Pc >$@ 2>/dev/null

src/sophon_lex.c: src/sophon_js_lex.c src/sophon_js_parser.h src/uc_id_start_table.c src/uc_id_cont_table.c
src/sophon_parser.c: src/sophon_js_parser.h src/sophon_js_parser.c
src/sophon_json.c: src/sophon_json_parser.c
src/sophon_gc.c: src/sophon_js_parser.h

TARGET:=integrate
SRCS:=examples/integrate.c
SLIBS:=libsophonjs
AUTO_BUILD:=0
include build/exe.mk

TARGET:=extend
SRCS:=examples/extend.c
SLIBS:=libsophonjs
AUTO_BUILD:=0
include build/exe.mk

TARGET:=shell
SRCS:=examples/shell.c
SLIBS:=libsophonjs
AUTO_BUILD:=0
include build/exe.mk

TARGET:=insfile
SRCS:=examples/insfile.c
SLIBS:=libsophonjs
AUTO_BUILD:=0
include build/exe.mk

examples: $(OUT)/integrate$(EXE_SUFFIX) $(OUT)/extend$(EXE_SUFFIX) $(OUT)/shell$(EXE_SUFFIX) $(OUT)/insfile$(EXE_SUFFIX) 

.PHONY: examples

TARGET:=test262
SRCS:=tests/test262.c
SLIBS:=libsophonjs
AUTO_BUILD:=0
include build/exe.mk

test262: $(OUT)/test262$(EXE_SUFFIX)
.PHONY: test262

INSTALL_HEADERS:=$(wildcard include/*.h)
INSTALL_HEADERS_PREFIX:=sophon
include build/install.mk

include build/tail.mk
