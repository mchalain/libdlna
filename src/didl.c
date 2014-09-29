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
#include <stdio.h>

#include "upnp_internals.h"
#include "vfs.h"
#include "didl.h"

/* CDS DIDL Messages */
#define DIDL_NAMESPACE \
    "xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\" " \
    "xmlns:dc=\"http://purl.org/dc/elements/1.1/\" " \
    "xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\""

#define DIDL_LITE                             "DIDL-Lite"
#define DIDL_ITEM                             "item"
#define DIDL_ITEM_ID                          "id"
#define DIDL_ITEM_REFID                       "refID"
#define DIDL_ITEM_PARENT_ID                   "parentID"
#define DIDL_ITEM_RESTRICTED                  "restricted"
#define DIDL_ITEM_CLASS                       "upnp:class"
#define DIDL_ITEM_TITLE                       "dc:title"
#define DIDL_ITEM_CREATOR                     "dc:creator"
//#define DIDL_ITEM_ARTIST                      "dc:artist"
#define DIDL_ITEM_ARTIST                      "upnp:artist"
#define DIDL_ITEM_DESCRIPTION                 "dc:description"
#define DIDL_ITEM_ALBUM                       "upnp:album"
#define DIDL_ITEM_TRACK                       "upnp:originalTrackNumber"
#define DIDL_ITEM_GENRE                       "upnp:genre"
#define DIDL_RES                              "res"
#define DIDL_RES_INFO                         "protocolInfo"
#define DIDL_RES_SIZE                         "size"
#define DIDL_RES_DURATION                     "duration"
#define DIDL_RES_BITRATE                      "bitrate"
#define DIDL_RES_SAMPLE_FREQUENCY             "sampleFrequency"
#define DIDL_RES_BPS                          "bitsPerSample"
#define DIDL_RES_AUDIO_CHANNELS               "nrAudioChannels"
#define DIDL_RES_RESOLUTION                   "resolution"
#define DIDL_CONTAINER                        "container"
#define DIDL_CONTAINER_ID                     "id"
#define DIDL_CONTAINER_PARENT_ID              "parentID"
#define DIDL_CONTAINER_CHILD_COUNT            "childCount"
#define DIDL_CONTAINER_RESTRICTED             "restricted"
#define DIDL_CONTAINER_SEARCH                 "searchable"
#define DIDL_CONTAINER_CLASS                  "upnp:class"
#define DIDL_CONTAINER_TITLE                  "dc:title"

static int
filter_has_val (const char *filter, const char *val)
{
  char *x = NULL, *token = NULL;
  char *m_buffer = NULL, *buffer;
  int len = strlen (val);
  int ret = 0;

  if (!strcmp (filter, "*"))
    return 1;

  x = strdup (filter);
  if (!x)
    return 0;

  m_buffer = malloc (strlen (x));
  if (!m_buffer)
  {
    free (x);
    return 0;
  }

  buffer = m_buffer;
  token = strtok_r (x, ",", &buffer);
  while (token)
  {
    if (*val == '@')
      token = strchr (token, '@');
    if (token && !strncmp (token, val, len))
    {
      ret = 1;
      break;
    }
    token = strtok_r (NULL, ",", &buffer);
  }

  free (m_buffer);
  free (x);
  
  return ret;
}

void
didl_add_header (buffer_t *out)
{
  buffer_appendf (out, "<%s %s>", DIDL_LITE, DIDL_NAMESPACE);
}

void
didl_add_footer (buffer_t *out)
{
  buffer_appendf (out, "</%s>", DIDL_LITE);
}

int
didl_add_tag (buffer_t *out, char *tag, char *value)
{
  if (value && *value != '\0')
    buffer_appendf (out, "<%s>%s</%s>", tag, value, tag);
  else
    return -1;
  return 0;
}

void
didl_add_param (buffer_t *out, char *param, char *value)
{
  if (value)
    buffer_appendf (out, " %s=\"%s\"", param, value);
}

void
didl_add_value (buffer_t *out, char *param, uint32_t value)
{
  buffer_appendf (out, " %s=\"%u\"", param, value);
}

