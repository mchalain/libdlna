/*
 * libdlna: reference DLNA standards implementation.
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
#include "sound_module.h"
#include "network.h"

static dlna_properties_t *item_get_properties (dlna_item_t *item);
static dlna_metadata_t *item_get_metadata (dlna_item_t *item);
static int item_prepare_stream (dlna_item_t *item);
static int item_read_stream (dlna_item_t *item);
static void item_close_stream (dlna_item_t *item);

static dlna_profile_t *default_profiles[] = {
  & (dlna_profile_t) {
    .id = "MP3",
    .mime = MIME_AUDIO_MPEG,
    .label = LABEL_AUDIO_2CH,
  },
	NULL
};

typedef struct mpg123_profiler_data_s mpg123_profiler_data_t;
struct mpg123_profiler_data_s
{
  dlna_profile_t *profile;
  mpg123_handle *handle;
  struct sound_module *sound;
  mpg123_profiler_data_t *next;
  mpg123_profiler_data_t *previous;
};

typedef struct profile_data_s profile_data_t;
struct profile_data_s
{
  mpg123_profiler_data_t *profiler;
  dlna_properties_t *prop;
  int fd;
  ssize_t buffsize;
  void *buffer;
  uint32_t offset;
};

mpg123_profiler_data_t *g_profiler = NULL;

int
open_url (char *url, int mode, struct http_info *info)
{
  int fd;
  int len = 0;

  if (!strncmp (url, "file:", 5))
  {
    struct stat finfo;
    fd = open (url + 5, mode);
    if (!fstat (fd, &finfo))
      len = finfo.st_size;
  }
  else if (!strncmp (url, "http:", 5))
    fd = http_get (url, info);
  else
  {
    struct stat finfo;
    fd = open (url, mode);
    if (!fstat (fd, &finfo))
      len = finfo.st_size;
  }
  if (len && info)
    info->length = len;
  return fd;
}

int
mpg123_profiler_init ()
{
  int ret = 0;
  mpg123_pars *pars;
  mpg123_profiler_data_t *profiler;
  mpg123_profiler_data_t *previous;
  const char **decoderslist;

  mpg123_init ();
  pars = mpg123_new_pars(&ret);
  if (ret)
    return ret;

	decoderslist = mpg123_decoders();
  while (*decoderslist)
  {
    printf ("decoder name %s\n", *decoderslist);
    if (!g_profiler)
    {
      profiler = calloc (1, sizeof (mpg123_profiler_data_t));
      g_profiler = profiler;
    }
    else
    {
      profiler->next = calloc (1, sizeof (mpg123_profiler_data_t));
      previous = profiler;
      profiler = profiler->next;
      profiler->previous = previous;
    }
    profiler->handle = mpg123_new(*decoderslist, &ret);
    if (ret)
    {
      free (profiler);
      previous->next = NULL;
      break;
    }
    mpg123_param(profiler->handle, MPG123_RESYNC_LIMIT, -1, 0);
    profiler->profile = default_profiles[0];
    profiler->sound = sound_module_get();
    decoderslist ++;
  }
	return ret;
}

static void
dlna_metadata_free (dlna_metadata_t *meta)
{
  if (!meta)
    return;

  if (meta->title)
    free (meta->title);
  if (meta->author)
    free (meta->author);
  if (meta->comment)
    free (meta->comment);
  if (meta->album)
    free (meta->album);
  if (meta->genre)
    free (meta->genre);
  free (meta);
}

static int
dlna_list_length (void *list)
{
  void **l = list;
  int n = 0;
  while (*l++)
    n++;

  return n;
}

static void *
dlna_list_add (char **list, char *element)
{
  char **l = list;
  int n = dlna_list_length (list) + 1;
  int i;

  for (i = 0; i < n; i++)
    if (l[i] && element && !strcmp (l[i], element))
      return l;
  
  l = realloc (l, (n + 1) * sizeof (char *));
  l[n] = NULL;
  l[n - 1] = element;
  
  return l;
}

char **
mpg123_profiler_get_supported_mime_types (char **mimes)
{
  mpg123_profiler_data_t *profiler;
  
  profiler = g_profiler;
  while (profiler)
  {
    mimes = dlna_list_add (mimes, profiler->profile->mime);
    profiler = profiler->next;
  }
  return mimes;
}

static void
profile_free(dlna_item_t *item);
static dlna_properties_t *
item_get_properties (dlna_item_t *item);
static dlna_metadata_t *
item_get_metadata (dlna_item_t *item);

int
mpg123_openstream(mpg123_handle *hd, int fdin, int *channels, int *encoding, long *rate, long *buffsize)
{
	if(mpg123_open_fd(hd, fdin) != MPG123_OK)
	{
    printf ("%s: %s\n", __FUNCTION__, mpg123_strerror (hd));
		return -1;
	}

	if (*channels == 0 && mpg123_getformat(hd, rate, channels, encoding) != MPG123_OK)
	{
    printf ("%s: %s\n", __FUNCTION__, mpg123_strerror (hd));
    mpg123_close(hd);
		return -1;
	}
	mpg123_format_none(hd);
	mpg123_format(hd, *rate, *channels, *encoding);

	if (mpg123_getformat(hd, rate, channels, encoding) != MPG123_OK)
	{
    printf ("%s: %s\n", __FUNCTION__, mpg123_strerror (hd));
    mpg123_close(hd);
		return -1;
	}
	*buffsize = mpg123_outblock(hd);
	return 0;
}

dlna_profile_t *
mpg123_profiler_guess_media_profile (char *filename, void **cookie)
{
  dlna_profile_t *profile;
  profile_data_t *data;
  mpg123_profiler_data_t *profiler;
  int  channels = 2, encoding = MPG123_ENC_SIGNED_32;
  long rate = 44100, buffsize = 0;
  int fd;
  uint32_t time, time_s, time_m, time_h;
  uint32_t len;
  struct http_info info;
  
  fd = open_url (filename, O_RDONLY, &info);
  
  profiler = g_profiler;
  while (mpg123_openstream (profiler->handle, fd, &channels, &encoding, &rate, &buffsize))
  {
    profiler = profiler->next;
    if (!profiler)
      return NULL;
  }

  profile = profiler->profile;
  data = calloc (1, sizeof (profile_data_t));
  data->buffsize = buffsize;
  data->profiler = profiler;
  data->prop = calloc (1, sizeof (dlna_properties_t));
  data->prop->sample_frequency = rate;
  data->prop->channels = channels;

  mpg123_set_filesize (profiler->handle, info.length);
  len = mpg123_length(profiler->handle);
  time = len / (rate * ((encoding & MPG123_ENC_8)? 1 : (encoding & MPG123_ENC_16)? 2: 4));

  time_h = time / 60 / 60;
  time_m = (time / 60) % 60;
  time_s = time % 60;
  snprintf(data->prop->duration, 63, "%02u:%02u:%02u", time_h, time_m, time_s);

  *cookie = data;
  mpg123_close(profiler->handle);

  profile->free = profile_free;
  profile->get_metadata = item_get_metadata;
  profile->get_properties = item_get_properties;
  profile->prepare_stream = item_prepare_stream;
  profile->read_stream = item_read_stream;
  profile->close_stream = item_close_stream;

  close (fd);
  return profile;
}

static void
profile_free(dlna_item_t *item)
{
  profile_data_t *cookie = (profile_data_t *)item->profile_cookie;

  if (item->metadata)
    dlna_metadata_free (item->metadata);
  if (cookie->prop)
    free (cookie->prop);
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
  dlna_metadata_t *meta;

  return NULL;
}

static int
item_prepare_stream (dlna_item_t *item)
{
  profile_data_t *cookie = (profile_data_t *)item->profile_cookie;
  mpg123_profiler_data_t *profiler = cookie->profiler;
  int  channels = 2, encoding = MPG123_ENC_SIGNED_32;
  long rate = 44100, buffsize = 0;

  cookie->fd = open_url (item->filename, O_RDONLY, NULL);

  if (mpg123_openstream (profiler->handle, cookie->fd, &channels, &encoding, &rate, &buffsize))
  {
    printf ("%s: %s\n", __FUNCTION__, mpg123_strerror (profiler->handle));
    return -1;
  }
  profiler->sound->open (channels, encoding, rate);
  cookie->buffer = calloc (1, cookie->buffsize);
  cookie->offset = 0;
  return 0;
}

static int
item_read_stream (dlna_item_t *item)
{
  profile_data_t *cookie = (profile_data_t *)item->profile_cookie;
  mpg123_profiler_data_t *profiler = cookie->profiler;
  size_t done = 0;
  int err;

  err = mpg123_read( profiler->handle, cookie->buffer, cookie->buffsize, &done );
  if (err > MPG123_OK || err == MPG123_ERR)
  {
    printf ("%s: %s\n", __FUNCTION__, mpg123_strerror (profiler->handle));
    return -1;
  }
  if (err == MPG123_OK)
  {
    cookie->offset += done;
    profiler->sound->write (cookie->buffer, cookie->buffsize);
  }
  return 1;
}

static void
item_close_stream (dlna_item_t *item)
{
  profile_data_t *cookie = (profile_data_t *)item->profile_cookie;
  mpg123_profiler_data_t *profiler = cookie->profiler;

  mpg123_close(profiler->handle);
  close (cookie->fd);
  profiler->sound->close ();
}

dlna_profile_t *
mpg123_profiler_get_media_profile (char *profileid)
{
  mpg123_profiler_data_t *profiler;
  int i = 0;

  profiler = g_profiler;
  while (profiler)
  {
    if (!strcmp(profileid, profiler->profile->id))
    {
      return profiler;
    }
    profiler = profiler->next;
  }
  return NULL;
}


const dlna_profiler_t mpg123_profiler = {
  .guess_media_profile = mpg123_profiler_guess_media_profile,
  .get_media_profile = mpg123_profiler_get_media_profile,
  .get_supported_mime_types = mpg123_profiler_get_supported_mime_types,
};
