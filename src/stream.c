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
#include <errno.h>

#include "dlna_internals.h"
#include "network.h"
#include "minmax.h"

#define NORMAL_STREAM
//#define FULLLOAD_STREAM
#define DBUFFER_STREAM
/***********************************************************************
 * scatter gather
 * 
 * solution for variatic network rate.
 * But it's useless with simple buffer, because the kernel already uses
 * a scatter gather.
 * 
 * TODO
 **/
/***********************************************************************
 * stream buffer with complete loading into memory of the file
 * 
 * advantages:
 *  it's possible to seek inside the complete file
 * mistakes:
 *  it uses a lot of memory and with un-terminated stream an error occures
 * !!! This version is not a good solution but it's the first one
 **/
#ifdef FULLLOAD_STREAM
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
    file->length = info.length;

    data = calloc (1, sizeof (struct fullload_data_s));
    data->total = info.length;

    file->private = data;
  }

  return file;
}
#endif
/***********************************************************************
 * double buffer streaming
 * 
 * This buffer is a partial solution to seek inside the stream.
 * advantages:
 *  - capability to seek forward into the file
 *  - for variatic network rate, the buffer creates latence
 *     and the network driver manages the variations.
 * mistakes:
 *  - small capability to seek backward.
 *  - generate blank when seeking a real time stream 
 **/
#ifdef DBUFFER_STREAM

#define DBUFFER_SIZE 8182
struct dbuffer_data_s {
  char buffer[2][DBUFFER_SIZE];
  char *current_buffer;
  off_t offset;
  off_t threshold;
  ssize_t buffersize;
  off_t total_offset;
  int next_ready;
};

static void
dbuffer_fillbuffer (void *opaque, char *buffer)
{
  dlna_stream_t *file = opaque;
  struct dbuffer_data_s *data = file->private;
  ssize_t len;

	len = read (file->fd, buffer, data->buffersize);
  if (len < 0)
    return;
	while (len < data->buffersize)
	{
		len += read (file->fd, buffer + len, data->buffersize - len);
	}
}

static void
dbuffer_fillcurrent (void *opaque)
{
  dlna_stream_t *file = opaque;
  struct dbuffer_data_s *data = file->private;

  dbuffer_fillbuffer (opaque, data->current_buffer);
  data->offset = 0;
}

static void
dbuffer_fillnext (void *opaque)
{
  dlna_stream_t *file = opaque;
  struct dbuffer_data_s *data = file->private;
  char *buffer;

  buffer = 
    (data->current_buffer == data->buffer[0])? data->buffer[1]:data->buffer[0];
  dbuffer_fillbuffer (opaque, buffer);
  data->next_ready = 1;
}

static char *
dbuffer_nextbuffer (void *opaque)
{
  dlna_stream_t *file = opaque;
  struct dbuffer_data_s *data = file->private;

  if (!data->next_ready)
    dbuffer_fillnext (opaque);
  data->current_buffer = 
    (data->current_buffer == data->buffer[0])? data->buffer[1]:data->buffer[0];
  data->next_ready = 0;
  data->offset = 0;
  return data->current_buffer;
}

static void
dbuffer_reset (void *opaque)
{
  dlna_stream_t *file = opaque;
  struct dbuffer_data_s *data = file->private;

  if (data->offset)
  {
    close (file->fd);
    file->fd = http_get (file->url, NULL);
  }
  data->current_buffer = data->buffer[0];
  dbuffer_fillcurrent (opaque);
}

static ssize_t
dbuffer_read (void *opaque, void *buf, size_t len)
{
  dlna_stream_t *file = opaque;
  struct dbuffer_data_s *data = file->private;

  if (len < ((size_t)data->buffersize - (size_t)data->offset))
  {
    /** there is enought data inside the current buffer **/
    memcpy (buf, data->current_buffer + data->offset, len);
    data->offset += len;
    data->total_offset += len;
  }
  else
  {
    off_t wlen = 0;
    /** the requested length requires the next buffer **/
    while (len > ((size_t)data->buffersize - (size_t)data->offset))
    {
      memcpy (buf, data->current_buffer + data->offset, data->buffersize - data->offset);
      data->total_offset += data->buffersize - data->offset;
      len -= data->buffersize - data->offset;
      buf += data->buffersize - data->offset;
      wlen += data->buffersize - data->offset;
      dbuffer_nextbuffer (opaque);
    }
    memcpy (buf, data->current_buffer + data->offset, len);
    data->offset += len;
    data->total_offset += len;
    /** recompute the return of the function **/
    len += wlen;

    if (data->offset >= data->buffersize)
    {
      dbuffer_nextbuffer (opaque);
    }
  }
  if (data->offset >= data->threshold && !data->next_ready)
  {
    /** move inside the current buffer requires to fill the next one **/
    dbuffer_fillnext (opaque);
  }
  return len;
}

