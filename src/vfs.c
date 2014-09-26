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

#include "upnp_internals.h"
#include "vfs.h"

#define STARTING_ENTRY_ID_XBOX360 100000

extern uint32_t
crc32(uint32_t crc, const void *buf, size_t size);

dlna_vfs_t *
dlna_vfs_new (dlna_t *dlna dlna_unused)
{
  dlna_vfs_t *vfs;

  vfs = calloc (1, sizeof (dlna_vfs_t));
  vfs->storage_type = DLNA_DMS_STORAGE_MEMORY;
  vfs->vfs_root = NULL;
  vfs->vfs_items = 0;
  vfs->mode = dlna->mode;
  dlna_vfs_add_container (vfs, "root", 0, 0);
  return vfs;
}

void
dlna_vfs_free (dlna_vfs_t *vfs)
{
  vfs_item_free (vfs, vfs->vfs_root);
  vfs->vfs_root = NULL;
  free (vfs);
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
    if (item->u.resource.item)
      dlna_item_free (item->u.resource.item);
    if (item->u.resource.url)
      free (item->u.resource.url);
    break;
  case DLNA_CONTAINER:
  {
    if (item->u.container.title)
      free (item->u.container.title);
    vfs_item_t **children;
    for (children = item->u.container.children; *children; children++)
      vfs_item_free (vfs, *children);
    free (item->u.container.children);
    break;
  }
  }
  
  item->parent = NULL;
  vfs->vfs_items--;
  free (item);
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
        dlna_metadata_t *metadata = dlna_item_metadata (item->u.resource.item);
        if (metadata && metadata->title && !strcmp (metadata->title, name))
          return item;
        else if (!strcmp (basename (item->u.resource.item->filename), name))
          return item;
      }
      break;
    }
  }
  return NULL;
}

static int
list_get_length (void *list)
{
  void **l = list;
  int n = 0;

  while (*(l++))
    n++;

  return n;
}

static void
vfs_item_add_child (vfs_item_t *item, vfs_item_t *child)
{
  vfs_item_t **children;
  int n;

  if (!item || !child)
    return;

  for (children = item->u.container.children; *children; children++)
    if (*children == child)
      return; /* already present */

  n = list_get_length ((void *) item->u.container.children) + 1;
  item->u.container.children = (vfs_item_t **)
    realloc (item->u.container.children,
             (n + 1) * sizeof (*(item->u.container.children)));
  item->u.container.children[n] = NULL;
  item->u.container.children[n - 1] = child;
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
  item->u.container.children = calloc (1, sizeof (vfs_item_t *));
  *(item->u.container.children) = NULL;
  item->u.container.children_count = 0;
  
  if (!vfs->vfs_root)
    vfs->vfs_root = item;
  
  /* check for a valid parent id */
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

  item->u.container.updateID = 0;

  dlna_log (DLNA_MSG_INFO, "Container is parent of #%u (%s)\n",
            item->parent->id, item->parent->u.container.title);
  
  return item->id;
}

uint32_t
dlna_vfs_add_resource (dlna_vfs_t *vfs, char *name,
                       dlna_item_t *dlna_item, uint32_t container_id)
{
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
  
  item->id = vfs_provide_next_id (vfs, dlna_item->filename);

  item->u.resource.item = dlna_item;
  item->u.resource.cnv = DLNA_ORG_CONVERSION_NONE;

  HASH_ADD_INT (vfs->vfs_root, id, item);
  
  dlna_log (DLNA_MSG_INFO, "New resource id #%u (%s)\n",
            item->id, dlna_item->filename);
  item->u.resource.fd = -1;

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

  item->u.resource.resources = dlna_http_create_resource (item);
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
