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
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#include "upnp_internals.h"
#include "vfs.h"

#define STARTING_ENTRY_ID_XBOX360 100000

extern uint32_t
crc32(uint32_t crc, const void *buf, size_t size);

static dlna_stream_t *dlna_vfs_stream_open (void *cookie, const char *url);

static dlna_stream_t *
dlna_vfs_stream_open (void *cookie, const char *url)
{
  uint32_t id;
  vfs_item_t *item;
  dlna_item_t *dlna_item;
  dlna_vfs_t *vfs = (dlna_vfs_t *)cookie;
  dlna_stream_t *stream;
  vfs_resource_t *resource;
  char *page;

  if (strncmp (url, VIRTUAL_DIR, VIRTUAL_DIR_LEN))
    return NULL;
  /* ask for anything else ... */
  page = strrchr (url, '/') + 1;
  id = strtoul (page, NULL, 10);
  item = vfs_get_item_by_id (vfs, id);
  if (!item)
    return NULL;

  if (item->type != DLNA_RESOURCE)
    return NULL;

  dlna_item = vfs_item_get(item);
  if (!dlna_item)
    return NULL;

  if (!dlna_item->filename)
    return NULL;

  resource = vfs_resource_get (item);
  while (resource)
  {
    char *res_url = resource->url (resource);
    char *res_page;

    res_page = strrchr (res_url, '/') + 1;
    if (!strcmp (res_page, page))
    {
      free (res_url);
      break;
    }
    resource = resource->next;
    free (res_url);
  }
  stream = stream_open (dlna_item->filename);
  if (stream && resource && resource->protocol_info->other)
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
  printf ("stream info %s %d\n",stream->mime,strlen (stream->mime));
  return stream;
}

dlna_vfs_t *
dlna_vfs_new (dlna_t *dlna)
{
  dlna_vfs_t *vfs;

  vfs = calloc (1, sizeof (dlna_vfs_t));
  vfs->storage_type = DLNA_DMS_STORAGE_MEMORY;
  vfs->vfs_root = NULL;
  vfs->vfs_items = 0;
  vfs->mode = dlna->mode;
  dlna_vfs_add_container (vfs, "root", 0, 0);

  dlna_http_callback_t *callback;
  callback = calloc (1, sizeof (dlna_http_callback_t));
  callback->cookie = vfs;
  callback->open = dlna_vfs_stream_open;
  dlna_http_set_callback (callback);

  return vfs;
}

void
dlna_vfs_set_mode (dlna_vfs_t *vfs, int dlna_flags)
{
  if (dlna_flags != 0)
    vfs->mode |= DLNA_CAPABILITY_DLNA;
  vfs->flags = dlna_flags;
}

void
dlna_vfs_add_protocol (dlna_vfs_t *vfs, dlna_protocol_t *protocol)
{
  protocol->next = vfs->protocols;
  vfs->protocols= protocol;
}

void
dlna_vfs_free (dlna_vfs_t *vfs)
{
  vfs_item_free (vfs, vfs->vfs_root);
  vfs->vfs_root = NULL;
  free (vfs);
}

dlna_item_t *
vfs_item_get(vfs_item_t *item)
{
	if (item->type == DLNA_RESOURCE)
    return item->u.resource.item;
  return NULL;
}

void
vfs_item_free (dlna_vfs_t *vfs, vfs_item_t *item)
{
  if (!vfs || !vfs->vfs_root || !item)
    return;

  HASH_DEL (vfs->vfs_root, item);
  
  switch (item->type)
  {
  case DLNA_RESOURCE:
  {
    vfs_resource_t *resource;

    if (item->u.resource.item)
      dlna_item_free (item->u.resource.item);
    resource = item->u.resource.resources;
    while (resource)
    {
      item->u.resource.resources = resource->next;
      free (resource);
      resource = item->u.resource.resources;
    }
    break;
  }
  case DLNA_CONTAINER:
  {
    if (item->u.container.title)
      free (item->u.container.title);
    vfs_items_list_t *children;
    for (children = item->u.container.children; children; children = children->next)
    {
      vfs_item_free (vfs, children->item);
      free (children);
    }
    break;
  }
  }
  
  item->parent = NULL;
  vfs->vfs_items--;
  free (item);
}

