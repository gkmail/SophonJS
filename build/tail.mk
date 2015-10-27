CONFIG_INCLUDE_ENTRIES:=$(foreach i,$(CONFIG_INCLUDE),$($(i)))
CONFIG_NUMBER_ENTRIES:=$(foreach b,$(CONFIG_NUMBER),$(if $($(b)),$(CONFIG_PREFIX)$(b)=$($(b))))
CONFIG_STRING_ENTRIES:=$(foreach s,$(CONFIG_STRING),$(if $($(s)),$(CONFIG_PREFIX)$(s)='"'$($(s))'"'))

$(CONFIG_FILE): Makefile config.mk
	$(Q)$(INFO) GEN $@
	$(Q)$(MKDIR) $(dir $@)
	$(Q)$(RM) $@
	$(Q)for i in $(CONFIG_INCLUDE_ENTRIES); do\
		echo '#include "'$$i.h'"' >> $@;\
	done
	$(Q)for i in $(CONFIG_NUMBER_ENTRIES); do\
		echo '#define '$$i | sed s/=/\ / >> $@;\
	done
	$(Q)for i in $(CONFIG_STRING_ENTRIES); do\
		echo '#define '$$i | sed s/=/\ / >> $@;\
	done

clean:
	$(Q)$(INFO) clean files
	$(Q)$(RM) $(CLEAN_TARGETS)

clean-all:
	$(Q)$(INFO) clean all files
	$(Q)$(RM) out

.PHONY: clean clean-all

