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

#include "upnp_internals.h"
#include "vfs.h"
#include "dlna_db.h"

#define STARTING_ENTRY_ID_XBOX360 100000

extern uint32_t
crc32(uint32_t crc, const void *buf, size_t size);

int
dlna_vfs_init (dlna_t *dlna)
{
  dlna->dms.storage_type = DLNA_DMS_STORAGE_MEMORY;
  dlna->dms.vfs_root = NULL;
  dlna->dms.vfs_items = 0;
#ifdef HAVE_SQLITE
  dlna->dms.db = NULL;
#endif /* HAVE_SQLITE */
  dlna_vfs_add_container (dlna, "root", 0, 0);
  return 0;
}

void
dlna_vfs_uninit (dlna_t *dlna)
{
  vfs_item_free (dlna, dlna->dms.vfs_root);
  dlna->dms.vfs_root = NULL;
}

void
vfs_item_free (dlna_t *dlna, vfs_item_t *item)
{
  if (!dlna || !dlna->dms.vfs_root || !item)
    return;

  HASH_DEL (dlna->dms.vfs_root, item);
  
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
      vfs_item_free (dlna, *children);
    free (item->u.container.children);
    break;
  }
  }
  
  item->parent = NULL;
  dlna->dms.vfs_items--;
  free (item);
}

static dlna_status_code_t
vfs_is_id_registered (dlna_t *dlna, uint32_t id)
{
  vfs_item_t *item = NULL;

  if (!dlna || !dlna->dms.vfs_root)
    return DLNA_ST_ERROR;

  HASH_FIND_INT (dlna->dms.vfs_root, &id, item);

  return item ? DLNA_ST_OK : DLNA_ST_ERROR;
}

static uint32_t
vfs_provide_next_id (dlna_t *dlna, char *fullpath)
{
  uint32_t i;
  uint32_t start = 1;

  if (dlna->mode == DLNA_CAPABILITY_UPNP_AV_XBOX)
    start += STARTING_ENTRY_ID_XBOX360;


  if (!dlna->dms.vfs_root)
    return (start - 1);

  for (i = start; i < UINT_MAX; i++)
  {
    if (fullpath)
      i = crc32(i, fullpath, strlen(fullpath));
    if (vfs_is_id_registered (dlna, i) == DLNA_ST_ERROR)
      break;
  }

  return i;
}

vfs_item_t *
vfs_get_item_by_id (dlna_t *dlna, uint32_t id)
{
  vfs_item_t *item = NULL;

  if (!dlna || !dlna->dms.vfs_root)
    return NULL;

  HASH_FIND_INT (dlna->dms.vfs_root, &id, item);

  return item;
}