static void
vfs_add_source (dlna_vfs_t *vfs, protocol_info_t *source)
{
  source->next = vfs->sources;
  vfs->sources = source;
}

void
vfs_resource_add (vfs_item_t *item, vfs_resource_t *resource) 
{
  resource->next = item->u.resource.resources;
  item->u.resource.resources = resource;
}

inline vfs_resource_t * 
vfs_resource_get (vfs_item_t *item) 
{
  return item->u.resource.resources;
}

static dlna_status_code_t
vfs_is_id_registered (dlna_vfs_t *vfs, uint32_t id)
{
  vfs_item_t *item = NULL;

  if (!vfs || !vfs->vfs_root)
    return DLNA_ST_ERROR;

  HASH_FIND_INT (vfs->vfs_root, &id, item);

  return item ? DLNA_ST_OK : DLNA_ST_ERROR;
}

static uint32_t
vfs_provide_next_id (dlna_vfs_t *vfs, char *fullpath)
{
  uint32_t i;
  uint32_t start = 1;

  if (vfs->mode & DLNA_CAPABILITY_UPNP_AV_XBOX)
    start += STARTING_ENTRY_ID_XBOX360;


  if (!vfs->vfs_root)
    return (start - 1);

  for (i = start; i < UINT_MAX; i++)
  {
    if (fullpath)
      i = crc32(i, fullpath, strlen(fullpath));
    if (vfs_is_id_registered (vfs, i) == DLNA_ST_ERROR)
      break;
  }

  return i;
}

vfs_item_t *
vfs_get_item_by_id (dlna_vfs_t *vfs, uint32_t id)
{
  vfs_item_t *item = NULL;

  if (!vfs || !vfs->vfs_root)
    return NULL;

  HASH_FIND_INT (vfs->vfs_root, &id, item);

  return item;
}

vfs_item_t *
vfs_get_item_by_name (dlna_vfs_t *vfs, char *name)
{
  vfs_item_t *item = NULL;

  if (!vfs || !vfs->vfs_root)
    return NULL;
  
  for (item = vfs->vfs_root; item; item = item->hh.next)
  {
    switch (item->type)
    {
    case DLNA_CONTAINER:
      if (!strcmp (item->u.container.title, name))
        return item;
      break;
    case DLNA_RESOURCE:
      {
        dlna_item_t *dlna_item = vfs_item_get (item);
        dlna_metadata_t *metadata = dlna_item_metadata (dlna_item, GET);
        if (metadata && metadata->title && !strcmp (metadata->title, name))
          return item;
        else if (!strcmp (basename (dlna_item->filename), name))
          return item;
        dlna_item_metadata (dlna_item, FREE);
      }
      break;
    }
  }
  return NULL;
}

static void
vfs_item_add_child (vfs_item_t *item, vfs_item_t *child)
{
  vfs_items_list_t *children;

  if (!item || !child)
    return;

  for (children = item->u.container.children; children; children = children->next)
    if (children->item == child)
      return; /* already present */

  children = calloc (1, sizeof (vfs_items_list_t));
  children->next = item->u.container.children;
  children->previous = NULL;
  children->item = child;
  if (item->u.container.children)
    item->u.container.children->previous = children;
  item->u.container.children = children;
  item->u.container.children_count++;
}

uint32_t
dlna_vfs_add_container (dlna_vfs_t *vfs, char *name,
                        uint32_t object_id, uint32_t container_id)
{
  vfs_item_t *item;
  struct stat st;

  if (!vfs || !name)
    return 0;

  dlna_log (DLNA_MSG_INFO, "Adding container '%s'\n", name);

  item = calloc (1, sizeof (vfs_item_t));

  item->type = DLNA_CONTAINER;
  item->restricted = 1;

  /* is requested 'object_id' available ? */
  if (object_id == 0)
    item->id = vfs_provide_next_id (vfs, basename (name));
  else if (vfs_is_id_registered (vfs, object_id) == DLNA_ST_OK)
    return object_id;
  else
    item->id = object_id;

  HASH_ADD_INT (vfs->vfs_root, id, item);
  
  dlna_log (DLNA_MSG_INFO,
            "New container id (asked for #%u, granted #%u)\n",
            object_id, item->id);

  if (stat (name, &st) == 0 && S_ISDIR (st.st_mode))
    item->u.container.media_class = DLNA_CLASS_FOLDER;
  else
    item->u.container.media_class = DLNA_CLASS_COLLECTION;
  item->u.container.title = strdup (basename (name));
  item->u.container.children = NULL;
  item->u.container.children_count = 0;
  
  if (!vfs->vfs_root)
    vfs->vfs_root = item;
  
  /* check for a valid parent id */
  if (!(container_id == 0 && item->id == 0))
  {
    item->parent = vfs_get_item_by_id (vfs, container_id);
    if (!item->parent)
      item->parent = vfs->vfs_root;
    else
      item->parent->u.container.updateID ++;
    /* add new child to parent */
    if (item->parent != item)
    {
      vfs_item_add_child (item->parent, item);
      vfs->vfs_items++;
    }
  }

  item->u.container.updateID = 0;

  if (item->parent)
    dlna_log (DLNA_MSG_INFO, "Container is parent of #%u (%s)\n",
            item->parent->id, item->parent->u.container.title);
  
  return item->id;
}

