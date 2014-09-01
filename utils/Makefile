ifeq (,$(wildcard ../config.mak))
$(error "config.mak is not present, run configure !")
endif
include ../config.mak

PROFILER_BIN  = dlna-profiler
PROFILER_SRCS = dlna-profiler.c

DMS_BIN       = dlna-dms
DMS_SRCS      = dlna-dms.c

SRCS = \
	$(PROFILER_SRCS) \
	$(DMS_SRCS) \

BINS = \
	$(PROFILER_BIN) \
	$(DMS_BIN) \

CFLAGS  += -I../src
LDFLAGS += -L../src -ldlna

FFMPEG_PROFILER_CFLAGS := -I../ffmpeg_profiler
FFMPEG_PROFILER_LDFLAGS := -L../ffmpeg_profiler -lffmpeg_profiler

ifeq ($(BUILD_STATIC),yes)
  LDFLAGS += $(EXTRALIBS)
endif

all: banner $(PROFILER_BIN) $(DMS_BIN)

banner:
	@echo 
	@echo "#############################################"
	@echo "#         Building DLNA Utilities           #"
	@echo "#############################################"

$(PROFILER_BIN): CFLAGS+= $(FFMPEG_PROFILER_CFLAGS)
$(PROFILER_BIN): LDFLAGS+= $(FFMPEG_PROFILER_LDFLAGS)
$(PROFILER_BIN): $(PROFILER_SRCS)
	$(CC) $? $(OPTFLAGS) $(CFLAGS) $(LDFLAGS) -o $@

$(DMS_BIN): CFLAGS+= $(FFMPEG_PROFILER_CFLAGS)
$(DMS_BIN): LDFLAGS+= $(FFMPEG_PROFILER_LDFLAGS)
$(DMS_BIN): $(DMS_SRCS)
	$(CC) $? $(OPTFLAGS) $(CFLAGS) $(LDFLAGS) -o $@

clean:
	-$(RM) -f $(BINS)

distclean: clean

.PHONY: clean distclean

dist-all:
	cp $(EXTRADIST) $(SRCS) Makefile $(DIST)

.PHONY: dist dist-all
