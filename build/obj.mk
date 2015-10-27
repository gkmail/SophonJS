define build_obj
obj=$$(OUT)/$$(dir $(1))$(2)-$$(basename $$(notdir $(1))).o
dep:=$$(OUT)/$$(dir $(1))$(2)-$$(basename $$(notdir $(1))).dep
-include $$(dep)
$$(obj): $$(CONFIG_FILE)
$$(obj): depfile:=$$(dep)
$$(obj): $(1)
	$(Q)$(INFO) CC $$< '->' $$@
	$$(Q)$$(MKDIR) $$(dir $$@)
	$$(Q)$$(CC) $$(GLOBAL_CFLAGS) $(3) -c -MMD -MF $$(depfile) -o $$@ $$<
endef

define build_host_obj
obj=$$(HOST_OUT)/$$(dir $(1))$(2)-$$(basename $$(notdir $(1))).o
dep:=$$(HOST_OUT)/$$(dir $(1))$(2)-$$(basename $$(notdir $(1))).dep
-include $$(dep)
$$(obj): depfile:=$$(dep)
$$(obj): $(1)
	$(Q)$(INFO) HOST CC $$< '->' $$@
	$$(Q)$$(MKDIR) $$(dir $$@)
	$$(Q)$$(HOST_CC) $$(GLOBAL_HOST_CFLAGS) $(3) -c -MMD -MF $$(depfile) -o $$@ $$<
endef
