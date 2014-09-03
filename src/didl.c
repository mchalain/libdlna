/*
 * libdlna: reference DLNA standards implementation.
 * Copyright (C) 2007-2008 Benjamin Zores <ben@geexbox.org>
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
#define DIDL_ITEM_PARENT_ID                   "parentID"
#define DIDL_ITEM_RESTRICTED                  "restricted"
#define DIDL_ITEM_CLASS                       "upnp:class"
#define DIDL_ITEM_TITLE                       "dc:title"
#define DIDL_ITEM_ARTIST                      "dc:artist"
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
didl_add_value (buffer_t *out, char *param, off_t value)
{
  buffer_appendf (out, " %s=\"%ld\"", param, value);
}

void
didl_add_short_item (buffer_t *out, dlna_dmp_item_t *item)
{
  dlna_metadata_t *metadata;

  buffer_appendf (out, "<%s", DIDL_ITEM);
  didl_add_value (out, DIDL_ITEM_ID, item->id);
  didl_add_value (out, DIDL_ITEM_PARENT_ID, 0);
  buffer_append (out, ">");

  metadata = item->item->metadata;

  if (metadata && strlen (metadata->title) > 1)
    didl_add_tag (out, DIDL_ITEM_TITLE, metadata->title);
  else
    didl_add_tag (out, DIDL_ITEM_TITLE, item->item->filename);
  if (metadata)
  {
    if (strlen ( metadata->author) > 1)
      didl_add_tag (out, DIDL_ITEM_ARTIST, metadata->author);
    if (strlen ( metadata->comment) > 1)
      didl_add_tag (out, DIDL_ITEM_DESCRIPTION, metadata->comment);
    if (strlen ( metadata->album) > 1)
      didl_add_tag (out, DIDL_ITEM_ALBUM, metadata->album);
    if (metadata->track)
      didl_add_value (out, DIDL_ITEM_TRACK, metadata->track);
    if (strlen ( metadata->genre) > 1)
      didl_add_tag (out, DIDL_ITEM_GENRE, metadata->genre);
  }
  buffer_appendf (out, "</%s>", DIDL_ITEM);
}

void
didl_add_item (dlna_t *dlna, buffer_t *out, vfs_item_t *item,
               char *restricted, char *filter)
{
  dlna_item_t *dlna_item;
  char *class;
  int add_item_name;
     
  buffer_appendf (out, "<%s", DIDL_ITEM);
  didl_add_value (out, DIDL_ITEM_ID, item->id);
  didl_add_value (out, DIDL_ITEM_PARENT_ID,
                  item->parent ? item->parent->id : 0);
  didl_add_param (out, DIDL_ITEM_RESTRICTED, restricted);
  buffer_append (out, ">");

  dlna_item = dlna_item_get(dlna, item);
  if (dlna_item)
  {
    class = dlna_profile_upnp_object_item (dlna_item->profile);

    add_item_name = 1;
    if (dlna_item->metadata)
      add_item_name = didl_add_tag (out, DIDL_ITEM_TITLE, dlna_item->metadata->title);
    if (add_item_name)
      didl_add_tag (out, DIDL_ITEM_TITLE, item->title);
  
    didl_add_tag (out, DIDL_ITEM_CLASS, class);

    if (dlna_item->metadata)
    {
      didl_add_tag (out, DIDL_ITEM_ARTIST,
                    dlna_item->metadata->author);
      didl_add_tag (out, DIDL_ITEM_DESCRIPTION,
                    dlna_item->metadata->comment);
      didl_add_tag (out, DIDL_ITEM_ALBUM,
                    dlna_item->metadata->album);
      didl_add_value (out, DIDL_ITEM_TRACK,
                      dlna_item->metadata->track);
      didl_add_tag (out, DIDL_ITEM_GENRE,
                    dlna_item->metadata->genre);
    }
  
    if (filter_has_val (filter, DIDL_RES))
    {
      char *protocol_info;

      protocol_info =
        dlna_write_protocol_info (dlna, DLNA_PROTOCOL_INFO_TYPE_HTTP,
                                DLNA_ORG_PLAY_SPEED_NORMAL,
                                item->u.resource.cnv,
                                DLNA_ORG_OPERATION_RANGE,
                                dlna->flags, dlna_item->profile);
    
      buffer_appendf (out, "<%s", DIDL_RES);
      didl_add_param (out, DIDL_RES_INFO, protocol_info);
      free (protocol_info);
    
      if (dlna_item->filesize && filter_has_val (filter, "@"DIDL_RES_SIZE))
        didl_add_value (out, DIDL_RES_SIZE, dlna_item->filesize);
    
      if (dlna_item->properties)
      {
        didl_add_param (out, DIDL_RES_DURATION,
                    dlna_item->properties->duration);
        didl_add_value (out, DIDL_RES_BITRATE,
                    dlna_item->properties->bitrate);
        didl_add_value (out, DIDL_RES_BPS,
                    dlna_item->properties->bps);
        didl_add_value (out, DIDL_RES_AUDIO_CHANNELS,
                    dlna_item->properties->channels);
        if (strlen (dlna_item->properties->resolution) > 1)
          didl_add_param (out, DIDL_RES_RESOLUTION,
                      dlna_item->properties->resolution);
      }

      buffer_append (out, ">");
      buffer_appendf (out, "http://%s:%d%s/%lu",
                    dlnaGetServerIpAddress (),
                    dlna->port, VIRTUAL_DIR, item->id);
      buffer_appendf (out, "</%s>", DIDL_RES);
    }
  }
  buffer_appendf (out, "</%s>", DIDL_ITEM);
}

void
didl_add_container (buffer_t *out, vfs_item_t *item,
                    char *restricted, char *searchable, char *class)
{
  buffer_appendf (out, "<%s", DIDL_CONTAINER);

  didl_add_value (out, DIDL_CONTAINER_ID, item->id);
  didl_add_value (out, DIDL_CONTAINER_PARENT_ID,
                  item->parent ? item->parent->id : 0);
  
  didl_add_value (out, DIDL_CONTAINER_CHILD_COUNT,
                  item->u.container.children_count);
  
  didl_add_param (out, DIDL_CONTAINER_RESTRICTED, restricted);
  didl_add_param (out, DIDL_CONTAINER_SEARCH, searchable);
  buffer_append (out, ">");

  didl_add_tag (out, DIDL_CONTAINER_CLASS, class);
  didl_add_tag (out, DIDL_CONTAINER_TITLE, item->title);

  buffer_appendf (out, "</%s>", DIDL_CONTAINER);
}
