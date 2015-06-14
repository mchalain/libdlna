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

static vfs_item_t *dlna_vfs_get_item_by_id (dlna_vfs_t *vfs, uint32_t id);
static vfs_item_t *dlna_vfs_get_item_by_name (dlna_vfs_t *vfs, char *name);
static void dlna_vfs_remove_item_by_id (dlna_vfs_t *vfs, uint32_t id);
static void dlna_vfs_remove_item_by_name (dlna_vfs_t *vfs, char *name);
static void dlna_vfs_free_item (dlna_vfs_t *vfs, vfs_item_t *item);

dlna_vfs_t *
dlna_vfs_new (dlna_t *dlna)
{
  dlna_vfs_t *vfs;
  struct dlna_vfs_cookie_s *cookie;

  vfs = calloc (1, sizeof (dlna_vfs_t));
  vfs->storage_type = DLNA_DMS_STORAGE_MEMORY;
  vfs->get_item_by_id = dlna_vfs_get_item_by_id;
  vfs->get_item_by_name = dlna_vfs_get_item_by_name;
  vfs->remove_item_by_id = dlna_vfs_remove_item_by_id;
  vfs->remove_item_by_name = dlna_vfs_remove_item_by_name;
  vfs->free_item = dlna_vfs_free_item;
  cookie = vfs->cookie = calloc (1, sizeof (struct dlna_vfs_cookie_s));
  cookie->vfs_root = NULL;
  cookie->vfs_items = 0;
  vfs->mode = dlna->mode;
  dlna_vfs_add_container (vfs, "root", 0, 0);

  return vfs;
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
  struct dlna_vfs_cookie_s *cookie;

  cookie = vfs->cookie;
  vfs->free_item (vfs, cookie->vfs_root);
  cookie->vfs_root = NULL;
  free (vfs->cookie);
  free (vfs);
}

static dlna_item_t *
vfs_item_get(vfs_item_t *item)
{
	if (item->type == DLNA_RESOURCE)
    return item->u.resource.item;
  return NULL;
}

static vfs_items_list_t *
vfs_item_children (vfs_item_t *item)
{
  return item->u.container.children_cookie;
}

static void
vfs_item_add_child (vfs_item_t *item, vfs_item_t *child)
{
  vfs_items_list_t *children, *first;

  if (!item || !child || !item->children)
    return;

  first = item->children (item);
  for (children = first; children; children = children->next)
    if (children->item == child)
      return; /* already present */

  children = calloc (1, sizeof (vfs_items_list_t));
  if (!children)
    return; /* not enought memory */
  children->count = first->count + 1;
  children->next = first;
  children->previous = NULL;
  children->item = child;
  first->previous = children;
  item->u.container.children_cookie = (void *)children;
}

static void
vfs_resource_add (vfs_item_t *item, vfs_resource_t *resource) 
{
  resource->next = item->u.resource.resources;
  item->u.resource.resources = resource;
}

static void
dlna_vfs_free_item (dlna_vfs_t *vfs, vfs_item_t *item)
{
  struct dlna_vfs_cookie_s *cookie;

  cookie = vfs->cookie;
  if (!vfs || !cookie->vfs_root || !item)
    return;

  HASH_DEL (cookie->vfs_root, item);
  
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
    for (children = item->children (item); children; children = children->next)
    {
      vfs->free_item (vfs, children->item);
      free (children);
    }
    break;
  }
  }
  
  item->parent = NULL;
  cookie->vfs_items--;
  free (item);
}

static void
vfs_add_source (dlna_vfs_t *vfs, protocol_info_t *source)
{
  source->next = vfs->sources;
  vfs->sources = source;
}

