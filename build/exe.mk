TARGET_SUFFIX:=$(TARGET)$(EXE_SUFFIX)
TARGET_FULL:=$(OUT)/$(TARGET_SUFFIX)
#TARGET_LIB_FLAGS:=$(foreach lib,$(SLIBS),--static -l$(patsubst lib%,%,$(lib)))
TARGET_LIB_FLAGS+=$(foreach lib,$(DLIBS),-l$(patsubst lib%,%,$(lib)))
#TARGET_DEPS:=$(foreach lib,$(SLIBS),$(OUT)/$(lib)$(SLIB_SUFFIX))
TARGET_DEPS+=$(foreach lib,$(DLIBS),$(OUT)/$(lib)$(DLIB_SUFFIX))

ifneq ($(AUTO_BUILD), 0)
all: $(TARGET_FULL)
endif

OBJS:=$(foreach src,$(SRCS),$(OUT)/$(dir $(src))$(TARGET_SUFFIX)-$(basename $(notdir $(src))).o)
OBJS+=$(foreach lib,$(SLIBS),$(OUT)/$(lib)$(SLIB_SUFFIX))

$(foreach src,$(SRCS),$(eval $(call build_obj,$(src),$(TARGET_SUFFIX),$(CFLAGS))))

$(TARGET_FULL): BUILD_LDFLAGS:=-L$(OUT) $(TARGET_LIB_FLAGS) $(LDFLAGS) $(GLOBAL_LDFLAGS)
$(TARGET_FULL): PWD:=$(shell pwd)
$(TARGET_FULL): $(OBJS)
	$(Q)$(INFO) CC '->' $@
	$(Q)$(CC) -o $@ $^ $(BUILD_LDFLAGS)
ifneq ($(DEBUG),1)
	$(Q)$(STRIP) $@
endif
	$(Q)$(INFO) GEN $@.sh
	$(Q)$(ECHO) '#!/bin/bash' > $@.sh
	$(Q)$(ECHO) "LD_LIBRARY_PATH=$(PWD)/$(OUT) $(PWD)/$@ "'$$@' >> $@.sh
	$(Q)$(CHMOD) 755 $@.sh

ifneq ($(TARGET_DEPS),)
$(OBJS): $(TARGET_DEPS)
endif

CLEAN_TARGETS+=$(TARGET_FULL) $(OBJS)

include build/clear_env.mk
