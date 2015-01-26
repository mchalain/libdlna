/*
 * Copyright (C) 2014-2016 Marc Chalain <marc.chalain@gmail.com>
 *
 * This file is part of uplaymusic.
 *
 * libdlna is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * libdlna is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with libdlna; if not, write to the Free Software
 * Foundation, Inc, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <mpg123.h>

#include "dlna_internals.h"

#include "mpg123app.h"

//#define dbgprintf(...)
#define dbgprintf printf

static dlna_properties_t *item_get_properties (dlna_item_t *item);
static dlna_metadata_t *item_get_metadata (dlna_item_t *item);
static int item_prepare_stream (dlna_item_t *item);
static int item_read_stream (dlna_item_t *item);
static void item_close_stream (dlna_item_t *item);

typedef struct mpg123_profile_s mpg123_profile_t;
const dlna_profile_t *default_media_profiles[] = {
  & (dlna_profile_t) {
    .id = "MP3",
    .ext = ".mp3",
    .mime = MIME_AUDIO_MPEG,
    .label = LABEL_AUDIO_2CH,
    .media_class = DLNA_CLASS_AUDIO,
  },
  & (dlna_profile_t) {
    .id = "MP3X",
    .ext = ".mpa",
    .mime = MIME_AUDIO_MPEG,
    .label = LABEL_AUDIO_2CH,
    .media_class = DLNA_CLASS_AUDIO,
  },
  & (dlna_profile_t) {
    .id = "MP3",
    .ext = ".mp3",
    .mime = MIME_AUDIO_MPEG,
    .label = LABEL_AUDIO_2CH,
    .media_class = DLNA_CLASS_RADIO,
  },
  & (dlna_profile_t) {
    .id = "MP3X",
    .ext = ".mpa",
    .mime = MIME_AUDIO_MPEG,
    .label = LABEL_AUDIO_2CH,
    .media_class = DLNA_CLASS_RADIO,
  },
  NULL
};

typedef struct mpg123_profile_s mpg123_profile_t;
const struct mpg123_profile_s
{
  int profileid;
  enum mpg123_version version;
  int layer;
  enum mpg123_channelcount channels;
} *default_profiles[] = {
  &(const mpg123_profile_t) {
    .profileid = 0,
    .version = MPG123_1_0,
    .layer = 3,
    .channels = MPG123_STEREO,
  },
  &(const mpg123_profile_t) {
    .profileid = 1,
    .version = MPG123_2_0,
    .layer = 3,
    .channels = MPG123_STEREO,
  },
  &(const mpg123_profile_t) {
    .profileid = 1,
    .version = MPG123_2_5,
    .layer = 3,
    .channels = MPG123_STEREO,
  },
  &(const mpg123_profile_t) {
    .profileid = 2,
    .version = MPG123_1_0,
    .layer = 3,
    .channels = MPG123_STEREO,
  },
  &(const mpg123_profile_t) {
    .profileid = 3,
    .version = MPG123_2_0,
    .layer = 3,
    .channels = MPG123_STEREO,
  },
  &(const mpg123_profile_t) {
    .profileid = 3,
    .version = MPG123_2_5,
    .layer = 3,
    .channels = MPG123_STEREO,
  },
  NULL
};

typedef struct profile_data_s profile_data_t;
struct profile_data_s
{
  const mpg123_profile_t *profiler;
  dlna_properties_t *prop;
  dlna_metadata_t *meta;
  int fd;
  ssize_t buffsize;
  void *buffer;
  uint32_t offset;
  uint32_t length;
};

static mpg123_handle *g_profiler_handle = NULL;
audio_output_t *g_ao = NULL;

extern mpg123_module_t mpg123_output_module_info;

audio_output_t * 
init_audio_output(mpg123_module_t *module)
{
	audio_output_t* ao = malloc( sizeof( audio_output_t ) );
	if (ao==NULL)
    return NULL;

	/* Initialise variables */
	ao->fn = -1;
	ao->rate = -1;
	ao->gain = 0;
	ao->userptr = NULL;
	ao->device = NULL;
	ao->channels = -1;
	ao->format = -1;
	ao->flags = 0;
	ao->auxflags = 0;
	ao->is_open = FALSE;

	ao->module = module;

	/* Set the callbacks to NULL */
	ao->open = NULL;
	ao->get_formats = NULL;
	ao->write = NULL;
	ao->flush = NULL;
	ao->close = NULL;
	ao->deinit = NULL;

  ao->module->init_output (ao);
	return ao;
}