void
didl_add_item (buffer_t *out, vfs_item_t *item,
    char *filter, dlna_org_flags_t flags)
{
  char *class;
  int add_item_name;
  dlna_item_t *dlna_item;

  if (!item)
    return;

  buffer_appendf (out, "<%s", DIDL_ITEM);
  didl_add_value (out, DIDL_ITEM_ID, item->id);
  didl_add_value (out, DIDL_ITEM_PARENT_ID, item->parent ? item->parent->id : 0);
  didl_add_value (out, DIDL_ITEM_RESTRICTED, item->restricted?1:0);
  buffer_append (out, ">");

  dlna_item = vfs_item_get (item);
  if (dlna_item)
  {
    dlna_metadata_t *metadata;
    vfs_resource_t *resource;

    metadata = dlna_item_metadata (dlna_item);

    add_item_name = 1;
    if (metadata)
      add_item_name = didl_add_tag (out, DIDL_ITEM_TITLE, metadata->title);
    if (add_item_name)
      didl_add_tag (out, DIDL_ITEM_TITLE, basename (dlna_item->filename));

    resource = vfs_resource_get (item);
    class = dlna_profile_upnp_object_item (resource->profile);
    didl_add_tag (out, DIDL_ITEM_CLASS, class);

    if (metadata)
    {
      if (!filter || filter_has_val (filter, DIDL_ITEM_CREATOR))
      {
        didl_add_tag (out, DIDL_ITEM_CREATOR, metadata->author);
      }
      if (!filter || filter_has_val (filter, DIDL_ITEM_ARTIST))
      {
        didl_add_tag (out, DIDL_ITEM_ARTIST, metadata->author);
      }
      if (!filter || filter_has_val (filter, DIDL_ITEM_DESCRIPTION))
      {
        didl_add_tag (out, DIDL_ITEM_DESCRIPTION, metadata->comment);
      }
      if (!filter || filter_has_val (filter, DIDL_ITEM_ALBUM))
      {
        didl_add_tag (out, DIDL_ITEM_ALBUM, metadata->album);
      }
      if (!filter || filter_has_val (filter, DIDL_ITEM_TRACK))
      {
        char track[10];
        snprintf(track,9,"%u", metadata->track);
        didl_add_tag (out, DIDL_ITEM_TRACK, track);
      }
      if (!filter || filter_has_val (filter, DIDL_ITEM_GENRE))
      {
        didl_add_tag (out, DIDL_ITEM_GENRE, metadata->genre);
      }
    }
  
    if ((!filter || filter_has_val (filter, DIDL_RES)))
    {
      while (resource)
      {
        char *protocol_info;
        char *url;

      buffer_appendf (out, "<%s", DIDL_RES);
      protocol_info =
        dlna_write_protocol_info (NULL, resource->protocolid,
                            DLNA_ORG_PLAY_SPEED_NORMAL,
                            item->u.resource.cnv,
                            DLNA_ORG_OPERATION_RANGE,
                            flags, resource->profile);
      if (protocol_info)
        didl_add_param (out, DIDL_RES_INFO, protocol_info);
      free (protocol_info);

      if ((resource->size > 0) && filter_has_val (filter, "@"DIDL_RES_SIZE))
        didl_add_value (out, DIDL_RES_SIZE, resource->size);
    
        didl_add_param (out, DIDL_RES_DURATION, resource->properties.duration);
        didl_add_value (out, DIDL_RES_BITRATE, resource->properties.bitrate);
        didl_add_value (out, DIDL_RES_BPS, resource->properties.bps);
        didl_add_value (out, DIDL_RES_AUDIO_CHANNELS, resource->properties.channels);
        didl_add_param (out, DIDL_RES_RESOLUTION, resource->properties.resolution);

      buffer_append (out, ">");
        url = resource->url(item);
        buffer_appendf (out, url);
        free (url);
      buffer_appendf (out, "</%s>", DIDL_RES);
        resource = resource->next;
      }
    }
  }
  buffer_appendf (out, "</%s>", DIDL_ITEM);
}

void
didl_add_container (buffer_t *out, vfs_item_t *item, uint32_t searchable)
{
  buffer_appendf (out, "<%s", DIDL_CONTAINER);

  didl_add_value (out, DIDL_CONTAINER_ID, item->id);
  didl_add_value (out, DIDL_CONTAINER_PARENT_ID,
                  item->parent ? item->parent->id : 0);
  
  didl_add_value (out, DIDL_CONTAINER_CHILD_COUNT,
                  item->u.container.children_count);
  
  didl_add_value (out, DIDL_CONTAINER_RESTRICTED, item->restricted?1:0);
  didl_add_value (out, DIDL_CONTAINER_SEARCH, searchable?1:0);
  buffer_append (out, ">");

  didl_add_tag (out, DIDL_CONTAINER_CLASS, dlna_upnp_object_type (item->u.container.media_class));
  didl_add_tag (out, DIDL_CONTAINER_TITLE, item->u.container.title);

  buffer_appendf (out, "</%s>", DIDL_CONTAINER);
}
