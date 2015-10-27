TARGET_SUFFIX:=$(TARGET)$(SLIB_SUFFIX)
TARGET_FULL:=$(OUT)/$(TARGET_SUFFIX)

ifneq ($(AUTO_BUILD), 0)
all: $(TARGET_FULL)
endif

OBJS:=$(foreach src,$(SRCS),$(OUT)/$(dir $(src))$(TARGET_SUFFIX)-$(basename $(notdir $(src))).o)

$(foreach src,$(SRCS),$(eval $(call build_obj,$(src),$(TARGET_SUFFIX),$(CFLAGS))))

$(TARGET_FULL): BUILD_LDFLAGS:=$(LDFLAGS)
$(TARGET_FULL): $(OBJS)
	$(Q)$(INFO) AR '->' $@
	$(Q)$(AR) rcs -o $@ $^
	$(Q)$(RANLIB) $@

CLEAN_TARGETS+=$(TARGET_FULL) $(OBJS)
	
include build/clear_env.mk