void
deinit_audio_output (audio_output_t * ao)
{
  if(ao->is_open && ao->close != NULL) ao->close(ao);
  if (ao->deinit) ao->deinit( ao );
  free (ao);
}

int
open_audio_output (audio_output_t * ao, int rate, int channels, long encoding)
{
  ao->rate = rate;
  ao->channels = channels;
  ao->format = encoding;
  ao->is_open = !ao->open(ao);
  return ao->is_open;
}
int
write_audio_output (audio_output_t * ao, unsigned char *bytes, int count)
{
  int sum = 0;
  int written;
  do
  {
    written = ao->write(ao, bytes, count);
    if(written >= 0)
    {
      sum+=written;
      count -= written; 
    }
    else
      return written;
  }	while(count>0 && written>=0);
  return sum;
}

void
close_audio_output (audio_output_t * ao)
{
  if(ao->is_open)
  {
    ao->is_open = FALSE;
    if(ao->close != NULL) ao->close(ao);
  }
}

static void
profile_free(dlna_item_t *item)
{
  profile_data_t *cookie = (profile_data_t *)item->profile_cookie;

  free (cookie);
  item->profile_cookie = NULL;
}

static dlna_properties_t *
item_get_properties (dlna_item_t *item)
{
  profile_data_t *cookie = (profile_data_t *)item->profile_cookie;
  return cookie->prop;
}

static dlna_metadata_t *
item_get_metadata (dlna_item_t *item)
{
  profile_data_t *cookie = (profile_data_t *)item->profile_cookie;
  return cookie->meta;
}

static int
item_prepare_stream (dlna_item_t *item)
{
  profile_data_t *cookie = (profile_data_t *)item->profile_cookie;
  int  channels = 2, encoding = MPG123_ENC_SIGNED_16;
  long rate = 44100;

  mpg123_replace_reader_handle (g_profiler_handle, item->stream->read, item->stream->lseek, item->stream->cleanup);

  if (mpg123_open_handle (g_profiler_handle, item->stream))
  {
    dbgprintf ("%s: %s\n", __FUNCTION__, mpg123_strerror (g_profiler_handle));
    return -1;
  }

  mpg123_format_none(g_profiler_handle);
  if (item->properties)
  {
    rate = item->properties->sample_frequency;
    channels = item->properties->channels;
  }
  mpg123_format(g_profiler_handle, rate, channels, encoding);

  if (mpg123_getformat(g_profiler_handle, &rate, &channels, &encoding) != MPG123_OK)
  {
    dbgprintf ("%s: %s\n", __FUNCTION__, mpg123_strerror (g_profiler_handle));
    mpg123_close(g_profiler_handle);
    return -1;
  }

  dbgprintf ("rate %ld channels %d format 0x%X\n", rate, channels, encoding);
  if( !g_ao)
    g_ao = init_audio_output (&mpg123_output_module_info);
  if (g_ao)
    open_audio_output (g_ao, rate, channels, encoding);
  cookie->buffer = calloc (1, cookie->buffsize);
  cookie->offset = 0;
  return 0;
}

static int
item_read_stream (dlna_item_t *item)
{
  profile_data_t *cookie = (profile_data_t *)item->profile_cookie;
  //mpg123_profiler_data_t *profiler = cookie->profiler;
  size_t done = 0;
  int err;

  err = mpg123_read( g_profiler_handle, cookie->buffer, cookie->buffsize, &done );

  if (err > MPG123_OK || err == MPG123_ERR)
  {
    dbgprintf ("%s: %s\n", __FUNCTION__, mpg123_strerror (g_profiler_handle));
    return -1;
  }
  if (err == MPG123_DONE || ( err == MPG123_NEED_MORE && cookie->offset >= cookie->length))
  {
    return -1;
  }
  if (err == MPG123_OK && g_ao)
  {
    cookie->offset += done;
    err = write_audio_output (g_ao, cookie->buffer, cookie->buffsize);
    if (err <  0)
       dbgprintf ("%s: %d %ld\n", __FUNCTION__, err, done);
  }
  return 1;
}

static void
item_close_stream (dlna_item_t *item dlna_unused)
{
  //profile_data_t *cookie = (profile_data_t *)item->profile_cookie;
  //mpg123_profiler_data_t *profiler = cookie->profiler;

  mpg123_close(g_profiler_handle);
  if (g_ao)
  {
    close_audio_output (g_ao);
  }
}

