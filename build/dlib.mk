TARGET_SUFFIX:=$(TARGET)$(DLIB_SUFFIX)
TARGET_FULL:=$(OUT)/$(TARGET_SUFFIX)

ifneq ($(AUTO_BUILD), 0)
all: $(TARGET_FULL)
endif

OBJS:=$(foreach src,$(SRCS),$(OUT)/$(dir $(src))$(TARGET_SUFFIX)-$(basename $(notdir $(src))).o)

$(foreach src,$(SRCS),$(eval $(call build_obj,$(src),$(TARGET_SUFFIX),$(CFLAGS) -fPIC)))

$(TARGET_FULL): BUILD_LDFLAGS:=$(LDFLAGS) $(GLOBAL_LDFLAGS)
$(TARGET_FULL): $(OBJS)
	$(Q)$(INFO) CC '->' $@
	$(Q)$(CC) -o $@ --shared $(BUILD_LDFLAGS) $^
ifneq ($(DEBUG),1)
	$(Q)$(STRIP) $@
endif

CLEAN_TARGETS+=$(TARGET_FULL) $(OBJS)

include build/clear_env.mk
