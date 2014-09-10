ifeq (,$(wildcard ../config.mak))
$(error "config.mak is not present, run configure !")
endif
include ../config.mak

PROFILER_BIN  = dlna-profiler
PROFILER_SRCS = dlna-profiler.c

DMS_BIN       = dlna-dms
DMS_SRCS      = dlna-dms.c

DMR_BIN       = dlna-dmr
DMR_SRCS      = dlna-dmr.c

SRCS = \
	$(PROFILER_SRCS) \
	$(DMS_SRCS) \

BINS = \
	$(PROFILER_BIN) \
	$(DMS_BIN) \
	$(DMR_BIN) \

CFLAGS  += -I../src
LDFLAGS += -L../src -ldlna -ldl

FFMPEG_PROFILER_CFLAGS := -I../ffmpeg_profiler
FFMPEG_PROFILER_LDFLAGS := -L../ffmpeg_profiler -lffmpeg_profiler

MPG123_PROFILER_CFLAGS := -I../mpg123_profiler
MPG123_PROFILER_LDFLAGS := -L../mpg123_profiler -lmpg123_profiler

ifeq ($(BUILD_STATIC),yes)
  LDFLAGS += $(EXTRALIBS)
endif

all: banner $(UTILS)

banner:
	@echo 
	@echo "#############################################"
	@echo "#         Building DLNA Utilities           #"
	@echo "#############################################"

$(PROFILER_BIN): CFLAGS+= $(FFMPEG_PROFILER_CFLAGS)
$(PROFILER_BIN): LDFLAGS+= $(FFMPEG_PROFILER_LDFLAGS)
$(PROFILER_BIN): $(PROFILER_SRCS)
	$(CC) $? $(OPTFLAGS) $(CFLAGS) $(LDFLAGS) -o $@

$(DMS_BIN): CFLAGS+= $(MPG123_PROFILER_CFLAGS) $(FFMPEG_PROFILER_CFLAGS)
$(DMS_BIN): LDFLAGS+= $(MPG123_PROFILER_LDFLAGS) $(FFMPEG_PROFILER_LDFLAGS)
$(DMS_BIN): $(DMS_SRCS)
	$(CC) $? $(OPTFLAGS) $(CFLAGS) $(LDFLAGS) -o $@

$(DMR_BIN): CFLAGS+= $(MPG123_PROFILER_CFLAGS)
$(DMR_BIN): LDFLAGS+= $(MPG123_PROFILER_LDFLAGS)
$(DMR_BIN): $(DMR_SRCS)
	$(CC) $? $(OPTFLAGS) $(CFLAGS) $(LDFLAGS) -o $@

clean:
	-$(RM) -f $(BINS)

distclean: clean

.PHONY: clean distclean

dist-all:
	cp $(EXTRADIST) $(SRCS) Makefile $(DIST)

.PHONY: dist dist-all
