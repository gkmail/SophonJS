TARGET_SUFFIX:=$(TARGET)$(DLIB_SUFFIX)
TARGET_FULL:=$(OUT)/$(TARGET_SUFFIX)

all: $(TARGET_FULL)

OBJS:=$(foreach src,$(SRCS),$(OUT)/$(dir $(src))$(TARGET_SUFFIX)-$(basename $(notdir $(src))).o)

$(foreach src,$(SRCS),$(eval $(call build_obj,$(src),$(TARGET_SUFFIX),$(CFLAGS) -fPIC)))

$(TARGET_FULL): BUILD_LDFLAGS:=$(LDFLAGS) $(GLOBAL_LDFLAGS)
$(TARGET_FULL): $(OBJS)
	$(Q)$(INFO) CC '->' $@
	$(Q)$(CC) -o $@ --shared $(BUILD_LDFLAGS) $^
	$(Q)$(STRIP) $@

CLEAN_TARGETS+=$(TARGET_FULL) $(OBJS)

include build/clear_env.mk