#define ONLY_ONE
//#define SELECT_DECODER "generic"
static int
mpg123_profiler_init (dlna_t *dlna dlna_unused)
{
  int ret = 0;
  const char **decoderslist;

  mpg123_init ();
  
  decoderslist = mpg123_decoders();
  while (*decoderslist)
  {
#ifdef SELECT_DECODER
    if (strcmp (*decoderslist, SELECT_DECODER))
    {
      decoderslist ++;
      continue;
    }
#endif
    g_profiler_handle = mpg123_new(*decoderslist, &ret);
    decoderslist ++;
    if (ret)
      continue;
    int i = 0;
    for (i = 0; default_media_profiles[i]; i++)
    {
      dlna_profile_t *media_profile;

      media_profile = (dlna_profile_t *)default_media_profiles[i];
      media_profile->features.playable = 1;
      media_profile->features.store_metadata = 0;
      media_profile->features.store_properties = 0;
      media_profile->free = profile_free;
      media_profile->get_metadata = item_get_metadata;
      media_profile->free_metadata = NULL;
      media_profile->set_metadata = NULL;
      media_profile->get_properties = item_get_properties;
      media_profile->prepare_stream = item_prepare_stream;
      media_profile->read_stream = item_read_stream;
      media_profile->seek_stream = NULL;
      media_profile->close_stream = item_close_stream;
    }
    mpg123_param(g_profiler_handle, MPG123_RESYNC_LIMIT, -1, 0);
#ifdef ONLY_ONE
    break;
#endif
  }
	return ret;
}

static const dlna_profile_t **
mpg123_profiler_get_supported_media_profiles ()
{
  return default_media_profiles;
}

static void
mpg123_profiler_free (dlna_profiler_t *profiler dlna_unused)
{
  mpg123_delete (g_profiler_handle);
  mpg123_exit ();
  if (g_ao)
  {
    close_audio_output (g_ao);
    deinit_audio_output (g_ao);
    g_ao = NULL;
  }
}

static void
profile_free(dlna_item_t *item);
static dlna_properties_t *
item_get_properties (dlna_item_t *item);
static dlna_metadata_t *
item_get_metadata (dlna_item_t *item);

static char*
dup_mpg123_string (mpg123_string *string)
{
  char *ret = NULL;
  if (string && string->fill)
    ret = strndup (string->p,string->size);
  return ret;
}

