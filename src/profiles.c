/*
 * libdlna: reference DLNA standards implementation.
 * Copyright (C) 2007-2008 Benjamin Zores <ben@geexbox.org>
 * Copyright (C) 2014-2016 Marc Chalain <marc.chalain@gmail.com>
 *
 * This file is part of libdlna.
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
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "dlna_internals.h"
#include "vfs.h"
#include "network.h"

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

void
dlna_append_supported_mime_types (dlna_t *dlna, int sink, char *mime)
{
  if (sink)
  {
    if (!dlna->cms.sinkmimes)
    {
      dlna->cms.sinkmimes = malloc (sizeof (char*));
      *dlna->cms.sinkmimes = NULL;
	}
    dlna->cms.sinkmimes = dlna_list_add (dlna->cms.sinkmimes, mime);
  }
  else
  {
    if (!dlna->cms.sourcemimes)
    {
      dlna->cms.sourcemimes = malloc (sizeof (char*));
      *dlna->cms.sourcemimes = NULL;
	}
    dlna->cms.sourcemimes = dlna_list_add (dlna->cms.sourcemimes, mime);
  }
}

/* UPnP ContentDirectory Object Item */
static char *upnp_object_type[] = {
	[DLNA_CLASS_IMAGE] = "object.item.imageItem.photo",
	[DLNA_CLASS_AUDIO] = "object.item.audioItem.musicTrack",
	[DLNA_CLASS_AV] = "object.item.videoItem.movie",
	[DLNA_CLASS_COLLECTION] = "object.container.playlistContainer",
	[DLNA_CLASS_RADIO] = "object.item.audioItem.audioBroadcast",
	[DLNA_CLASS_TV] = "object.item.videoItem.videoBroadcast",
	[DLNA_CLASS_FOLDER] = "object.container.storageFolder",
	[DLNA_CLASS_ALBUM] = "object.container.album.musicAlbum",
};

char *
dlna_upnp_object_type (dlna_media_class_t media_class)
{
  if (media_class < DLNA_CLASS_LAST)
	return upnp_object_type[media_class];
  else
    return NULL;
}

char *
dlna_profile_upnp_object_item (dlna_profile_t *profile)
{
  if (!profile)
    return NULL;

  return dlna_upnp_object_type (profile->media_class);
}

dlna_profile_t *
dlna_get_media_profile_by_id (dlna_t *dlna, char *profileid)
{
  dlna_profiler_list_t *profilerit;
  dlna_profile_t *profile;

  if (!profileid)
    return NULL;
  for (profilerit = dlna->profilers; profilerit; profilerit = profilerit->next)
  {
    profile = profilerit->profiler->get_media_profile (profileid);
    if (profile)
      break;
  }
  return profile;
}

dlna_item_t *
dlna_item_new (dlna_t *dlna, const char *filename)
{
  dlna_profiler_list_t *profilerit;
  dlna_item_t *item;
  dlna_stream_t *reader;

  if (!dlna || !filename)
    return NULL;
  
  if (!dlna->inited)
    dlna = dlna_init ();

  item = calloc (1, sizeof (dlna_item_t));

  item->filename   = strdup (filename);
  reader = stream_open (item->filename);

  if (reader)
  {
    for (profilerit = dlna->profilers; profilerit; profilerit = profilerit->next)
    {
      if (strlen (reader->mime) > 0)
      {
        char **mimestable = profilerit->profiler->get_supported_mime_types ();
        while (*mimestable)
        {
          if ( !strcmp (*mimestable, reader->mime))
          {
            break;
          }
          mimestable ++;
        }
        if (!*mimestable)
        {
          continue;
        }
      }
      item->profile = profilerit->profiler->guess_media_profile (reader, &item->profile_cookie);
      reader->cleanup (reader);
      if (item->profile)
        break;
    }
    if (reader->length > 0)
      item->filesize = reader->length;
    stream_close (reader);
  }

  if (!item->profile) /* not DLNA compliant */
  {
    dlna_log (DLNA_MSG_CRITICAL, "can't open file: %s\n", filename);
    free (item->filename);
    free (item);
    return NULL;
  }
  if (item->profile->get_properties)
    item->properties = item->profile->get_properties (item);
  if (item->profile->get_metadata)
    item->metadata   = item->profile->get_metadata (item);
  return item;
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

void
dlna_item_free (dlna_item_t *item)
{
  if (!item)
    return;

  if (item->profile->free)
    item->profile->free (item);
  if (item->filename)
    free (item->filename);
  if (item->properties)
    free (item->properties);
  if (item->metadata)
    dlna_metadata_free (item->metadata);
  item->profile = NULL;
  free (item);
}

const char *
dlna_item_mime (dlna_item_t * item)
{
  return item->profile->mime;
}

dlna_media_class_t
dlna_item_type (dlna_item_t * item)
{
  return item->profile->media_class;
}

dlna_metadata_t *
dlna_item_metadata (dlna_item_t * item)
{
  if (item->metadata)
    return item->metadata;
  else if (item->profile->get_metadata)
    return item->profile->get_metadata (item);
  else
    return NULL;
}
