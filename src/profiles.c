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
dlna_profile_upnp_object_item (const dlna_profile_t *profile)
{
  if (!profile)
    return NULL;

  return dlna_upnp_object_type (profile->media_class);
}

const dlna_profile_t *
dlna_get_media_profile_by_id (dlna_t *dlna, char *profileid)
{
  dlna_profiler_list_t *profilerit;
  const dlna_profile_t *profile;

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
        const dlna_profile_t **profiles = profilerit->profiler->get_supported_media_profiles ();
        while (*profiles)
        {
          char *mime = strdup (reader->mime);
          char *other = strchr (mime, ':');
          if (other)
            *other = 0;
          if ( !strcmp ((*profiles)->mime, mime))
          {
            free (mime);
            break;
          }
          free (mime);
          profiles ++;
        }
        if (!*profiles)
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
  if (item->profile->get_properties && item->profile->features.store_properties)
    item->properties = item->profile->get_properties (item);
  if (item->profile->get_metadata && item->profile->features.store_metadata)
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
dlna_item_metadata (dlna_item_t * item, dlna_object_action_t action)
{
  if (action == GET)
  {
    if (item->metadata)
      return item->metadata;
    else if (item->profile->get_metadata)
      return item->profile->get_metadata (item);
  }
  else if (action == SET)
  {
    if (item->profile->set_metadata)
    {
      item->profile->set_metadata (item);
    }
    return dlna_item_metadata (item, GET);
  }
  else if (action == FREE)
  {
    if (item->profile->free_metadata)
    {
      item->profile->free_metadata (item);
    }
  }
  return NULL;
}

dlna_properties_t *
dlna_item_properties (dlna_item_t * item)
{
  if (item->properties)
    return item->properties;
  else if (item->profile->get_properties)
    return item->profile->get_properties (item);
  else
    return NULL;
}