static const dlna_profile_t *
mpg123_profiler_guess_media_profile (dlna_stream_t *reader, void **cookie)
{
  const dlna_profile_t *profile;
  dlna_properties_t *prop;
  dlna_metadata_t *meta;
  profile_data_t *data;
  int  encoding = MPG123_ENC_SIGNED_32;
  long rate = 44100;
  enum mpg123_channelcount channels;
  off_t length;
  uint32_t time, time_s, time_m, time_h;
  struct mpg123_frameinfo mpg_info;
  int metaflags;
  mpg123_id3v1 *v1 = NULL;
  mpg123_id3v2 *v2 = NULL;
  dlna_media_class_t class;

  mpg123_replace_reader_handle (g_profiler_handle, reader->read, reader->lseek, reader->cleanup);

  if(mpg123_open_handle(g_profiler_handle, reader) != MPG123_OK)
  {
    dbgprintf ("%s 1: %s\n", __FUNCTION__, mpg123_strerror (g_profiler_handle));
    return NULL;
  }
  if (mpg123_scan(g_profiler_handle) != MPG123_OK && 
      mpg123_errcode (g_profiler_handle) != MPG123_NO_SEEK && 
      mpg123_errcode (g_profiler_handle) != MPG123_NO_SEEK_FROM_END)
  {
    dbgprintf ("%s 2: %s\n", __FUNCTION__, mpg123_strerror (g_profiler_handle));
    mpg123_close(g_profiler_handle);
    return NULL;
  }

  if (mpg123_info (g_profiler_handle, &mpg_info) != MPG123_OK)
  {
    dbgprintf ("%s 4: %s\n", __FUNCTION__, mpg123_strerror (g_profiler_handle));
    mpg123_close(g_profiler_handle);
    return NULL;
  }
  if (reader->length > 0)
  {
    mpg123_set_filesize (g_profiler_handle, reader->length);
    length = mpg123_length(g_profiler_handle);
    if (length < MPG123_OK)
    {
      dbgprintf ("%s 3: %s\n", __FUNCTION__, mpg123_strerror (g_profiler_handle));
      mpg123_close(g_profiler_handle);
      return NULL;
    }
  }
  rate = mpg_info.rate;
  channels = (mpg_info.mode == MPG123_M_MONO)? MPG123_MONO:MPG123_STEREO;

  if (reader->length < 0)
    class = DLNA_CLASS_RADIO;
  else
    class = DLNA_CLASS_AUDIO;

  int i;
  for (i = 0; default_profiles[i]; i++)
  {
    if (mpg_info.version == default_profiles[i]->version && 
        mpg_info.layer == default_profiles[i]->layer && 
        channels == default_profiles[i]->channels &&
        class == (default_media_profiles[default_profiles[i]->profileid])->media_class)
      break;
  }

  if (!default_profiles[i])
  {
    dbgprintf ("found MPEG version %d layer %d nb channels %d\n", mpg_info.version, mpg_info.layer, channels);
    mpg123_close(g_profiler_handle);
    return NULL;
  }

  data = calloc (1, sizeof (profile_data_t));
  data->profiler = default_profiles[i];

  /* check the possible output */
  mpg123_format_none(g_profiler_handle);
  mpg123_format(g_profiler_handle, rate, (int)channels, encoding);

	if (mpg123_getformat(g_profiler_handle, &rate, (int *)&channels, &encoding) != MPG123_OK)
  {
    dbgprintf ("%s 5: %s\n", __FUNCTION__, mpg123_strerror (g_profiler_handle));
    mpg123_close(g_profiler_handle);
    return NULL;
  }
  data->buffsize = mpg123_outblock(g_profiler_handle);

  /* properties setup */
  prop = calloc (1, sizeof (dlna_properties_t));

  prop->sample_frequency = rate;
  prop->channels = channels;
  prop->bps = (encoding & MPG123_ENC_8)? 8:(encoding & MPG123_ENC_16)? 16: (encoding & MPG123_ENC_32)? 32: 0;
  prop->spf = mpg123_spf(g_profiler_handle);

  if (length > 0)
  {
    data->length = length;
    time = length / rate;

    time_h = time / 60 / 60;
    time_m = (time / 60) % 60;
    time_s = time % 60;
    snprintf(prop->duration, 63, "%02u:%02u:%02u", time_h, time_m, time_s);
    data->prop = prop;
  }
  else
    snprintf(prop->duration, 63, "00:00:00");
  
  /* metadata setup */
  metaflags = mpg123_meta_check(g_profiler_handle);
  if((metaflags & MPG123_ID3) && mpg123_id3(g_profiler_handle, &v1, &v2) == MPG123_OK)
  {
    meta = calloc (1, sizeof (dlna_metadata_t));
    if (v2)
    {
      meta->title = dup_mpg123_string (v2->title);
      meta->author = dup_mpg123_string (v2->artist);
      meta->album = dup_mpg123_string (v2->album);
      meta->comment = dup_mpg123_string (v2->comment);
      meta->genre = dup_mpg123_string (v2->genre);
    }
    if (v1 && !meta->title && v1->title)
      meta->title = strndup (v1->title,sizeof(v1->title));
    if (v1 && !meta->author && v1->artist)
      meta->author = strndup (v1->artist,sizeof(v1->artist));
    if (v1 && !meta->album && v1->album)
      meta->album = strndup (v1->album,sizeof(v1->album));
    if (v1 && !meta->comment && v1->comment)
      meta->comment = strndup (v1->comment,sizeof(v1->comment));
    if (v1 && !meta->genre)
      meta->genre = strdup ("default");

    mpg123_meta_free (g_profiler_handle);
    data->meta = meta;
  }

  *cookie = data;
  mpg123_close(g_profiler_handle);

  profile = default_media_profiles[data->profiler->profileid];
  return profile;
}

const dlna_profile_t *
mpg123_profiler_get_media_profile (char *profileid)
{
  int i;

  for (i = 0; default_profiles[i]; i++)
  {
    if (!strcmp(profileid, (default_media_profiles[default_profiles[i]->profileid])->id))
    {
      return default_media_profiles[default_profiles[i]->profileid];
    }
  }
  return NULL;
}


const dlna_profiler_t mpg123_profiler = {
  .init = mpg123_profiler_init,
  .guess_media_profile = mpg123_profiler_guess_media_profile,
  .get_media_profile = mpg123_profiler_get_media_profile,
  .get_supported_media_profiles = mpg123_profiler_get_supported_media_profiles,
  .free = mpg123_profiler_free,
};

const dlna_profiler_t *profiler = &mpg123_profiler;