static char *
vfs_resource_other_dlna (protocol_info_t *pinfo)
{
  char dlna_other[256];
  dlna_org_flags_t flags;

  flags = DLNA_ORG_FLAG_STREAMING_TRANSFER_MODE |
    DLNA_ORG_FLAG_BACKGROUND_TRANSFERT_MODE |
    DLNA_ORG_FLAG_CONNECTION_STALL |
    DLNA_ORG_FLAG_DLNA_V15;
  sprintf (dlna_other, "%s=%s;%s=%.8x%.24x",
            "DLNA.ORG_PN", pinfo->profile->id,
            "DLNA.ORG_FLAGS", flags, 0);
  return strdup (dlna_other);
}

uint32_t
dlna_vfs_add_resource (dlna_vfs_t *vfs, char *name,
                       dlna_item_t *dlna_item, uint32_t container_id)
{
  dlna_protocol_t *protocol;
  vfs_item_t *item;
  
  if (!vfs || !name || !dlna_item)
    return 0;

  if (!vfs->vfs_root)
  {
    dlna_log (DLNA_MSG_ERROR, "No VFS root found. Add one first\n");
    return 0;
  }

  item = calloc (1, sizeof (vfs_item_t));

  item->type = DLNA_RESOURCE;
  item->restricted = 1;
  
  item->id = vfs_provide_next_id (vfs, dlna_item->filename);

  item->u.resource.item = dlna_item;

  HASH_ADD_INT (vfs->vfs_root, id, item);
  
  dlna_log (DLNA_MSG_INFO, "New resource id #%u (%s)\n",
            item->id, dlna_item->filename);

  /* check for a valid parent id */
  item->parent = vfs_get_item_by_id (vfs, container_id);
  if (!item->parent)
    item->parent = vfs->vfs_root;
  else
    item->parent->u.container.updateID ++;

  dlna_log (DLNA_MSG_INFO,
            "Resource is parent of #%u (%s)\n", item->parent->id, item->parent->u.container.title);

  /* add new child to parent */
  vfs_item_add_child (item->parent, item);
  vfs->vfs_items++;

  for (protocol = vfs->protocols; protocol; protocol = protocol->next)
  {
    vfs_resource_t *resource = protocol->create_resource (item);
    if (vfs->flags && resource->protocol_info->profile->id)
      resource->protocol_info->other = vfs_resource_other_dlna;
    vfs_resource_add (item, resource);
    vfs_add_source (vfs, resource->protocol_info);
  }
  return item->id;
}

void
dlna_vfs_remove_item_by_id (dlna_vfs_t *vfs, uint32_t id)
{
  vfs_item_t *item;

  if (!vfs)
    return;
  
  item = vfs_get_item_by_id (vfs, id);
  if (item)
  {
    dlna_log (DLNA_MSG_INFO,
              "Removing item #%u\n", item->id);
    vfs_item_free (vfs, item);
  }
}

void
dlna_vfs_remove_item_by_title (dlna_vfs_t *vfs, char *name)
{
  vfs_item_t *item;

  if (!vfs || !name)
    return;

  item = vfs_get_item_by_name (vfs, name);
  if (item)
  {
    dlna_log (DLNA_MSG_INFO,
              "Removing item #%u (%s)\n", item->id, name);
    vfs_item_free (vfs, item);
  }
}
