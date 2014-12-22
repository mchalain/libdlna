/*
 * libdlna: reference DLNA standards implementation.
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
#include "cms.h"

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

void *
didl_new ()
{
  IXML_Document *doc;
  ixmlParseBufferEx( "<"DIDL_LITE" "DIDL_NAMESPACE"></"DIDL_LITE">",&doc);
  return doc;
}

static IXML_Element *
didl_append_tag (IXML_Document *doc, IXML_Element *itemNode, char *tag, char *value)
{
  IXML_Element *elem = NULL;
  if (value && *value != '\0')
  {
    IXML_Node *text;
    elem = ixmlDocument_createElement (doc, tag);
    text = ixmlDocument_createTextNode (doc, value);
    ixmlNode_appendChild ((IXML_Node *)elem, text);
    ixmlNode_appendChild ((IXML_Node *)itemNode, (IXML_Node *)elem);
  }
  return elem;
}

int
didl_append_item (void *didl, vfs_item_t *item, char *filter)
{
  IXML_Document *doc = didl;
  if (item && item->type == DLNA_RESOURCE)
  {
    IXML_Element *elem;
    char valuestr[10];
    dlna_metadata_t *metadata;

    elem = ixmlDocument_createElement (doc, DIDL_ITEM);
    sprintf (valuestr, "%9u", item->id);
    ixmlElement_setAttribute (elem, DIDL_ITEM_ID, valuestr);
    if (item->parent && item != item->parent)
      sprintf (valuestr, "%9u", item->parent->id);
    else
      strcpy (valuestr , "-1");
    ixmlElement_setAttribute (elem, DIDL_ITEM_PARENT_ID, valuestr);
    sprintf (valuestr, "%1u", item->restricted?1:0);
    ixmlElement_setAttribute (elem, DIDL_ITEM_RESTRICTED, valuestr);

    dlna_item_t *dlna_item;
    dlna_item = vfs_item_get (item);
    IXML_Element *title = NULL;
    if (dlna_item)
    {
      metadata = dlna_item_metadata (dlna_item, GET);

      if (metadata)
        title = didl_append_tag (doc, elem, DIDL_ITEM_TITLE, metadata->title);
    }
    if (!title)
      title = didl_append_tag (doc, elem, DIDL_ITEM_TITLE, basename (dlna_item->filename));

    if (dlna_item)
    {
      char *class;
      class = dlna_profile_upnp_object_item (dlna_item->profile);
      didl_append_tag (doc, elem, DIDL_ITEM_CLASS, class);
    }

    if (metadata)
    {
      if (!filter || filter_has_val (filter, DIDL_ITEM_CREATOR))
      {
        didl_append_tag (doc, elem, DIDL_ITEM_CREATOR, metadata->author);
      }
      if (!filter || filter_has_val (filter, DIDL_ITEM_ARTIST))
      {
        didl_append_tag (doc, elem, DIDL_ITEM_ARTIST, metadata->author);
      }
      if (!filter || filter_has_val (filter, DIDL_ITEM_DESCRIPTION))
      {
        didl_append_tag (doc, elem, DIDL_ITEM_DESCRIPTION, metadata->comment);
      }
      if (!filter || filter_has_val (filter, DIDL_ITEM_ALBUM))
      {
        didl_append_tag (doc, elem, DIDL_ITEM_ALBUM, metadata->album);
      }
      if (!filter || filter_has_val (filter, DIDL_ITEM_TRACK))
      {
        char track[10];
        snprintf(track,9,"%u", metadata->track);
        didl_append_tag (doc, elem, DIDL_ITEM_TRACK, track);
      }
      if (!filter || filter_has_val (filter, DIDL_ITEM_GENRE))
      {
        didl_append_tag (doc, elem, DIDL_ITEM_GENRE, metadata->genre);
      }
    }

    if ((!filter || filter_has_val (filter, DIDL_RES)))
    {
      vfs_resource_t *resource;

      resource = vfs_resource_get (item);
      while (resource)
      {
        char *url;
        IXML_Element *resNode;
        buffer_t *out;

        url = resource->url (resource);
        resNode = didl_append_tag (doc, elem, DIDL_RES, url);
        free (url);

        out = buffer_new();
        cms_write_protocol_info (out, resource->protocol_info);
        if (resource->protocol_info->other)
          buffer_appendf (out, ";DLNA.ORG_PS=%d;DLNA.ORG_CI=%d;DLNA.ORG_OP=%02d;",
                resource->info.speed, resource->info.cnv, resource->info.op);
        ixmlElement_setAttribute (resNode, DIDL_RES_INFO, out->buf);
        buffer_free(out);

        if ((resource->size > 0) && filter_has_val (filter, "@"DIDL_RES_SIZE))
        {
          sprintf (valuestr, "%9ld", resource->size);
          ixmlElement_setAttribute (resNode, DIDL_RES_SIZE, valuestr);
        }
        sprintf (valuestr, "%9u", resource->properties.bitrate);
        ixmlElement_setAttribute (resNode, DIDL_RES_BITRATE, valuestr);
        if (resource->properties.duration)
        sprintf (valuestr, "%9u", resource->properties.bps);
        ixmlElement_setAttribute (resNode, DIDL_RES_BPS, valuestr);
        if (resource->properties.duration)
        sprintf (valuestr, "%9u", resource->properties.channels);
        ixmlElement_setAttribute (resNode, DIDL_RES_AUDIO_CHANNELS, valuestr);
        if (resource->properties.duration)
          ixmlElement_setAttribute (elem, DIDL_RES_DURATION, resource->properties.duration);
        if (resource->properties.resolution)
          ixmlElement_setAttribute (resNode, DIDL_RES_RESOLUTION, resource->properties.resolution);

        resource = resource->next;
      }
    }
    if (dlna_item)
      dlna_item_metadata (dlna_item, FREE);

    IXML_Node *first = ixmlNode_getFirstChild( ( IXML_Node * ) doc );
    ixmlNode_appendChild (first, (IXML_Node *)elem);
  }
  else
    return -1;
  return 0;
}

int
didl_append_container (void *didl, vfs_item_t *item, uint32_t searchable)
{
  IXML_Document *doc = didl;
  if (item && item->type == DLNA_CONTAINER)
  {
    IXML_Element *elem;
    char valuestr[10];

    elem = ixmlDocument_createElement (doc, DIDL_CONTAINER);
    sprintf (valuestr, "%9u", item->id);
    ixmlElement_setAttribute (elem, DIDL_CONTAINER_ID, valuestr);
    if (item->parent && item != item->parent)
      sprintf (valuestr, "%9u", item->parent->id);
    else
      strcpy (valuestr , "-1");
    ixmlElement_setAttribute (elem, DIDL_ITEM_PARENT_ID, valuestr);
    sprintf (valuestr, "%9u", item->u.container.children_count);
    ixmlElement_setAttribute (elem, DIDL_CONTAINER_CHILD_COUNT, valuestr);
    sprintf (valuestr, "%1u", item->restricted?1:0);
    ixmlElement_setAttribute (elem, DIDL_ITEM_RESTRICTED, valuestr);
    sprintf (valuestr, "%1u", searchable?1:0);
    ixmlElement_setAttribute (elem, DIDL_CONTAINER_SEARCH, valuestr);

    char *class;
    class = dlna_upnp_object_type (item->u.container.media_class);
    didl_append_tag (doc, elem, DIDL_CONTAINER_CLASS, class);
    didl_append_tag (doc, elem, DIDL_CONTAINER_TITLE, item->u.container.title);

    IXML_Node *first = ixmlNode_getFirstChild( ( IXML_Node * ) doc );
    ixmlNode_appendChild (first, (IXML_Node *)elem);
  }
  else
    return -1;
  return 0;
}

void
didl_print (void *didl, buffer_t *out)
{
  IXML_Document *doc = didl;
  if (out)
    buffer_append(out, ixmlPrintDocument(doc));
}

void
didl_free (void *didl)
{
  IXML_Document *doc = didl;
  ixmlDocument_free (doc);
}
