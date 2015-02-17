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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "upnp_internals.h"
#include "vfs.h"
#include "http_protocol.h"

#define PROTOCOL_TYPE_PRE_SZ  11   /* for the str length of "http-get:*:" */
#define PROTOCOL_TYPE_SUFF_SZ 2    /* for the str length of ":*" */

struct http_resource_s
{
  uint32_t id;
};

static char *
http_url (vfs_resource_t *resource)
{
  char *url;
  struct http_resource_s *cookie = resource->cookie;
  
  url = malloc (1024);
  sprintf (url, "http://%s:%d%s/%u%s",
                      dlnaGetServerIpAddress (),
                      dlnaGetServerPort (), VIRTUAL_DIR, 
                      cookie->id, resource->protocol_info->profile->ext);
  return url;
}

static dlna_protocol_t *static_http_protocol = NULL;

static dlna_stream_t *
dlna_http_stream_open (void *cookie, const char *url)
{
  dlna_vfs_t *vfs = (dlna_vfs_t *)cookie;
  vfs_item_t *item;
  dlna_item_t *dlna_item;
  dlna_stream_t *stream;
  vfs_resource_t *resource;

  if (strncmp (url, VIRTUAL_DIR, VIRTUAL_DIR_LEN))
    return NULL;

  uint32_t id;
  char *page;
  page = strchr (url, '/') + 1;
  id = strtoul (strrchr(page, '/') + 1, NULL, 10);
  item = vfs->get_item_by_id (vfs, id);

  if (!item)
    return NULL;

  if (item->type != DLNA_RESOURCE)
    return NULL;

  resource = vfs_resource_get (item);
  while (resource)
  {
    char *res_url = resource->url (resource);
    char *res_page;

    res_page = strchr (strchr (res_url, '/') + 2, '/') + 1;
    if (!strcmp (res_page, page))
    {
      free (res_url);
      break;
    }
    resource = resource->next;
    free (res_url);
  }
  if (resource)
  {
    dlna_item = vfs_item_get(item);
    if (!dlna_item)
      return NULL;

    if (!dlna_item->filename)
      return NULL;

    stream = stream_open (dlna_item->filename);
    dlna_log (DLNA_MSG_INFO, "%s, file read %s\n", __FUNCTION__, dlna_item->filename);
    if (stream && resource->protocol_info->other)
    {
      char *other = resource->protocol_info->other (resource->protocol_info);
      char *mime = strdup (stream->mime);
      snprintf (stream->mime, 199, 
	    "%s:%s;DLNA.ORG_PS=%d;DLNA.ORG_CI=%d;DLNA.ORG_OP=%02d;", 
	    mime, other,
	    resource->info.speed, resource->info.cnv, resource->info.op);
      free (other);
      free (mime);
    }
  }
  return stream;
}

static vfs_resource_t *
dlna_http_resource_new (vfs_item_t *item)
{
  vfs_resource_t *resource;
  dlna_item_t *dlna_item;
  struct http_resource_s *cookie;

  resource = calloc (1, sizeof (vfs_resource_t));
  cookie = calloc (1, sizeof (struct http_resource_s));
  cookie->id = item->id;
  resource->cookie = cookie;
  resource->url = http_url;

  dlna_item = vfs_item_get (item);
  resource->protocol_info = calloc (1, sizeof (protocol_info_t));
  resource->protocol_info->protocol = static_http_protocol;
  resource->protocol_info->profile = dlna_item->profile;

  resource->size = dlna_item->filesize;
  dlna_properties_t *properties = dlna_item_properties (dlna_item);
  if (properties)
    memcpy (&resource->properties, properties, sizeof (dlna_properties_t));
  resource->info.protocolid = DLNA_PROTOCOL_INFO_TYPE_HTTP;
  resource->info.cnv = DLNA_ORG_CONVERSION_NONE;
  resource->info.op = DLNA_ORG_OPERATION_RANGE;
  resource->info.speed = DLNA_ORG_PLAY_SPEED_NORMAL;
  return resource;
}

static const char *
http_name ()
{
  return "http-get";
}

static const char *
http_net ()
{
  return "*";
}

static int
dlna_http_init (dlna_vfs_t *vfs)
{
  dlna_http_callback_t *callback;
  callback = calloc (1, sizeof (dlna_http_callback_t));
  callback->cookie = vfs;
  callback->open = dlna_http_stream_open;
  dlna_http_set_callback(VIRTUAL_DIR, callback);
  return 0;
}

dlna_protocol_t *
http_protocol_new (dlna_t *dlna dlna_unused)
{
  if (!static_http_protocol)
  {
    static_http_protocol = calloc (1, sizeof (dlna_protocol_t));
    static_http_protocol->type = DLNA_PROTOCOL_INFO_TYPE_HTTP;
    static_http_protocol->create_resource = dlna_http_resource_new;
    static_http_protocol->init = dlna_http_init;
    static_http_protocol->name = http_name;
    static_http_protocol->net = http_net;
  }
  return static_http_protocol;
}