vfs_item_t *
vfs_get_item_by_name (dlna_t *dlna, char *name)
{
  vfs_item_t *item = NULL;

  if (!dlna || !dlna->dms.vfs_root)
    return NULL;
  
  for (item = dlna->dms.vfs_root; item; item = item->hh.next)
  {
    switch (item->type)
    {
    case DLNA_CONTAINER:
      if (!strcmp (item->u.container.title, name))
        return item;
      break;
    case DLNA_RESOURCE:
      if ((!item->u.resource.item->metadata || !item->u.resource.item->metadata->title) &&
          !strcmp (basename (item->u.resource.item->filename), name))
        return item;
      else if (!strcmp (item->u.resource.item->metadata->title, name))
        return item;
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
vfs_item_add_child (dlna_t *dlna, vfs_item_t *item, vfs_item_t *child)
{
  vfs_item_t **children;
  int n;

  if (!dlna || !item || !child)
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
  dlna->dms.vfs_items++;
}

uint32_t
dlna_vfs_add_container (dlna_t *dlna, char *name,
                        uint32_t object_id, uint32_t container_id)
{
  vfs_item_t *item;
  
  if (!dlna || !name)
    return 0;

  dlna_log (dlna, DLNA_MSG_INFO, "Adding container '%s'\n", name);
  
  item = calloc (1, sizeof (vfs_item_t));

  item->type = DLNA_CONTAINER;
  
  /* is requested 'object_id' available ? */
  if (object_id == 0 || vfs_is_id_registered (dlna, object_id) == DLNA_ST_OK)
    item->id = vfs_provide_next_id (dlna, name);
  else
    item->id = object_id;

  HASH_ADD_INT (dlna->dms.vfs_root, id, item);
  
  dlna_log (dlna, DLNA_MSG_INFO,
            "New container id (asked for #%u, granted #%u)\n",
            object_id, item->id);

  item->u.container.title = strdup (name);
  item->u.container.children = calloc (1, sizeof (vfs_item_t *));
  *(item->u.container.children) = NULL;
  item->u.container.children_count = 0;
  
  if (!dlna->dms.vfs_root)
    dlna->dms.vfs_root = item;
  
  /* check for a valid parent id */
  item->parent = vfs_get_item_by_id (dlna, container_id);
  if (!item->parent)
    item->parent = dlna->dms.vfs_root;
  else
    item->parent->u.container.updateID ++;

  /* add new child to parent */
  if (item->parent != item)
    vfs_item_add_child (dlna, item->parent, item);

  item->u.container.updateID = 0;

  dlna_log (dlna, DLNA_MSG_INFO, "Container is parent of #%u (%s)\n",
            item->parent->id, item->parent->u.container.title);
  
  return item->id;
}

uint32_t
dlna_vfs_add_resource (dlna_t *dlna, char *name,
                       dlna_item_t *dlna_item, uint32_t container_id)
{
  vfs_item_t *item;
  
  if (!dlna || !name || !dlna_item)
    return 0;

  if (!dlna->dms.vfs_root)
  {
    dlna_log (dlna, DLNA_MSG_ERROR, "No VFS root found. Add one first\n");
    return 0;
  }

  item = calloc (1, sizeof (vfs_item_t));

  item->type = DLNA_RESOURCE;
  
  item->id = vfs_provide_next_id (dlna, dlna_item->filename);
  if (!dlna_item->metadata)
  {
    dlna_item->metadata = calloc (1, sizeof (dlna_metadata_t));
  }
  if (!dlna_item->metadata->title)
    dlna_item->metadata->title = strdup (name);

  item->u.resource.item = dlna_item;
  item->u.resource.cnv = DLNA_ORG_CONVERSION_NONE;

  HASH_ADD_INT (dlna->dms.vfs_root, id, item);
  
  dlna_log (dlna, DLNA_MSG_INFO, "New resource id #%u (%s)\n",
            item->id, dlna_item->metadata->title);
  item->u.resource.fd = -1;

  /* add the mime to cms source protocol info */
  dlna_append_supported_mime_types (dlna, 0, 
                                (char *)dlna_item->profile->mime);

  /* check for a valid parent id */
  item->parent = vfs_get_item_by_id (dlna, container_id);
  if (!item->parent)
    item->parent = dlna->dms.vfs_root;
  else
    item->parent->u.container.updateID ++;

  dlna_log (dlna, DLNA_MSG_INFO,
            "Resource is parent of #%u (%s)\n", item->parent->id, item->parent->u.container.title);

  /* add new child to parent */
  vfs_item_add_child (dlna, item->parent, item);
  
  return item->id;
}

void
dlna_vfs_remove_item_by_id (dlna_t *dlna, uint32_t id)
{
  vfs_item_t *item;

  if (!dlna)
    return;
  
  item = vfs_get_item_by_id (dlna, id);
  if (item)
  {
    dlna_log (dlna, DLNA_MSG_INFO,
              "Removing item #%u\n", item->id);
    vfs_item_free (dlna, item);
  }
}

void
dlna_vfs_remove_item_by_title (dlna_t *dlna, char *name)
{
  vfs_item_t *item;

  if (!dlna || !name)
    return;

  item = vfs_get_item_by_name (dlna, name);
  if (item)
  {
    dlna_log (dlna, DLNA_MSG_INFO,
              "Removing item #%u (%s)\n", item->id, name);
    vfs_item_free (dlna, item);
  }
}
