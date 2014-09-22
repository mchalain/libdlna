#ifndef __MPG123APP_PROFILER_H__
#define __MPG123APP_PROFILER_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#ifndef MPG123_H_TRUE
#define MPG123_H_TRUE

#define FALSE 0
#define TRUE  1

#endif

#if defined(WIN32) && defined(DYNAMIC_BUILD)
#define LINK_MPG123_DLL
#endif
#include "mpg123.h"
#define MPG123_REMOTE 
#define REMOTE_BUFFER_SIZE 2048
#define MAXOUTBURST 32768

#ifdef __GNUC__
#define INLINE inline
#else
#define INLINE
#endif

#include "audio.h"

struct parameter
{
	int outmode;	/* where to out the decoded sampels */
	int quiet;	/* shut up! */
	int verbose;    /* verbose level */
	char* output_module;	/* audio output module to use */
	char* output_device;	/* audio output device to use */
	int   output_flags;	/* legacy output destination for AIX/HP/Sun */
	long force_rate;
	char *force_encoding;
	long gain; /* audio output gain, for selected outputs */
};

extern struct parameter param;

#endif