static dlna_status_code_t
vfs_is_id_registered (dlna_vfs_t *vfs, uint32_t id)
{
  vfs_item_t *item = NULL;
  struct dlna_vfs_cookie_s *cookie;

  cookie = vfs->cookie;

  if (!vfs || !cookie->vfs_root)
    return DLNA_ST_ERROR;

  HASH_FIND_INT (cookie->vfs_root, &id, item);

  return item ? DLNA_ST_OK : DLNA_ST_ERROR;
}

static uint32_t
vfs_provide_next_id (dlna_vfs_t *vfs, char *fullpath)
{
  uint32_t i;
  uint32_t start = 1;
  struct dlna_vfs_cookie_s *cookie;

  cookie = vfs->cookie;

  if (vfs->mode & DLNA_CAPABILITY_UPNP_AV_XBOX)
    start += STARTING_ENTRY_ID_XBOX360;


  if (!cookie->vfs_root)
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

static vfs_item_t *
dlna_vfs_get_item_by_id (dlna_vfs_t *vfs, uint32_t id)
{
  vfs_item_t *item = NULL;
  struct dlna_vfs_cookie_s *cookie;

  cookie = vfs->cookie;

  if (!vfs || !cookie->vfs_root)
    return NULL;

  HASH_FIND_INT (cookie->vfs_root, &id, item);

  return item;
}

static vfs_item_t *
dlna_vfs_get_item_by_name (dlna_vfs_t *vfs, char *name)
{
  vfs_item_t *item = NULL;
  struct dlna_vfs_cookie_s *cookie;

  cookie = vfs->cookie;

  if (!vfs || !cookie->vfs_root)
    return NULL;
  
  for (item = cookie->vfs_root; item; item = item->hh.next)
  {
    if (!strcmp (item->title(item), name))
      return item;
  }
  return NULL;
}

static char *
dlna_vfs_get_item_name (vfs_item_t *item)
{
  char *name = NULL;
  dlna_item_t *dlna_item = item->data (item);
  dlna_metadata_t *metadata = dlna_item_metadata (dlna_item, GET);
  if (metadata && metadata->title)
    name = metadata->title;
  else
    name = basename (dlna_item->filename);
  dlna_item_metadata (dlna_item, FREE);
  return name;
}

unsigned long
dlna_vfs_get_item_updateID (vfs_item_t *item)
{
  return item->root->u.container.updateID;
}

vfs_resource_t *
dlna_vfs_get_item_resources (vfs_item_t *item)
{
  return item->u.resource.resources;
}

static char *
dlna_vfs_get_container_name (vfs_item_t *item)
{
  char *name = NULL;
  name = item->u.container.title;
  return name;
}

unsigned long
dlna_vfs_get_container_updateID (vfs_item_t *item)
{
  return item->u.container.updateID;
}

uint32_t
dlna_vfs_add_container (dlna_vfs_t *vfs, char *name,
                        uint32_t object_id, uint32_t container_id)
{
  vfs_item_t *item;
  struct stat st;
  struct dlna_vfs_cookie_s *cookie;

  if (!vfs || !name)
    return 0;

  cookie = vfs->cookie;

  dlna_log (DLNA_MSG_INFO, "Adding container '%s'\n", name);

  item = calloc (1, sizeof (vfs_item_t));

  item->type = DLNA_CONTAINER;
  item->restricted = 1;
  item->title = dlna_vfs_get_container_name;
  item->updateID = dlna_vfs_get_container_updateID;
  item->data = NULL;
  item->children = vfs_item_children;

  /* is requested 'object_id' available ? */
  if (object_id == 0)
    item->id = vfs_provide_next_id (vfs, basename (name));
  else if (vfs_is_id_registered (vfs, object_id) == DLNA_ST_OK)
    return object_id;
  else
    item->id = object_id;

  HASH_ADD_INT (cookie->vfs_root, id, item);
  
  dlna_log (DLNA_MSG_INFO,
            "New container id (asked for #%u, granted #%u)\n",
            object_id, item->id);

  if (stat (name, &st) == 0 && S_ISDIR (st.st_mode))
    item->u.container.media_class = DLNA_CLASS_FOLDER;
  else
    item->u.container.media_class = DLNA_CLASS_COLLECTION;
  item->u.container.title = strdup (basename (name));
  item->u.container.add_child = vfs_item_add_child;
  
  if (item->id != 0)
  {
    item->parent = dlna_vfs_get_item_by_id (vfs, container_id);
    if (!item->parent)
      item->parent = cookie->vfs_root;
    else
      item->parent->u.container.updateID ++;
    /* add new child to parent */
    if (item->parent != item)
    {
      if (item->parent->u.container.add_child)
        item->parent->u.container.add_child (item->parent, item);
      cookie->vfs_items++;
    }
    item->root = item->parent->root;
  }
  else
  {
    item->root = item;
  }    

  item->u.container.updateID = 0;

  if (item->parent)
    dlna_log (DLNA_MSG_INFO, "Container is parent of #%u (%s)\n",
            item->parent->id, item->parent->title(item->parent));
  
  return item->id;
}

uint32_t
dlna_vfs_add_resource (dlna_vfs_t *vfs, char *name,
                       dlna_item_t *dlna_item, uint32_t container_id)
{
  dlna_protocol_t *protocol;
  vfs_item_t *item;
  struct dlna_vfs_cookie_s *cookie;
  
  if (!vfs || !name || !dlna_item)
    return 0;

  cookie = vfs->cookie;
  if (!cookie->vfs_root)
  {
    dlna_log (DLNA_MSG_ERROR, "No VFS root found. Add one first\n");
    return 0;
  }

  item = calloc (1, sizeof (vfs_item_t));

  item->type = DLNA_RESOURCE;
  item->restricted = 1;
  item->title = dlna_vfs_get_item_name;
  item->updateID = dlna_vfs_get_item_updateID;
  item->resources = dlna_vfs_get_item_resources;
  item->data = vfs_item_get;
  item->children = NULL;
  
  item->id = vfs_provide_next_id (vfs, dlna_item->filename);

  item->u.resource.item = dlna_item;

  HASH_ADD_INT (cookie->vfs_root, id, item);
  
  dlna_log (DLNA_MSG_INFO, "New resource id #%u (%s)\n",
            item->id, dlna_item->filename);

  /* check for a valid parent id */
  item->parent = dlna_vfs_get_item_by_id (vfs, container_id);
  if (!item->parent)
    item->parent = cookie->vfs_root;
  else
    item->parent->u.container.updateID ++;

  if (item->parent)
    item->root = item->parent->root;

  dlna_log (DLNA_MSG_INFO,
            "Resource is parent of #%u (%s)\n", item->parent->id, item->parent->title(item->parent));

  /* add new child to parent */
  item->parent->u.container.add_child (item->parent, item);
  cookie->vfs_items++;

  for (protocol = vfs->protocols; protocol; protocol = protocol->next)
  {
    vfs_resource_t *resource = protocol->create_resource (item);
    vfs_resource_add (item, resource);
    vfs_add_source (vfs, resource->protocol_info);
  }
  return item->id;
}

static void
dlna_vfs_remove_item_by_id (dlna_vfs_t *vfs, uint32_t id)
{
  vfs_item_t *item;

  if (!vfs)
    return;
  
  item = dlna_vfs_get_item_by_id (vfs, id);
  if (item)
  {
    dlna_log (DLNA_MSG_INFO,
              "Removing item #%u\n", item->id);
    vfs->free_item (vfs, item);
  }
}

static void
dlna_vfs_remove_item_by_name (dlna_vfs_t *vfs, char *name)
{
  vfs_item_t *item;

  if (!vfs || !name)
    return;

  item = dlna_vfs_get_item_by_name (vfs, name);
  if (item)
  {
    dlna_log (DLNA_MSG_INFO,
              "Removing item #%u (%s)\n", item->id, name);
    vfs->free_item (vfs, item);
  }
}

