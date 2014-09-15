ifeq (,$(wildcard ../config.mak))
$(error "../config.mak is not present, run configure !")
endif
include ../config.mak

ifeq ($(HAVE_EXTERNAL_LIBUPNP),internal)
  include ixml/ixml.mak
  include threadutil/threadutil.mak
  include upnp/upnp.mak

  CFLAGS += -Iixml
  CFLAGS += -Ithreadutil
  CFLAGS += -Iupnp
endif

LIBNAME = libdlna
LIBNAME_STATIC = ${LIBNAME}.a

ifeq ($(BUILD_STATIC),yes)
  BUILD_RULES   += lib_static
  INSTALL_RULES += install_static
endif
ifeq ($(BUILD_SHARED),yes)
  ifeq ($(BUILD_DYLIB),yes)
    LIBNAME_SHARED  = ${LIBNAME}.dylib
    LIBNAME_VERSION = $(LIBNAME).$(VERSION).dylib
    LIBNAME_MAJOR   = $(LIBNAME).$(shell echo $(VERSION) | cut -f1 -d.).dylib
    LIBNAME_FLAGS   = -dynamiclib -Wl,-headerpad_max_install_names,-undefined,dynamic_lookup,-install_name,$(LIBNAME_VERSION)
  else
    LIBNAME_SHARED  = ${LIBNAME}.so
    LIBNAME_VERSION = $(LIBNAME_SHARED).$(VERSION)
    LIBNAME_MAJOR   = $(LIBNAME_SHARED).$(shell echo $(VERSION) | cut -f1 -d.)
    LIBNAME_FLAGS   = -shared -Wl,-soname,$(LIBNAME_MAJOR)
  endif
  BUILD_RULES   += lib_shared
  INSTALL_RULES += install_shared
endif

SRCS :=  \
	profiles.c \
	dlna.c \
	upnp.c \
	buffer.c \
	didl.c \
	vfs.c \
	crc32.c \
	devices.c \
	services.c \
	cms.c \
	cds.c \
	avts.c \
	msr.c \
	rcs.c \
	http.c \
	upnpav_profiler.c \
	upnp_dms.c \
	upnp_dmr.c \

ifeq ($(HAVE_SQLITE),yes)
SRCS +=  \
  db_sqlite.c
else
SRCS +=  \
  dlna_db.c
endif

EXTRADIST = \
	dlna.h \
	dlna_internals.h \
	upnp_internals.h \
	cms.h \
	cds.h \
	avts.h \
	msr.h \
	rcs.h \

ifeq ($(DEBUG),yes)
CFLAGS += -DHAVE_DEBUG
CFLAGS += -DDEBUG # for ixml library
CFLAGS += -DSTATS # for threadutil library
else
CFLAGS += -DNDEBUG # for ixml library
CFLAGS += -DNO_DEBUG # for threadutil library
endif

CFLAGS += -I.

OBJS = $(SRCS:.c=.o)
LOBJS = $(SRCS:.c=.lo)

.SUFFIXES: .c .o .lo

#DEVELOPER_MODE:=TAGS tags
all: depend $(BUILD_RULES) $(DEVELOPER_MODE)

.c.o:
	$(CC) -c $(CFLAGS) $(OPTFLAGS) -o $@ $<

.c.lo:
	$(CC) -c -fPIC $(CFLAGS) $(OPTFLAGS) -o $@ $<

lib_static_info_pre:
	@echo 
	@echo "#############################################"
	@echo "#         Building static objects           #"
	@echo "#############################################"

lib_static_info_post:
	@echo 
	@echo "#############################################"
	@echo "#         Linking static objects            #"
	@echo "#############################################"

lib_static: lib_static_info_pre $(OBJS) $(IXML_OBJS) $(THREADUTIL_OBJS) $(UPNP_OBJS) lib_static_info_post
	$(AR) r $(LIBNAME_STATIC) $(OBJS) \
	  $(IXML_OBJS) $(THREADUTIL_OBJS) $(UPNP_OBJS)
	$(RANLIB) $(LIBNAME_STATIC)

lib_shared_info_pre:
	@echo 
	@echo "#############################################"
	@echo "#         Building shared objects           #"
	@echo "#############################################"

lib_shared_info_post:
	@echo 
	@echo "#############################################"
	@echo "#         Linking shared objects            #"
	@echo "#############################################"

lib_shared: lib_shared_info_pre $(LOBJS) $(IXML_LOBJS) $(THREADUTIL_LOBJS) $(UPNP_LOBJS) lib_shared_info_post
	$(CC) $(LIBNAME_FLAGS) $(LOBJS) \
	  $(IXML_LOBJS) $(THREADUTIL_LOBJS) $(UPNP_LOBJS) \
	  $(LDFLAGS) $(EXTRALIBS) -o $(LIBNAME_VERSION)
	$(LN) -sf $(LIBNAME_VERSION) $(LIBNAME_MAJOR)
	$(LN) -sf $(LIBNAME_MAJOR) $(LIBNAME_SHARED)

TAGS:
	@rm -f $@; \
	 ( find -name '*.[chS]' -print ) | xargs etags -a

tags:
	@rm -f $@; \
	 ( find -name '*.[chS]' -print ) | xargs ctags -a;

clean:
	-$(RM) -f *.o *.lo *.a *.so*
	-$(RM) -f ixml/*.o ixml/*.lo
	-$(RM) -f threadutil/*.o threadutil/*.lo
	-$(RM) -f upnp/*.o upnp/*.lo
	-$(RM) -f .depend
	-$(RM) -f tags TAGS

install_static: lib_static
	$(INSTALL) -d $(libdir)
	$(INSTALL) $(LIBNAME_STATIC) $(libdir)

install_shared: lib_shared
	$(INSTALL) -d $(libdir)
	$(INSTALL) $(LIBNAME_VERSION) $(libdir)
	$(LN) -sf $(LIBNAME_VERSION) $(libdir)/$(LIBNAME_MAJOR)
	$(LN) -sf $(LIBNAME_MAJOR) $(libdir)/$(LIBNAME_SHARED)

install: $(INSTALL_RULES)
	$(INSTALL) -d $(includedir)
	$(INSTALL) -m 644 dlna.h $(includedir)

depend:
	$(CC) -MM $(CFLAGS) $(OPTFLAGS) $(SRCS) \
	  $(IXML_SRCS) $(THREADUTIL_SRCS) $(UPNP_SRCS) 1>.depend

.PHONY: clean depend

dist-all: ixml-dist-all threadutil-dist-all upnp-dist-all
	cp $(EXTRADIST) $(SRCS) Makefile $(DIST)

.PHONY: dist-all

#
# include dependency files if they exist
#
ifneq ($(wildcard .depend),)
include .depend
endif
