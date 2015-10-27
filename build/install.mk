INSTALL_LIBS:=$(foreach lib,$(INSTALL_SLIBS),$(OUT)/$(lib)$(SLIB_SUFFIX))
INSTALL_LIBS+=$(foreach lib,$(INSTALL_DLIBS),$(OUT)/$(lib)$(DLIB_SUFFIX))
INSTALL_EXES:=$(foreach exe,$(INSTALL_EXES),$(OUT)/$(exe)$(EXE_SUFFIX))
ifneq ($(INSTALL_HEADERS),)
INSTALL_HEADERS+=$(CONFIG_FILE)
endif

ifeq ($(INSTALL_DEST),)
INSTALL_DEST:=/usr
endif

INSTALL_LIBS_DEST:=$(INSTALL_DEST)/lib
ifneq ($(INSTALL_LIBS_PREFIX),)
INSTALL_LIBS_DEST:=$(INSTALL_LIBS_DEST)/$(INSTALL_LIBS_PREFIX)
endif

INSTALL_EXES_DEST:=$(INSTALL_DEST)/bin
ifneq ($(INSTALL_EXES_PREFIX),)
INSTALL_EXES_DEST:=$(INSTALL_EXES_DEST)/$(INSTALL_EXES_PREFIX)
endif

INSTALL_HEADERS_DEST:=$(INSTALL_DEST)/include
ifneq ($(INSTALL_HEADERS_PREFIX),)
INSTALL_HEADERS_DEST:=$(INSTALL_HEADERS_DEST)/$(INSTALL_HEADERS_PREFIX)
endif


install_libs: $(INSTALL_LIBS)
	$(Q)for f in $^; do\
		$(INFO) install $$f;\
		$(MKDIR) $(INSTALL_LIBS_DEST);\
		$(INSTALL) -m 755 $$f $(INSTALL_LIBS_DEST);\
	done
ifneq ($(INSTALL_DLIBS),)
	$(Q)$(LDCONFIG) $(INSTALL_LIBS_DEST)
endif

install_exes: $(INSTALL_EXES)
	$(Q)for f in $^; do\
		$(INFO) install $$f;\
		$(MKDIR) $(INSTALL_EXES_DEST);\
		$(INSTALL) -m 755 $$f $(INSTALL_EXES_DEST);\
	done

install_headers: $(INSTALL_HEADERS)
	$(Q)for f in $^; do\
		$(INFO) install $$f;\
		$(MKDIR) $(INSTALL_HEADERS_DEST);\
		$(INSTALL) -m 644 $$f $(INSTALL_HEADERS_DEST);\
	done

install: install_libs install_exes install_headers

uninstall_libs:
	$(Q)for f in $(INSTALL_LIBS); do\
		$(INFO) uninstall $$f;\
		$(RM) $(INSTALL_LIBS_DEST)/$$f;\
	done

uninstall_exes:
	$(Q)for f in $(INSTALL_EXES); do\
		$(INFO) uninstall $$f;\
		$(RM) $(INSTALL_EXES_DEST)/$$f;\
	done

uninstall_headers:
	$(Q)for f in $(INSTALL_HEADERS); do\
		$(INFO) uninstall $$f;\
		$(RM) $(INSTALL_HEADERS_DEST)/$$f;\
	done

uninstall: uninstall_libs uninstall_exes uninstall_headers

.PHONY: install install_libs install_exes install_headers uninstall uninstall_libs uninstall_exes uninstall_headers

