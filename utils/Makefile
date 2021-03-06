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

CFLAGS  += -I../src -I../src/ffmpeg_profiler
LDFLAGS += -L../src -ldlna  -L../src/ffmpeg_profiler -lffmpeg_profiler

ifeq ($(BUILD_STATIC),yes)
  LDFLAGS += $(EXTRALIBS)
endif

all: banner $(PROFILER_BIN) $(DMS_BIN) $(DMR_BIN)

banner:
	@echo 
	@echo "#############################################"
	@echo "#         Building DLNA Utilities           #"
	@echo "#############################################"

$(PROFILER_BIN): $(PROFILER_SRCS)
	$(CC) $? $(OPTFLAGS) $(CFLAGS) $(LDFLAGS) -o $@

$(DMS_BIN): $(DMS_SRCS)
	$(CC) $? $(OPTFLAGS) $(CFLAGS) $(LDFLAGS) -o $@

$(DMR_BIN): $(DMR_SRCS)
	$(CC) $? $(OPTFLAGS) $(CFLAGS) $(LDFLAGS) -o $@

clean:
	-$(RM) -f $(BINS)

distclean: clean

.PHONY: clean distclean

dist-all:
	cp $(EXTRADIST) $(SRCS) Makefile $(DIST)

.PHONY: dist dist-all
