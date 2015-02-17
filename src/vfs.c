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
#include "didl.h"
#include "cms.h"

#define STARTING_ENTRY_ID_XBOX360 100000

extern uint32_t
crc32(uint32_t crc, const void *buf, size_t size);

static vfs_item_t *vfs_get_item_by_id (dlna_vfs_t *vfs, uint32_t id);
static vfs_item_t *vfs_get_item_by_name (dlna_vfs_t *vfs, char *name);
void vfs_item_free (dlna_vfs_t *vfs, vfs_item_t *item);

dlna_vfs_t *
dlna_vfs_new (dlna_t *dlna)
{
  dlna_vfs_t *vfs;

  vfs = calloc (1, sizeof (dlna_vfs_t));
  vfs->storage_type = DLNA_DMS_STORAGE_MEMORY;
  vfs->get_item_by_id = vfs_get_item_by_id;
  vfs->vfs_root = NULL;
  vfs->vfs_items = 0;
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
    for (children = item->u.container.children (item); children; children = children->next)
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

static vfs_item_t *
vfs_get_item_by_id (dlna_vfs_t *vfs, uint32_t id)
{
  vfs_item_t *item = NULL;

  if (!vfs || !vfs->vfs_root)
    return NULL;

  HASH_FIND_INT (vfs->vfs_root, &id, item);

  return item;
}

static vfs_item_t *
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

static vfs_items_list_t *
vfs_item_children (vfs_item_t *item)
{
  return item->u.container.children_cookie;
}

static void
vfs_item_add_child (vfs_item_t *item, vfs_item_t *child)
{
  vfs_items_list_t *children;

  if (!item || !child || !item->u.container.children)
    return;

  for (children = item->u.container.children (item); children; children = children->next)
    if (children->item == child)
      return; /* already present */

  children = calloc (1, sizeof (vfs_items_list_t));
  children->next = item->u.container.children (item);
  children->previous = NULL;
  children->item = child;
  if (item->u.container.children_cookie)
    ((vfs_items_list_t *)item->u.container.children_cookie)->previous = children;
  item->u.container.children_cookie = (void *)children;
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
  item->u.container.children = vfs_item_children;
  item->u.container.add_child = vfs_item_add_child;
  item->u.container.children_count = 0;
  
  if (item->id != 0)
  {
    item->parent = vfs_get_item_by_id (vfs, container_id);
    if (!item->parent)
      item->parent = vfs->vfs_root;
    else
      item->parent->u.container.updateID ++;
    /* add new child to parent */
    if (item->parent != item)
    {
      if (item->parent->u.container.add_child)
        item->parent->u.container.add_child (item->parent, item);
      vfs->vfs_items++;
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

  if (item->parent)
    item->root = item->parent->root;

  dlna_log (DLNA_MSG_INFO,
            "Resource is parent of #%u (%s)\n", item->parent->id, item->parent->u.container.title);

  /* add new child to parent */
  item->parent->u.container.add_child (item->parent, item);
  vfs->vfs_items++;

  for (protocol = vfs->protocols; protocol; protocol = protocol->next)
  {
    vfs_resource_t *resource = protocol->create_resource (item);
    if ((vfs->mode & DLNA_CAPABILITY_DLNA) && resource->protocol_info->profile->id)
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

typedef int (*sort_cmp_cb)(vfs_item_t *, vfs_item_t *);

static int
cmp_container_title (vfs_item_t *item1, vfs_item_t *item2)
{
  return strcasecmp (item1->u.container.title, item2->u.container.title);
}

static int
cmp_nocmp (vfs_item_t *item1 dlna_unused, vfs_item_t *item2 dlna_unused)
{
  return 0;
}

static int
cmp_item_filename (vfs_item_t *item1, vfs_item_t *item2)
{
  dlna_item_t *ditem1 = vfs_item_get (item1);
  dlna_item_t *ditem2 = vfs_item_get (item2);
  return strcasecmp (ditem2->filename, ditem1->filename);
}

static int
cmp_item_title (vfs_item_t *item1, vfs_item_t *item2)
{
  dlna_item_t *ditem1 = vfs_item_get (item1);
  dlna_metadata_t *meta1 = dlna_item_metadata (ditem1, GET);
  dlna_item_t *ditem2 = vfs_item_get (item2);
  dlna_metadata_t *meta2 = dlna_item_metadata (ditem2, GET);
  if (!meta2->title)
    return -1;
  if (!meta1->title)
    return 1;
  return strcasecmp (meta2->title, meta1->title);
}

static int
cmp_item_author (vfs_item_t *item1, vfs_item_t *item2)
{
  dlna_item_t *ditem1 = vfs_item_get (item1);
  dlna_metadata_t *meta1 = dlna_item_metadata (ditem1, GET);
  dlna_item_t *ditem2 = vfs_item_get (item2);
  dlna_metadata_t *meta2 = dlna_item_metadata (ditem2, GET);
  if (!meta2->author)
    return -1;
  if (!meta1->author)
    return 1;
  return strcasecmp (meta2->author, meta1->author);
}

static int
cmp_item_album (vfs_item_t *item1, vfs_item_t *item2)
{
  dlna_item_t *ditem1 = vfs_item_get (item1);
  dlna_metadata_t *meta1 = dlna_item_metadata (ditem1, GET);
  dlna_item_t *ditem2 = vfs_item_get (item2);
  dlna_metadata_t *meta2 = dlna_item_metadata (ditem2, GET);
  if (!meta2->album)
    return -1;
  if (!meta1->album)
    return 1;
  return strcasecmp (meta2->album, meta1->album);
}

static int
cmp_item_genre (vfs_item_t *item1, vfs_item_t *item2)
{
  dlna_item_t *ditem1 = vfs_item_get (item1);
  dlna_metadata_t *meta1 = dlna_item_metadata (ditem1, GET);
  dlna_item_t *ditem2 = vfs_item_get (item2);
  dlna_metadata_t *meta2 = dlna_item_metadata (ditem2, GET);
  if (!meta2->genre)
    return -1;
  if (!meta1->genre)
    return 1;
  return strcasecmp (meta2->genre, meta1->genre);
}

static int
cmp_item_track (vfs_item_t *item1, vfs_item_t *item2)
{
  dlna_item_t *ditem1 = vfs_item_get (item1);
  dlna_metadata_t *meta1 = dlna_item_metadata (ditem1, GET);
  dlna_item_t *ditem2 = vfs_item_get (item2);
  dlna_metadata_t *meta2 = dlna_item_metadata (ditem2, GET);
  return meta2->track > meta1->track;
}

static vfs_items_list_t *
sort_insert (vfs_items_list_t *list, vfs_item_t *new, sort_cmp_cb cmp)
{
  vfs_items_list_t *item;
  vfs_items_list_t *move;

  item = calloc (1, sizeof (vfs_items_list_t));
  item->item = new;
  if (!list)
  {
    list = item;
    return list;
  }
  move = list;
  while (move && cmp (move->item, new) > 0 )
  {
    item->previous = move;
    move = move->next;
  }
  item->next = move;
  if (item->previous)
    item->previous->next = item;
  if (move)
  {
    if (!move->previous)
      list = item;
    move->previous = item;
  }
  return list;
}

static vfs_items_list_t *
vfs_sort (vfs_item_t *first, char *sort dlna_unused)
{
  vfs_items_list_t *children = NULL;
  vfs_items_list_t *items = NULL;
  vfs_items_list_t *containers = NULL;

  if (first->type != DLNA_CONTAINER || !first->u.container.children)
    return NULL;

#ifdef DISABLE_SORT
  return first->u.container.children (first);
#endif
  for (children = first->u.container.children (first); children; children = children->next)
  {
    if (!children->item)
      continue;
    vfs_item_t *child = children->item;

    switch (child->type)
    {
    case DLNA_CONTAINER:
      containers = sort_insert (containers, child, cmp_container_title);
    break;
    case DLNA_RESOURCE:
      items = sort_insert (items, child, cmp_item_filename);
    break;
    }
  }
  /* Rebuild the new children list*/
  children = containers;
  if (children)
  {
    while (children->next)
      children = children->next;
    if (items)
      items->previous = children;
    children->next = items;
    children = containers;
  }
  else
    children = items;
#ifdef STORE_CHILDREN
  first->u.container.children = children;
#endif

  return children;
}

int
vfs_browse_metadata (vfs_item_t *item, char *filter, didl_result_t *result)
{
  if (!item || !result)
    return -1;

  switch (item->type)
  {
  case DLNA_RESOURCE:
    didl_append_item (result->didl, item, filter);
    result->updateid = item->root->u.container.updateID;
    break;
  case DLNA_CONTAINER:
    didl_append_container (result->didl, item, 1);
    result->updateid = item->u.container.updateID;
    break;

  default:
    break;
  }

  result->nb_returned = 1;
  result->total_match = 1;

  return 1;
}

int
vfs_browse_directchildren (vfs_item_t *item, 
                           int index, unsigned long count, 
                           char *filter, char *sort,
                           didl_result_t *result)
{
  vfs_items_list_t *items;
  int s;

  /* browsing direct children only has a sense on containers */
  if (item->type != DLNA_CONTAINER)
    return -1;
  
  result->nb_returned = 0;


  /* go to the child pointed out by index */
  items = vfs_sort(item, sort);
  for (s = 0; s < index; s++)
    if (items)
      items = items->next;

  /* UPnP CDS compliance : If starting index = 0 and requested count = 0
     then all children must be returned */
  if (index == 0 && count == 0)
    count = item->u.container.children_count;
  for (; items; items = items->next)
  {
    vfs_item_t *child = items->item;

    /* only fetch the requested count number or all entries if count = 0 */
    if (count == 0 || result->nb_returned < count)
    {
      switch (child->type)
      {
      case DLNA_CONTAINER:
        didl_append_container (result->didl, child, 0);
        break;

      case DLNA_RESOURCE:
        didl_append_item (result->didl, child, filter);
        break;

      default:
        break;
      }
      result->nb_returned++;
    }
  }

  result->total_match = item->u.container.children_count;
  result->updateid = item->u.container.updateID;

  return result->nb_returned;
}

/* CDS Search Parameters */
#define SEARCH_CLASS_MATCH_KEYWORD            "(upnp:class = \""
#define SEARCH_CLASS_DERIVED_KEYWORD          "(upnp:class derivedfrom \""
#define SEARCH_PROTOCOL_CONTAINS_KEYWORD      "(res@protocolInfo contains \""
#define SEARCH_OBJECT_KEYWORD                 "object"
#define SEARCH_AND                            ") and ("

static int
vfs_search_match (vfs_item_t *item, char *search_criteria)
{
  char keyword[256];
  int derived_from = 0, protocol_contains = 0, result = 0;
  char *and_clause = NULL;
  char *object_type = NULL;
  dlna_item_t *dlna_item;
  
  if (!item || !search_criteria)
    return 0;

  memset (keyword, '\0', sizeof (keyword));
  
  if (!strncmp (search_criteria, SEARCH_CLASS_MATCH_KEYWORD,
                strlen (SEARCH_CLASS_MATCH_KEYWORD)))
  {
    strncpy (keyword, search_criteria
             + strlen (SEARCH_CLASS_MATCH_KEYWORD), sizeof (keyword));
  }
  else if (!strncmp (search_criteria, SEARCH_CLASS_DERIVED_KEYWORD,
                     strlen (SEARCH_CLASS_DERIVED_KEYWORD)))
  {
    derived_from = 1;
    strncpy (keyword, search_criteria
             + strlen (SEARCH_CLASS_DERIVED_KEYWORD), sizeof (keyword));
  }
  else if (!strncmp (search_criteria, SEARCH_PROTOCOL_CONTAINS_KEYWORD,
                     strlen (SEARCH_PROTOCOL_CONTAINS_KEYWORD)))
  {
    protocol_contains = 1;
    strncpy (keyword, search_criteria
             + strlen (SEARCH_PROTOCOL_CONTAINS_KEYWORD), sizeof (keyword));
  }
  else
    strcpy (keyword, SEARCH_OBJECT_KEYWORD);

  dlna_item = vfs_item_get (item);
  object_type = dlna_profile_upnp_object_item (dlna_item->profile);
  
  if (derived_from && object_type
      && !strncmp (object_type, keyword, strlen (keyword)))
    result = 1;
  else if (protocol_contains)
  {
    vfs_resource_t *resource = item->u.resource.resources;

    while (resource)
    {
      buffer_t *out;
      out = buffer_new();
      cms_write_protocol_info (out, resource->protocol_info);
      if ( strstr (out->buf, keyword))
      {
        buffer_free (out);
        result = 1;
        break;
      }
      resource = resource->next;
      buffer_free (out);
    }
  }
  else if (object_type && !strcmp (object_type, keyword))
    result = 1;
  
  and_clause = strstr (search_criteria, SEARCH_AND);
  if (and_clause)
    return (result && vfs_search_match (item, and_clause + strlen (SEARCH_AND) -1));

  return 1;
}

static int
vfs_search_recursive (vfs_items_list_t *items, int index, uint32_t count, char *filter,
                       char *search_criteria, didl_result_t *result)
{
  for (; items; items = items->next)
  {
    vfs_item_t *item = items->item;
    /* only fetch the requested count number or all entries if count = 0 */
    if (count == 0 || result->nb_returned < count)
    {
      switch (item->type)
      {
      case DLNA_CONTAINER:
        if (!result->lite)
          didl_create_container(result->didl);
        vfs_search_recursive (item->u.container.children (item), index,
                                (count == 0) ? 0 : (count - result->nb_returned),
                                filter, search_criteria, result);
        if (!result->lite)
          didl_append_container (result->didl, item, 1);
        break;

      case DLNA_RESOURCE:        
        if (vfs_search_match (item, search_criteria))
        {
          if (count > result->nb_returned)
          {
            didl_append_item (result->didl, item, filter);
            result->nb_returned++;
          }
          result->total_match++;
        }
        break;

      default:
        break;
      }
    }
  }

  return 0;
}

int
vfs_search_directchildren(vfs_item_t *item, int index,
                           uint32_t count, char *filter,
                           char *search_criteria, didl_result_t *result)
{
  vfs_items_list_t *items;
  int i, ret;

  /* go to the child pointed out by index */
  items = item->u.container.children (item);
  for (i = 0; i < index; i++)
    if (items)
      items = items->next;

  /* UPnP CDS compliance : If starting index = 0 and requested count = 0
     then all children must be returned */
  if (index == 0 && count == 0)
    count = item->u.container.children_count;

  ret = vfs_search_recursive (items, index, count, filter, search_criteria, result);

  return ret;
}

#define DIDL_VIRTUAL_DIR "/didl"
#define DIDL_VIRTUAL_DIR_LEN 9
#define DIDL_ITEM_ID "id"
static
dlna_stream_t *
vfs_stream_open (void *cookie, const char *url)
{
  dlna_stream_t *stream = NULL;
  dlna_vfs_t *vfs = cookie;

  if (strncmp (url, DIDL_VIRTUAL_DIR, DIDL_VIRTUAL_DIR_LEN))
    return NULL;

  uint32_t id = 0;
  char *idstr = strstr (url, DIDL_ITEM_ID"=");
  if (idstr)
  {
    id = atoi(idstr + strlen(DIDL_ITEM_ID"=") + 1);
  }
  vfs_item_t *item;
  item = vfs_get_item_by_id(vfs, id);

  didl_result_t result;
  result.didl = didl_new ();
  result.lite = 0;

  vfs_search_directchildren(item, 0, 0, "*", "object", &result);

  buffer_t *out = buffer_new();
  didl_print (result.didl, out);
  stream = memoryfile_open ((char *)url, out->buf, out->len, SERVICE_CONTENT_TYPE);
  buffer_free (out);

  didl_free (result.didl);
  return stream;
}

void
vfs_export_didl(dlna_vfs_t *vfs)
{
  dlna_http_callback_t *callback;
  callback = calloc (1, sizeof (dlna_http_callback_t));
  callback->cookie = (void *)vfs;
  callback->open = vfs_stream_open;
  dlna_http_set_callback(DIDL_VIRTUAL_DIR, callback);
}
