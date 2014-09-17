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

#include "dlna_internals.h"
#include "network.h"

/***********************************************************************
 * stream buffer with complete loading into memory of the file
 * 
 * !!! This version is not a good solution but it's the first one
 **/
struct fullload_data_s {
  void *buffer;
  ssize_t size;
  off_t offset;
  ssize_t total;
};

static ssize_t
fullload_read (void *opaque, void *buf, size_t len)
{
  dlna_stream_t *file = opaque;
  struct fullload_data_s *data = file->private;

  while (data->size < data->offset)
  {
    data->buffer = realloc (data->buffer, data->offset);
    data->size += read (file->fd, data->buffer + data->size, data->offset - data->size);
  }
  if (data->offset + len <= data->size)
  {
    len = (len > data->size - data->offset) ? data->size - data->offset:len;
    memcpy (buf, data->buffer + data->offset, len);
    data->offset += len;
  }
  else if (data->size < data->total )
  {
    off_t offset = data->size;
    size_t rlen = 0;
    data->buffer = realloc (data->buffer, data->offset + len);
    while (data->offset - data->size + len - rlen > 0)
      rlen += read (file->fd, data->buffer + offset, data->offset - data->size + len - rlen);
    memcpy (buf, data->buffer + data->offset, len);
    data->offset += len;
    data->size = data->offset;
  }
  else
  {
    len = 0;
  }
  return len;
}

static off_t
fullload_lseek (void *opaque, off_t len, int whence)
{
  dlna_stream_t *file = opaque;
  struct fullload_data_s *data = file->private;

  switch (whence)
  {
  case SEEK_END:
    data->offset = data->total + len;
    break;
  case SEEK_SET:
    data->offset = len;
    break;
  case SEEK_CUR:
    data->offset += len;
    break;
  }
  return data->offset;
}

static void
fullload_cleanup (void *opaque)
{
  dlna_stream_t *file = opaque;
  struct fullload_data_s *data = file->private;

  data->size = 0;
  data->offset = 0;
}

static void
fullload_close (void *opaque)
{
  dlna_stream_t *file = opaque;
  struct fullload_data_s *data = file->private;

  close (file->fd);
  free (file->url);
  free (data->buffer);
  data->buffer = NULL;
  free (data);
  free (file);
}

static dlna_stream_t *
fullload_open (char *url)
{
  int fd;
  struct http_info info;
  dlna_stream_t *file = NULL;
  struct fullload_data_s *data = NULL;

  fd = http_get (url, &info);
  if (fd > 0)
  {
    file = calloc (1, sizeof (dlna_stream_t));
    file->fd = fd;
    file->url = strdup (url);
    file->read = fullload_read;
    file->lseek = fullload_lseek;
    file->cleanup = fullload_cleanup;
    file->close = fullload_close;
    strcpy (file->mime, info.mime);

    data = calloc (1, sizeof (struct fullload_data_s));
    data->total = info.length;

    file->private = data;
  }

  return file;
}

/***********************************************************************
 * stream buffer for seekable stream
 **/
static ssize_t
seekable_read (void *opaque, void *buf, size_t len)
{
  dlna_stream_t *file = opaque;
  return read (file->fd, buf, len);
}

static off_t
seekable_lseek (void *opaque, off_t len, int whence)
{
  dlna_stream_t *file = opaque;
  return lseek (file->fd, len, whence);
}

static void
seekable_cleanup (void *opaque dlna_unused)
{
//  dlna_stream_t *file = opaque;
}

static void
seekable_close (void *opaque)
{
  dlna_stream_t *file = opaque;
  close (file->fd);
}

static dlna_stream_t *
seekable_open (char *url)
{
  int fd;
  dlna_stream_t *file = NULL;
  struct stat finfo;
  dlna_profile_t *profile;

  fd = open (url, O_RDONLY);
  if (!fstat (fd, &finfo))
  {
    file = calloc (1, sizeof (dlna_stream_t));
    file->fd = fd;
    file->url = strdup (url);
    file->read = seekable_read;
    file->lseek = seekable_lseek;
    file->cleanup = seekable_cleanup;
    file->close = seekable_close;

    profile = upnpav_profiler.guess_media_profile (file, NULL);
    if (profile)
      strcpy (file->mime, profile->mime);

    file->private = NULL;
  }
  return file;
}
/***********************************************************************
 * stream internal API
 **********************************************************************/
dlna_stream_t *
stream_open (char *url)
{
  if (!strncmp (url, "file:", 5))
  {
    return seekable_open (url + 5);
  }
  else if (!strncmp (url, "http:", 5))
  {
    return fullload_open (url);
  }
  else
  {
    return seekable_open (url);
  }
  return NULL;
}

void
stream_close (dlna_stream_t *stream)
{
  stream->close (stream);
}
