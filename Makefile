ifeq (,$(wildcard config.mak))
$(error "config.mak is not present, run configure !")
endif
include config.mak

DISTFILE = libdlna-$(VERSION).tar.bz2
PKGCONFIG_DIR = $(libdir)/pkgconfig
PKGCONFIG_FILE = libdlna.pc

#	mpg123_profiler/libmpg123_profiler.so \
PROFILERS= \
	ffmpeg_profiler/libffmpeg_profiler.so

EXTRADIST = \
	AUTHORS \
	ChangeLog \
	configure \
	COPYING \
	README \

SUBDIRS = \
	src \
	utils \

all: lib $(PROFILERS) utils

lib:
	$(MAKE) -C src

.PHONY: $(PROFILERS)
$(PROFILERS):
	make -C $(dir $@)

utils: lib
	$(MAKE) -C utils

clean:
	$(MAKE) -C src clean
	$(foreach profiler,$(PROFILERS),$(MAKE) -C $(dir $(profiler)) clean;)
	$(MAKE) -C utils clean
	-$(RM) -f IUpnpErrFile.txt IUpnpInfoFile.txt

distclean: clean
	-$(RM) -f config.log
	-$(RM) -f config.mak
	-$(RM) -f $(PKGCONFIG_FILE)

install: install-pkgconfig
	$(MAKE) -C src install

install-pkgconfig: $(PKGCONFIG_FILE)
	$(INSTALL) -d "$(PKGCONFIG_DIR)"
	$(INSTALL) -m 644 $< "$(PKGCONFIG_DIR)"

.PHONY: clean distclean
.PHONY: install install-pkgconfig

dist:
	-$(RM) $(DISTFILE)
	dist=$(shell pwd)/libdlna-$(VERSION) && \
	for subdir in . $(SUBDIRS); do \
		mkdir -p "$$dist/$$subdir"; \
		$(MAKE) -C $$subdir dist-all DIST="$$dist/$$subdir"; \
	done && \
	tar cjf $(DISTFILE) libdlna-$(VERSION)
	-$(RM) -rf libdlna-$(VERSION)

dist-all:
	cp $(EXTRADIST) Makefile $(DIST)

.PHONY: dist dist-all utils