static off_t
dbuffer_lseek (void *opaque, off_t len, int whence)
{
  dlna_stream_t *file = opaque;
  struct dbuffer_data_s *data = file->private;
  off_t rlen = 0;

  switch (whence)
  {
  case SEEK_END:
    errno = ESPIPE;
    return (off_t) -1;
    break;
  case SEEK_SET:
    if (len < 0)
    {
      errno = EINVAL;
      return (off_t) -1;
    }
    dbuffer_reset (opaque);
    data->total_offset = len;
    while (0 < len)
    {
      rlen = MIN(len, data->buffersize);
      len -= read (file->fd, data->current_buffer, rlen);
    }
    /** fill the current buffer with requested data for next read **/
    dbuffer_fillcurrent (opaque);
    break;
  case SEEK_CUR:
    if (len == 0)
      return data->total_offset;
    if ((data->offset + len) >= 0 && (data->offset + len) < data->buffersize)
    {
      data->offset += len;
      data->total_offset += len;
      if (data->offset > data->threshold && !data->next_ready)
        dbuffer_fillnext (opaque);
    }
    else if ((data->offset + len) < 0)
    {
      data->total_offset -= data->offset;
      len += data->offset;
      data->offset = 0;
      /** no more old data impossible to seek more  **/
      if (!data->next_ready)
      {
        /** move to the previous buffer **/
        data->next_ready = 1;
        dbuffer_nextbuffer (opaque);
        data->next_ready = 1;
        data->offset = data->buffersize;
        if ((data->offset + len) < 0)
        {
          data->total_offset -= data->offset;
          len += data->offset;
          data->offset = 0;
        }
        else
        {
          data->offset += len;
          data->total_offset += len;
        }
      }
    }
    else if ((data->offset + len) < data->buffersize)
    {
      /** move to the next buffer **/
      while ((data->offset + len) < data->buffersize)
      {
        len -= data->buffersize - data->offset;
        data->total_offset += data->buffersize - data->offset;
        data->offset = 0;
        dbuffer_nextbuffer (opaque);
      }
      /** move to the last bytes **/
      data->offset = len;
      data->total_offset += len;
    }
    break;
  }
  return data->total_offset;
}

static void
dbuffer_cleanup (void *opaque dlna_unused)
{
  dlna_stream_t *file = opaque;
  struct dbuffer_data_s *data = file->private;

  /** force the reopen of the stream **/
  data->offset = -1;
  dbuffer_reset (opaque);
}

static void
dbuffer_close (void *opaque)
{
  dlna_stream_t *file = opaque;
  struct dbuffer_data_s *data = file->private;

  free (file->url);
  close (file->fd);
  free (data);
  free (file);
}

static dlna_stream_t *
dbuffer_open (char *url)
{
  int fd;
  struct http_info info;
  dlna_stream_t *file = NULL;
  struct dbuffer_data_s *data = NULL;

  fd = http_get (url, &info);
  if (fd > 0)
  {
    file = calloc (1, sizeof (dlna_stream_t));
    file->fd = fd;
    file->url = strdup (url);
    file->read = dbuffer_read;
    file->lseek = dbuffer_lseek;
    file->cleanup = dbuffer_cleanup;
    file->close = dbuffer_close;
    strcpy (file->mime, info.mime);

    data = calloc (1, sizeof (struct dbuffer_data_s));
    data->buffersize = DBUFFER_SIZE;
    data->threshold = DBUFFER_SIZE * 9 / 10;

    file->private = data;
    dbuffer_reset (file);
  }

  return file;
}
#endif
/***********************************************************************
 * stream buffer for seekable stream
 * 
 * This solution is not a solution, it's just the use of standard functions
 * to move inside a file, and it's impossible to move inside a stream.
 * advantages:
 *  - no memory used
 *  - no latence
 * mistakes:
 *  - it's impossible to move fast inside a stream.
 *  - if the stream rate is variatic, there isn't any kind of smoothing mechanism
 **/
#ifdef NORMAL_STREAM
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
  free (file->url);
  free (file);
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
    file->length = finfo.st_size;

    file->private = NULL;
  }
  return file;
}

static dlna_stream_t *
seekable_http_open (char *url)
{
  int fd;
  dlna_stream_t *file = NULL;
  struct http_info info;

  fd = http_get (url, &info);
  if (fd > 0)
  {
    file = calloc (1, sizeof (dlna_stream_t));
    file->fd = fd;
    file->url = strdup (url);
    file->read = seekable_read;
    file->lseek = seekable_lseek;
    file->cleanup = seekable_cleanup;
    file->close = seekable_close;
    strcpy (file->mime, info.mime);
    file->length = info.length;

    file->private = NULL;
  }
  return file;
}

#endif
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
#ifdef DBUFFER_STREAM
    return dbuffer_open (url);
#endif
#ifdef FULLLOAD_STREAM
    return fullload_open (url);
#endif
    return seekable_http_open (url);
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
